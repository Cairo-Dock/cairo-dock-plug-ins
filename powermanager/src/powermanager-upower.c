/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
3* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"
#include "powermanager-common.h"
#include "powermanager-upower.h"


#ifdef CD_UPOWER_AVAILABLE  // code with libupower

static void _cd_upower_connect_async (CDSharedMemory *pSharedMemory)
{
	// connect to UPower on Dbus.
	UpClient *pUPowerClient = up_client_new ();
	
	// get the list of devices.
	if (! up_client_enumerate_devices_sync (pUPowerClient, NULL, NULL))
	{	
		cd_warning ("couldn't get devices from UPower daemon");
		g_object_unref (pUPowerClient);
		return;
	}
	
	// find the battery device.
	GPtrArray *pDevices = up_client_get_devices (pUPowerClient);
	g_return_if_fail (pDevices != NULL);  // just to be sure.
	UpDevice *pDevice, *pBatteryDevice = NULL;
	UpDeviceKind kind;
	guint i;
	for (i = 0; i < pDevices->len; i ++)
	{
		pDevice = g_ptr_array_index (pDevices, i);
		g_object_get (G_OBJECT (pDevice), "kind", &kind, NULL);
		if (kind == UP_DEVICE_KIND_BATTERY)
		{
			pBatteryDevice = pDevice;
			break;
		}
	}
	if (pBatteryDevice == NULL)
	{
		cd_debug ("no battery found amongst %d devices", pDevices->len);
		g_object_unref (pUPowerClient);
		return;
	}
	
	pSharedMemory->pUPowerClient = pUPowerClient;
	g_object_ref (pBatteryDevice);  // just to be sure it won't be destroyed before we go back to the main loop (shouldn't happen, since devices don't change over time).
	pSharedMemory->pBatteryDevice = pBatteryDevice;
}

static void _fetch_current_values (UpDevice *pDevice)
{
	gboolean is_present;
	g_object_get (G_OBJECT (pDevice), "is-present", &is_present, NULL);
	myData.bBatteryPresent = is_present;
	if (myData.bBatteryPresent)
	{
		UpDeviceState iState;
		g_object_get (G_OBJECT (pDevice), "state", &iState, NULL);
		myData.bOnBattery = (iState == UP_DEVICE_STATE_DISCHARGING || iState == UP_DEVICE_STATE_PENDING_DISCHARGE);
		
		gdouble percentage;
		g_object_get (G_OBJECT (pDevice), "percentage", &percentage, NULL);
		myData.iPercentage = round (percentage);
		
		guint64 time;
		g_object_get (G_OBJECT (pDevice), myData.bOnBattery ? "time-to-empty" : "time-to-full", &time, NULL);
		myData.iTime = time;
		
		if (myData.iTime == 0 && myData.iPercentage < 100)  // the UPower daemon doesn't give us a time, let's compute it ourselves.
			myData.iTime = cd_estimate_time ();
	}
	else
	{
		myData.bOnBattery = FALSE;
		myData.iTime = 0;
		myData.iPercentage = 0;  // since we are not on battery, this will not trigger any alert.
	}
}

static void _on_device_changed (UpDevice *pDevice, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("battery properties changed");
	
	// the client object is already up-to-date, just fetch its values.
	_fetch_current_values (pDevice);
	
	// and update the icon.
	update_icon ();
	CD_APPLET_LEAVE ();
}

static gboolean _cd_upower_update_state (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	if (pSharedMemory->pUPowerClient == NULL)  // no UPower available, try to find the battery by ourselves.
	{
		cd_debug ("no UPower available");
		cd_check_power_files ();
	}
	else  // UPower is available, fetch the values we got from it.
	{
		// fetch the values we got on the device, and keep them up-to-date (we don't need the device itself).
		// a priori, no need to watch the "onBattery" signal on the client, since we can deduce this property from the device state.
		_fetch_current_values (pSharedMemory->pBatteryDevice);
		
		g_object_get (pSharedMemory->pBatteryDevice, "technology", &myData.cTechnology, NULL);
		g_object_get (pSharedMemory->pBatteryDevice, "vendor", &myData.cVendor, NULL);
		g_object_get (pSharedMemory->pBatteryDevice, "model", &myData.cModel, NULL);
		g_object_get (pSharedMemory->pBatteryDevice, "capacity", &myData.fMaxAvailableCapacity, NULL);
		
		myData.iSignalID = g_signal_connect (pSharedMemory->pBatteryDevice, "changed", G_CALLBACK (_on_device_changed), NULL);  // a battery not present is still a valid device. So if we could find a battery device, it will stay here forever, so we don't need to watch for the destruction/creation of a battery device.
		
		// keep our client.
		myData.pUPowerClient = pSharedMemory->pUPowerClient;
		pSharedMemory->pUPowerClient = NULL;
	}
	
	// in any case, update the icon to show the current state we are in.
	update_icon ();
	
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	
	CD_APPLET_LEAVE (FALSE);
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	if (pSharedMemory->pUPowerClient)
		g_object_unref (pSharedMemory->pUPowerClient);
	if (pSharedMemory->pBatteryDevice)
	{
		// _on_device_changed can still be called
		if (g_signal_handler_is_connected (pSharedMemory->pBatteryDevice, myData.iSignalID))
			g_signal_handler_disconnect (pSharedMemory->pBatteryDevice, myData.iSignalID);
		g_object_unref (pSharedMemory->pBatteryDevice);  // remove the ref we took on it.
	}
	g_free (pSharedMemory);
}

void cd_powermanager_start (void)
{
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	myData.pTask = cairo_dock_new_task_full (0,
		(CairoDockGetDataAsyncFunc) _cd_upower_connect_async,
		(CairoDockUpdateSyncFunc) _cd_upower_update_state,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	cairo_dock_launch_task (myData.pTask);
}


gboolean cd_power_hibernate (void)
{
	if (myData.pUPowerClient != NULL)
		return up_client_hibernate_sync (myData.pUPowerClient, NULL, NULL);
	else
		return FALSE;
}

gboolean cd_power_suspend (void)
{
	if (myData.pUPowerClient != NULL)
		return up_client_suspend_sync (myData.pUPowerClient, NULL, NULL);
	else
		return FALSE;
}

gboolean cd_power_can_hibernate (void)
{
	if (myData.pUPowerClient != NULL)
		return up_client_get_can_hibernate (myData.pUPowerClient);
	else
		return FALSE;
}

gboolean cd_power_can_suspend (void)
{
	if (myData.pUPowerClient != NULL)
		return up_client_get_can_suspend (myData.pUPowerClient);
	else
		return FALSE;
}

#else // code without libupower

void cd_powermanager_start (void)
{
	cd_check_power_files ();
}

gboolean cd_power_hibernate (void)
{
	return FALSE;
}

gboolean cd_power_suspend (void)
{
	return FALSE;
}

gboolean cd_power_can_hibernate (void)
{
	return FALSE;
}

gboolean cd_power_can_suspend (void)
{
	return FALSE;
}

#endif

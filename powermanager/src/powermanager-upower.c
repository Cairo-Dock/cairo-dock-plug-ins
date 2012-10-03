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
	UpDevice *pDevice;
	GList *pBatteryDeviceList = NULL;
	UpDeviceKind kind;
	guint i;
	for (i = 0; i < pDevices->len; i ++)
	{
		pDevice = g_ptr_array_index (pDevices, i);
		g_object_get (G_OBJECT (pDevice), "kind", &kind, NULL);
		if (kind == UP_DEVICE_KIND_BATTERY)
		{
			pBatteryDeviceList = g_list_append (pBatteryDeviceList, pDevice);
			g_object_ref (pDevice);  // since the object does not belong to us, we ref it here, and we'll keep our ref until the end of the applet.
		}
	}
	if (pBatteryDeviceList == NULL)
	{
		cd_debug ("no battery found amongst %d devices", pDevices->len);
		/* g_object_unref (pUPowerClient); // => if we launch the dock without any battery and then connect the battery, we need to be notified
		return;*/ 
	}
	
	pSharedMemory->pUPowerClient = pUPowerClient;
	pSharedMemory->pBatteryDeviceList = pBatteryDeviceList;
}

static void _fetch_current_values (GList *pBatteryDeviceList)
{
	myData.bOnBattery = FALSE;
	myData.bBatteryPresent = FALSE;
	myData.iTime = 0;
	myData.iPercentage = 0;  // since we are not on battery, this will not trigger any alert.
	if (! pBatteryDeviceList)
		return;

	myData.bOnBattery = FALSE;
	myData.bBatteryPresent = FALSE;
	UpDevice *pDevice;
	gboolean is_present;
	gdouble fPercentageGlobal = 0.;
	GList *pItem;
	myData.iTime = 0;
	int iLength = 0;
	for (pItem = pBatteryDeviceList; pItem != NULL; pItem = g_list_next (pItem))
	{
		pDevice = pItem->data;
		g_object_get (G_OBJECT (pDevice), "is-present", &is_present, NULL);
		myData.bBatteryPresent |= is_present;
		
		if (is_present)
		{
			UpDeviceState iState;
			g_object_get (G_OBJECT (pDevice), "state", &iState, NULL);
			myData.bOnBattery |= (iState == UP_DEVICE_STATE_DISCHARGING || iState == UP_DEVICE_STATE_PENDING_DISCHARGE);
			
			gdouble percentage;
			g_object_get (G_OBJECT (pDevice), "percentage", &percentage, NULL);
			fPercentageGlobal += percentage;
			
			guint64 time;
			g_object_get (G_OBJECT (pDevice), myData.bOnBattery ? "time-to-empty" : "time-to-full", &time, NULL);
			myData.iTime += time;

			cd_debug ("New data (%d: %p): OnBattery %d ; percentage %f ; time %lu", iLength, pDevice, myData.bOnBattery, percentage, time);
			iLength++;
		}
	}
	if (iLength > 0)
		myData.iPercentage = round (fPercentageGlobal / iLength);
	if (myData.iTime == 0 && myData.iPercentage < 100)  // the UPower daemon doesn't give us a time, let's compute it ourselves.
		myData.iTime = cd_estimate_time ();
}

static void _on_device_list_changed (UpClient *pClient, UpDevice *pDevice, gpointer data)
{
	cd_debug ("Device list changed");
	g_free (myData.cTechnology);
	g_free (myData.cVendor);
	g_free (myData.cModel);
	cd_upower_stop ();
	cd_powermanager_start (); // yeah, it's not the best solution but we have to check the list and re-compute all properties (properties needs to be placed in a structure for each battery)
}

static void _on_device_changed (UpDevice *pDevice, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("battery properties changed");
	
	// the client object is already up-to-date, just fetch its values.
	_fetch_current_values (myData.pBatteryDeviceList);
	
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
		_fetch_current_values (pSharedMemory->pBatteryDeviceList);

		// fetch the values we got on the device, and keep them up-to-date (we don't need the device itself).
		UpDevice *pDevice;
		GList *pItem;
		gint iSignalID;
		GString *sTechnology = NULL, *sVendor = NULL, *sModel = NULL;
		const gchar *cTechnology;
		gchar *cVendor, *cModel;
		gdouble fMaxAvailableCapacity = 0., fTmp;
		UpDeviceTechnology iTechnology;
		gboolean bFirst = TRUE;
		for (pItem = pSharedMemory->pBatteryDeviceList ; pItem != NULL ; pItem = g_list_next (pItem))
		{
			pDevice = pItem->data;
			
			g_object_get (pDevice, "technology", &iTechnology, NULL);
			g_object_get (pDevice, "vendor", &cVendor, NULL);
			g_object_get (pDevice, "model", &cModel, NULL);
			g_object_get (pDevice, "capacity", &fTmp, NULL);
			fMaxAvailableCapacity += fTmp;

			cTechnology = up_device_technology_to_string (iTechnology);

			cd_debug ("New Battery: %s, %s, %s, %f", cTechnology, cVendor, cModel, fTmp);

			if (bFirst)
			{
				sTechnology = g_string_new (cTechnology);
				sVendor = g_string_new (cVendor);
				sModel = g_string_new (cModel);
			}
			else
			{
				g_string_append_printf (sTechnology, " & %s", cTechnology);
				g_string_append_printf (sVendor, " & %s", cVendor);
				g_string_append_printf (sModel, " & %s", cModel);
			}
			g_free (cVendor);
			g_free (cModel);

			// watch for any change. A priori, no need to watch the "onBattery" signal on the client, since we can deduce this property from the device state.
			iSignalID = g_signal_connect (pDevice, "changed", G_CALLBACK (_on_device_changed), NULL);  // a battery not present is still a valid device. So if we could find a battery device, it will stay here forever, so we don't need to watch for the destruction/creation of a battery device.
			myData.pSignalIDList = g_list_append (myData.pSignalIDList, GINT_TO_POINTER (iSignalID));

			bFirst = FALSE;
		}
		myData.iSignalIDAdded = g_signal_connect (pSharedMemory->pUPowerClient, "device-added", G_CALLBACK (_on_device_list_changed), NULL);
		myData.iSignalIDRemoved = g_signal_connect (pSharedMemory->pUPowerClient, "device-removed", G_CALLBACK (_on_device_list_changed), NULL);
		myData.fMaxAvailableCapacity = fMaxAvailableCapacity / g_list_length (pSharedMemory->pBatteryDeviceList);
		myData.cTechnology = g_string_free (sTechnology, FALSE);
		myData.cVendor = g_string_free (sVendor, FALSE);
		myData.cModel = g_string_free (sModel, FALSE);

		// keep our client and device.
		myData.pUPowerClient = pSharedMemory->pUPowerClient;
		pSharedMemory->pUPowerClient = NULL;
		myData.pBatteryDeviceList = pSharedMemory->pBatteryDeviceList;
		pSharedMemory->pBatteryDeviceList = NULL;  // so we won't unref it in the destroy callback of the task.
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
	if (pSharedMemory->pBatteryDeviceList)
		g_list_foreach (pSharedMemory->pBatteryDeviceList, (GFunc) g_object_unref, NULL);  // remove the ref we took on it.
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

void cd_upower_stop (void)
{
	if (myData.pUPowerClient != NULL)
	{
		g_object_unref (myData.pUPowerClient);
	}
	
	if (myData.pSignalIDList)
	{
		g_list_foreach (myData.pSignalIDList, (GFunc) g_source_remove, NULL);
		g_list_free (myData.pSignalIDList);
	}

	if (myData.pBatteryDeviceList)
	{
		g_list_foreach (myData.pBatteryDeviceList, (GFunc) g_object_unref, NULL); // remove the ref we took on the device. it may or not destroy the object, that's why we disconnected manually the signal above.
		g_list_free (myData.pBatteryDeviceList);
	}

	if (myData.iSignalIDAdded != 0)
		g_source_remove (myData.iSignalIDAdded);
	if (myData.iSignalIDRemoved != 0)
		g_source_remove (myData.iSignalIDRemoved);
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

void cd_upower_stop (void)
{

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

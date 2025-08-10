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

static GList * _cd_upower_add_and_ref_device_if_battery (UpDevice *pDevice, GList *pBatteryDeviceList)
{
	UpDeviceKind kind;
	g_object_get (G_OBJECT (pDevice), "kind", &kind, NULL);
	if (kind == UP_DEVICE_KIND_BATTERY)
	{
		pBatteryDeviceList = g_list_append (pBatteryDeviceList, pDevice);
		g_object_ref (pDevice);  // since the object does not belong to us, we ref it here, and we'll keep our ref until the end of the applet.
	}
	return pBatteryDeviceList;
}

static void _cd_upower_connect_async (CDSharedMemory *pSharedMemory)
{
	// connect to UPower on Dbus.
	UpClient *pUPowerClient = up_client_new ();
	
	// get the list of devices.
	if (pUPowerClient == NULL
	#ifndef CD_UPOWER_0_99 // no longer available with UPower 0.99+
		|| ! up_client_enumerate_devices_sync (pUPowerClient, NULL, NULL)
	#endif
		)
	{	
		cd_warning ("couldn't get devices from UPower daemon");
		if (pUPowerClient)
			g_object_unref (pUPowerClient);
		return;
	}
	
	// find the battery device.
	GPtrArray *pDevices = up_client_get_devices (pUPowerClient);
	g_return_if_fail (pDevices != NULL);  // just to be sure.
	UpDevice *pDevice;
	GList *pBatteryDeviceList = NULL;
	guint i;
	for (i = 0; i < pDevices->len; i ++)
	{
		pDevice = g_ptr_array_index (pDevices, i);
		pBatteryDeviceList = _cd_upower_add_and_ref_device_if_battery (pDevice, pBatteryDeviceList);
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
	
	UpDevice *pDevice;
	gboolean is_present;
	gdouble fPercentageGlobal = 0.;
	GList *pItem;
	gint iNbBatteries = 0;
	for (pItem = pBatteryDeviceList; pItem != NULL; pItem = g_list_next (pItem))
	{
		pDevice = pItem->data;
		g_object_get (G_OBJECT (pDevice), "is-present", &is_present, NULL);
		myData.bBatteryPresent |= is_present;
		
		if (is_present)
		{
			UpDeviceState iState;
			g_object_get (G_OBJECT (pDevice), "state", &iState, NULL);
			myData.bOnBattery |= (iState == UP_DEVICE_STATE_DISCHARGING || iState == UP_DEVICE_STATE_PENDING_DISCHARGE);  // we assume that all batteries are either charging or discharging, not a mix of both, so that we won't add a 'time-to-empty' with a 'time-to-full'; this avoid us making a first pass on the list to determine 'myData.bOnBattery' and then a 2nd pass to compute the time.
			
			gdouble percentage;
			g_object_get (G_OBJECT (pDevice), "percentage", &percentage, NULL);
			fPercentageGlobal += percentage;
			
			guint64 time;
			g_object_get (G_OBJECT (pDevice), myData.bOnBattery ? "time-to-empty" : "time-to-full", &time, NULL);
			myData.iTime += time;

			cd_debug ("New data (%d: %p): OnBattery %d ; percentage %f ; time %lu", iNbBatteries, pDevice, myData.bOnBattery, percentage, time);
			iNbBatteries ++;
		}
	}
	if (iNbBatteries > 0)
		myData.iPercentage = round (fPercentageGlobal / iNbBatteries);
	if (myData.iTime == 0 && myData.iPercentage < 100)  // the UPower daemon doesn't give us a time, let's compute it ourselves.
		myData.iTime = cd_estimate_time ();
}

static void _on_device_list_changed_free_data (void)
{
	cd_debug ("Device list changed");
	g_free (myData.cTechnology);
	myData.cTechnology = NULL;
	g_free (myData.cVendor);
	myData.cVendor = NULL;
	g_free (myData.cModel);
	myData.cModel = NULL;
}

static gboolean _cd_upower_update_state (CDSharedMemory *pSharedMemory);

static void _on_device_added (UpClient *pClient, UpDevice *pDevice, gpointer data)
{
	CD_APPLET_ENTER;
	if (pClient != myData.pUPowerClient) // should not happen...
	{
		g_object_unref (myData.pUPowerClient);
		myData.pUPowerClient = NULL;
	}

	// if it's really a new device (yes, be secured...)
	if (g_list_find (myData.pBatteryDeviceList, pDevice) == NULL)
	{
		_on_device_list_changed_free_data (); // free data

		// update state: get all devices and all info
		CDSharedMemory SharedMemory;
		SharedMemory.pBatteryDeviceList = _cd_upower_add_and_ref_device_if_battery (pDevice, myData.pBatteryDeviceList);
		SharedMemory.pUPowerClient = pClient;
		_cd_upower_update_state (&SharedMemory);
	}
	CD_APPLET_LEAVE ();
}

static void _on_device_removed (UpClient *pClient, UpDevice *pDevice, gpointer data)
{
	CD_APPLET_ENTER;
	if (pClient != myData.pUPowerClient) // should not happen...
	{
		g_object_unref (myData.pUPowerClient);
		myData.pUPowerClient = NULL;
	}

	GList *pOldDevice = g_list_find (myData.pBatteryDeviceList, pDevice);
	if (pOldDevice != NULL) // if we've already added this device (yes, be secured)
	{
		_on_device_list_changed_free_data (); // free data
		g_object_unref (pDevice); // unref, we no longer need it...

		// update state: get all devices and all info
		CDSharedMemory SharedMemory;
		SharedMemory.pBatteryDeviceList = g_list_delete_link (myData.pBatteryDeviceList, pOldDevice);
		SharedMemory.pUPowerClient = pClient;
		_cd_upower_update_state (&SharedMemory);
	}
	CD_APPLET_LEAVE ();
}

#ifdef CD_UPOWER_0_99 // one more param
static void _on_device_changed (G_GNUC_UNUSED UpDevice *pDevice, G_GNUC_UNUSED GParamSpec *pSpec, G_GNUC_UNUSED gpointer data)
#else
static void _on_device_changed (G_GNUC_UNUSED UpDevice *pDevice, G_GNUC_UNUSED gpointer data)
#endif
{
	// The applet is removed just before an update...
	if (myApplet == NULL)
		return;

	CD_APPLET_ENTER;
	cd_debug ("battery properties changed");
	
	// the client object is already up-to-date, just fetch its values.
	_fetch_current_values (myData.pBatteryDeviceList);
	
	// and update the icon.
	update_icon ();
	CD_APPLET_LEAVE ();
}

// Can be launched the first time (with the Task) or when a device is added/removed after.
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
		// fetch the current values we got on the devices
		_fetch_current_values (pSharedMemory->pBatteryDeviceList);
		
		// fetch static values of the devices, and watch for any change
		UpDevice *pDevice;
		GList *pItem;
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

			if (myData.pTask != NULL // only the first time
				|| myData.pBatteryDeviceList == NULL // or if it's a new device
				|| g_list_find (myData.pBatteryDeviceList, pDevice) == NULL)
				// or compare the up_device_get_object_path (pDevice) ?
			{
				/* watch for any change. A priori, no need to watch the
				 * "onBattery" signal on the client, since we can deduce this
				 * property from the device state.
				 * A battery not present is still a valid device. So if we could
				 * find a battery device, it will stay here forever, so we don't
				 * need to watch for the destruction/creation of a battery device.
				 */
				#ifdef CD_UPOWER_0_99 // Now called notify
				g_signal_connect (pDevice, "notify", G_CALLBACK (_on_device_changed), NULL);
				#else
				g_signal_connect (pDevice, "changed", G_CALLBACK (_on_device_changed), NULL);
				#endif
			}

			bFirst = FALSE;
		}

		myData.fMaxAvailableCapacity = fMaxAvailableCapacity; // Add all capacities

		if (! bFirst) // at least one battery, strings have been defined
		{
			myData.cTechnology = g_string_free (sTechnology, FALSE);
			myData.cVendor = g_string_free (sVendor, FALSE);
			myData.cModel = g_string_free (sModel, FALSE);
		}

		if (myData.pTask != NULL // only the first time
			|| pSharedMemory->pUPowerClient != myData.pUPowerClient) // or a new client (should not happen...)
		{
			myData.iSignalIDAdded = g_signal_connect (pSharedMemory->pUPowerClient,
				"device-added", G_CALLBACK (_on_device_added), NULL);
			myData.iSignalIDRemoved = g_signal_connect (pSharedMemory->pUPowerClient,
				"device-removed", G_CALLBACK (_on_device_removed), NULL);
			// Note: these signals (removed and added) are also send when resuming from suspend...
		}
		
		// keep our client and devices.
		myData.pUPowerClient = pSharedMemory->pUPowerClient;
		pSharedMemory->pUPowerClient = NULL;
		myData.pBatteryDeviceList = pSharedMemory->pBatteryDeviceList;
		pSharedMemory->pBatteryDeviceList = NULL;  // so we won't unref it in the destroy callback of the task.
	}
	
	// in any case, update the icon to show the current state we are in.
	update_icon ();

	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
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
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	myData.pTask = gldi_task_new_full (0,
		(GldiGetDataAsyncFunc) _cd_upower_connect_async,
		(GldiUpdateSyncFunc) _cd_upower_update_state,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	gldi_task_launch (myData.pTask);
}

static void _free_battery_dev (gpointer ptr)
{
	GObject *obj = (GObject*)ptr;
	g_signal_handlers_disconnect_by_func (obj, _on_device_changed, NULL);
	g_object_unref (obj); // remove the ref we took on the device. it may or not destroy the object, that's why we disconnected manually the signal above.
}

void cd_upower_stop (void)
{
	if (myData.pUPowerClient != NULL)
	{
		if (myData.iSignalIDAdded != 0)
		{
			g_signal_handler_disconnect (myData.pUPowerClient, myData.iSignalIDAdded);
			myData.iSignalIDAdded = 0;
		}
		if (myData.iSignalIDRemoved != 0)
		{
			g_signal_handler_disconnect (myData.pUPowerClient, myData.iSignalIDRemoved);
			myData.iSignalIDRemoved = 0;
		}
		
		g_object_unref (myData.pUPowerClient);
		myData.pUPowerClient = NULL;
	}

	if (myData.pBatteryDeviceList)
	{
		g_list_free_full (myData.pBatteryDeviceList, _free_battery_dev);
		myData.pBatteryDeviceList = NULL;
	}
}

#else // code without libupower

void cd_powermanager_start (void)
{
	cd_check_power_files ();
}

void cd_upower_stop (void)
{

}

#endif

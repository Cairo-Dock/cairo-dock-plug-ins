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

#include "powermanager-struct.h"
#include "powermanager-draw.h"
#include "powermanager-common.h"
#include "powermanager-upower.h"


#ifdef CD_UPOWER_AVAILABLE  // code with libupower

static void _cd_upower_update_state (gboolean bFirstUpdate);
static void _on_device_changed (G_GNUC_UNUSED UpDevice *pDevice, G_GNUC_UNUSED GParamSpec *pSpec, G_GNUC_UNUSED gpointer data);
static void _free_battery_dev (gpointer ptr);

static gboolean _cd_upower_add_and_ref_device_if_battery (UpDevice *pDevice, GList **pBatteryDeviceList)
{
	UpDeviceKind kind;
	g_object_get (G_OBJECT (pDevice), "kind", &kind, NULL);
	if (kind == UP_DEVICE_KIND_BATTERY)
	{
		*pBatteryDeviceList = g_list_prepend (*pBatteryDeviceList, pDevice);
		g_object_ref (pDevice);  // since the object does not belong to us, we ref it here, and we'll keep our ref until the end of the applet.
		return TRUE;
	}
	return FALSE;
}

static void _on_got_devices (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	// find the battery devices.
	UpClient *pUPowerClient = UP_CLIENT (pObj);
	GError *err = NULL;
	GPtrArray *pDevices = up_client_get_devices_finish (pUPowerClient, pRes, &err);
	if (!pDevices)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Error getting devices from UPower daemon: %s", err->message);
			_cd_upower_update_state (TRUE); // try fallback in this case (note: if cancelled, we should not call this function)
		}
		g_error_free (err);
		g_object_unref (pUPowerClient);
	}
	else
	{
		UpDevice *pDevice;
		GList *pBatteryDeviceList = NULL;
		guint i;
		for (i = 0; i < pDevices->len; i ++)
		{
			pDevice = g_ptr_array_index (pDevices, i);
			_cd_upower_add_and_ref_device_if_battery (pDevice, &pBatteryDeviceList);
		}
		if (pBatteryDeviceList == NULL)
			cd_debug ("no battery found amongst %d devices", pDevices->len);
			// we keep pUPowerClient since a battery might be added later
		g_ptr_array_unref (pDevices);
		
		myData.pUPowerClient = pUPowerClient;
		myData.pBatteryDeviceList = pBatteryDeviceList;
		_cd_upower_update_state (TRUE);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_got_upower (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	UpClient *pUPowerClient = up_client_new_finish (pRes, &err);
	if (!pUPowerClient)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Error connecting to UPower daemon: %s", err->message);
			_cd_upower_update_state (TRUE); // try fallback in this case (note: if cancelled, we should not call this function)
		}
		g_error_free (err);
	}
	else up_client_get_devices_async (pUPowerClient, myData.pCancel, _on_got_devices, NULL);
	
	CD_APPLET_LEAVE ();
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

static void _on_device_added (UpClient *pClient, UpDevice *pDevice, gpointer data)
{
	CD_APPLET_ENTER;
	if (pClient != myData.pUPowerClient) // should not happen, not sure what to do
	{
		g_object_unref (myData.pUPowerClient);
		myData.pUPowerClient = NULL;
	}

	// if it's really a new device (yes, be secured...) and it's a battery
	if (g_list_find (myData.pBatteryDeviceList, pDevice) == NULL &&
		_cd_upower_add_and_ref_device_if_battery (pDevice, &myData.pBatteryDeviceList))
	{
		// update state: get all devices and all info
		_cd_upower_update_state (FALSE);
		// connect to be notified for changes
		g_signal_connect (pDevice, "notify", G_CALLBACK (_on_device_changed), NULL);
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
		_free_battery_dev (pDevice); // unref and disconnect signal

		// update state: get all devices and all info
		myData.pBatteryDeviceList = g_list_delete_link (myData.pBatteryDeviceList, pOldDevice);
		_cd_upower_update_state (FALSE);
	}
	CD_APPLET_LEAVE ();
}

static void _on_device_changed (G_GNUC_UNUSED UpDevice *pDevice, G_GNUC_UNUSED GParamSpec *pSpec, G_GNUC_UNUSED gpointer data)
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

// Can be called the first time (when getting the list of devices) or when a device is added/removed after.
static void _cd_upower_update_state (gboolean bFirstUpdate)
{
	if (myData.pUPowerClient == NULL)  // no UPower available, try to find the battery by ourselves.
	{
		cd_debug ("no UPower available");
		cd_check_power_files ();
	}
	else  // UPower is available, fetch the values we got from it.
	{
		// clear previous values
		g_free (myData.cTechnology);
		myData.cTechnology = NULL;
		g_free (myData.cVendor);
		myData.cVendor = NULL;
		g_free (myData.cModel);
		myData.cModel = NULL;
		
		// fetch the current values we got on the devices
		_fetch_current_values (myData.pBatteryDeviceList);
		
		// fetch static values of the devices, and watch for any change
		UpDevice *pDevice;
		GList *pItem;
		GString *sTechnology = NULL, *sVendor = NULL, *sModel = NULL;
		const gchar *cTechnology;
		gchar *cVendor, *cModel;
		gdouble fMaxAvailableCapacity = 0., fTmp;
		UpDeviceTechnology iTechnology;
		gboolean bFirst = TRUE;
		for (pItem = myData.pBatteryDeviceList ; pItem != NULL ; pItem = g_list_next (pItem))
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

			if (bFirstUpdate)
			{
				/* watch for any change. A priori, no need to watch the
				 * "onBattery" signal on the client, since we can deduce this
				 * property from the device state.
				 * A battery not present is still a valid device. So if we could
				 * find a battery device, it will stay here forever, so we don't
				 * need to watch for the destruction/creation of a battery device.
				 */
				g_signal_connect (pDevice, "notify", G_CALLBACK (_on_device_changed), NULL);
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

		if (bFirstUpdate)
		{
			myData.iSignalIDAdded = g_signal_connect (myData.pUPowerClient,
				"device-added", G_CALLBACK (_on_device_added), NULL);
			myData.iSignalIDRemoved = g_signal_connect (myData.pUPowerClient,
				"device-removed", G_CALLBACK (_on_device_removed), NULL);
			// Note: these signals (removed and added) are also send when resuming from suspend...
		}
	}
	
	// in any case, update the icon to show the current state we are in.
	update_icon ();
}

void cd_powermanager_start (void)
{
	myData.pCancel = g_cancellable_new ();
	up_client_new_async (myData.pCancel, _on_got_upower, NULL);
}

static void _free_battery_dev (gpointer ptr)
{
	GObject *obj = (GObject*)ptr;
	g_signal_handlers_disconnect_by_func (obj, _on_device_changed, NULL);
	g_object_unref (obj); // remove the ref we took on the device. it may or not destroy the object, that's why we disconnected manually the signal above.
}

void cd_upower_stop (void)
{
	if (myData.pCancel != NULL)
	{
		g_cancellable_cancel (myData.pCancel);
		g_object_unref (myData.pCancel);
		myData.pCancel = NULL;
	}
	
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

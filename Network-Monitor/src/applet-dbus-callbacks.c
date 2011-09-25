/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
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
// http://projects.gnome.org/NetworkManager/developers/spec.html

#define _BSD_SOURCE

#include <unistd.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-connections.h"
#include "applet-dbus-callbacks.h"

// NetworkManager's properties changed.
// Les proprietes de l'objet NetworkManager sont :
// WirelessEnabled - b - (readwrite)
//     Indicates if wireless is currently enabled or not. 
// WirelessHardwareEnabled - b - (read)
//     Indicates if the wireless hardware is currently enabled, i.e. the state of the RF kill switch. 
// ActiveConnections - ao - (read)
//     List of active connection object paths. (<-- devices)
// State - u - (read) (NM_STATE)
//     The overall state of the NetworkManager daemon.
void onChangeNMProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data)
{
	cd_debug ("%s ()\n", __func__);
	GValue *value;
	
	// on regarde quelles proprietes ont change.
	value = g_hash_table_lookup (properties, "ActiveConnections");  // device path, qui donnera un device object, qui contient des proprietes.
	if (value && G_VALUE_HOLDS (value, DBUS_TYPE_G_OBJECT_PATH))
	{
		cd_debug (" -> changement dans les connections actives\n");
		cd_NetworkMonitor_get_active_connection_info();
		cd_NetworkMonitor_draw_icon ();
	}
	
	value = g_hash_table_lookup (properties, "State");  // NM_STATE_UNKNOWN = 0, NM_STATE_ASLEEP = 1, NM_STATE_CONNECTING = 2, NM_STATE_CONNECTED = 3, NM_STATE_DISCONNECTED = 4
	if (value && G_VALUE_HOLDS_UINT (value))
	{
		cd_debug (" -> changement de l'etat de NM : %d\n", g_value_get_uint (value));
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_stop_icon_animation (myIcon);
		switch (g_value_get_uint (value))
		{
			case 0:  // NM_STATE_UNKNOWN
			default:
			break;
			
			case 1:  // NM_STATE_ASLEEP
				cairo_dock_show_temporary_dialog_with_icon (D_("Network connection state changed to inactive."), myIcon, myContainer, 4000, "same icon");
				myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
				cd_NetworkMonitor_draw_no_wireless_extension ();
			break;
			
			case 2:  // NM_STATE_CONNECTING
				cairo_dock_show_temporary_dialog_with_icon (D_("Connecting..."), myIcon, myContainer, 4000, "same icon");
				cairo_dock_request_icon_animation (myIcon, CAIRO_DOCK (myContainer), myConfig.cAnimation, 1e3);
				myData.iPreviousQuality = 0;
				cd_NetworkMonitor_draw_icon ();
			break;
			
			case 3:  // NM_STATE_CONNECTED
				cairo_dock_show_temporary_dialog_with_icon (D_("Network connection is established."), myIcon, myContainer, 4000, "same icon");
				myData.iPreviousQuality = 0;
				cd_NetworkMonitor_draw_icon ();
			break;
			
			case 4:  // NM_STATE_DISCONNECTED
				cairo_dock_show_temporary_dialog_with_icon (D_("Network connection state changed to disconnected."), myIcon, myContainer, 4000, "same icon");
				myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
				cd_NetworkMonitor_draw_no_wireless_extension ();
			break;
		}
		CD_APPLET_REDRAW_MY_ICON;  // on redessine pour le cas ou l'animation s'est arretee, sinon on reste "au milieu".
	}
	/*for device_path in props['ActiveConnections']:
        device = bus.get_object('org.freedesktop.NetworkManager', device_path)
        device_props = device.GetAll("org.freedesktop.NetworkManager.Connection.Active", dbus_interface="org.freedesktop.DBus.Properties")
        if device_props['Default']:
            return
        ap_path = device_props['SpecificObject']
        if ap_path.startswith('/org/freedesktop/NetworkManager/AccessPoint/'):
            ap = bus.get_object('org.freedesktop.NetworkManager', ap_path)
            ssid = ap.Get("org.freedesktop.NetworkManager.AccessPoint", "Ssid", dbus_interface="org.freedesktop.DBus.Properties")
            ssid = ''.join([chr(c) for c in ssid])
            if ssid not in config.sections():
                return
            print ssid
            device.connect_to_signal("PropertiesChanged", device_properties_changed_signal_handler(ssid), dbus_interface="org.freedesktop.NetworkManager.Connection.Active")*/
}


void onChangeWirelessDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *hProperties, gpointer data)
{
	cd_debug ("%s ()\n", __func__);
	GValue *value;
	
	value = g_hash_table_lookup (hProperties, "ActiveAccessPoint");
	if (G_VALUE_HOLDS (value, DBUS_TYPE_G_OBJECT_PATH))
	{
		g_free (myData.cAccessPoint);
		myData.cAccessPoint = NULL;
		
		gchar *cAccessPointPath = g_value_get_boxed (value);
		cd_debug ("Network-Monitor : New active point : %s\n", cAccessPointPath);
		
		if (cAccessPointPath && strncmp (cAccessPointPath, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
		{
			g_free (myData.cAccessPoint);
			myData.cAccessPoint = g_strdup (cAccessPointPath);
			
			cd_NetworkMonitor_get_new_access_point ();
		}
		else
		{
			cd_debug ("plus de point d'acces !\n");
			/// que faire ?...
		}
	}
}

void onChangeWiredDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *hProperties, gpointer data)
{
	cd_debug ("%s ()\n", __func__);
	GValue *v;
	v = g_hash_table_lookup (hProperties, "Carrier");
	if (G_VALUE_HOLDS_BOOLEAN (v))
	{
		gboolean bCablePlugged = g_value_get_boolean (v);
		cd_debug (">>> Network-Monitor :  cable branche : %d", bCablePlugged);
		cairo_dock_show_temporary_dialog_with_icon (bCablePlugged ? D_("A cable has been plugged") : D_("A cable has been unplugged"), myIcon, myContainer, 3000, "same icon");
	}
}


void onChangeAccessPointProperties (DBusGProxy *dbus_proxy, GHashTable *hProperties, gpointer data)
{
	cd_debug ("%s ()\n", __func__);
	cd_NetworkMonitor_fetch_access_point_properties (hProperties);
	
	cd_NetworkMonitor_draw_icon ();
}


void onChangeActiveConnectionProperties (DBusGProxy *dbus_proxy, GHashTable *hProperties, gpointer data)
{
	cd_debug ("%s ()\n", __func__);
	GValue *v;
	v = g_hash_table_lookup (hProperties, "Connection");
	if (G_VALUE_HOLDS (v, DBUS_TYPE_G_OBJECT_PATH))
	{
		cd_debug (">>> Network-Monitor : new connection path : %s", (gchar*)g_value_get_boxed (v));
	}
	v = g_hash_table_lookup (hProperties, "SpecificObject");
	if (G_VALUE_HOLDS (v, DBUS_TYPE_G_OBJECT_PATH))
	{
		cd_debug (">>> Network-Monitor : new SpecificObject : %s", (gchar*)g_value_get_boxed (v));
	}
	v = g_hash_table_lookup (hProperties, "State");
	if (G_VALUE_HOLDS_UINT (v))
	{
		cd_debug (">>> Network-Monitor : new state : %d", g_value_get_uint (v));
	}
	
}

void onNewConnection (DBusGProxy *dbus_proxy, const GValue *pNewConnectionPath, gpointer data)
{
	cd_debug ("%s (%s)\n", __func__, g_value_get_boxed (pNewConnectionPath));
	
	
}

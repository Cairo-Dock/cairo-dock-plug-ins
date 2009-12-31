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
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-connections.h"

#define CD_DBUS_TYPE_HASH_TABLE dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE)

#define _reset_proxy(p) if (p) {\
	g_object_unref (p);\
	p = NULL; }


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
static void onChangeNMProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data)
{
	g_print ("%s ()\n", __func__);
	GValue *value;
	
	// on regarde quelles proprietes ont change.
	value = g_hash_table_lookup (properties, "ActiveConnections");  // device path, qui donnera un device object, qui contient des proprietes.
	if (value && G_VALUE_HOLDS (value, DBUS_TYPE_G_OBJECT_PATH))
	{
		g_print (" -> changement dans les connections actives\n");
		cd_NetworkMonitor_get_active_connection_info();
		cd_NetworkMonitor_draw_icon ();
	}
	
	value = g_hash_table_lookup (properties, "State");  // NM_STATE_UNKNOWN = 0, NM_STATE_ASLEEP = 1, NM_STATE_CONNECTING = 2, NM_STATE_CONNECTED = 3, NM_STATE_DISCONNECTED = 4
	if (value && G_VALUE_HOLDS_UINT (value))
	{
		g_print (" -> changement dans l'etat de NM : %d\n", g_value_get_uint (value));
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_stop_icon_animation (myIcon);
		switch (g_value_get_uint (value))
		{
			case 0:  // NM_STATE_UNKNOWN
			default:
			break;
			
			case 1:  // NM_STATE_ASLEEP
				cairo_dock_show_temporary_dialog_with_icon (D_("Network connection has become inactive."), myIcon, myContainer, 4000, "same icon");
			break;
			
			case 2:  // NM_STATE_CONNECTING
				cairo_dock_show_temporary_dialog_with_icon (D_("Connecting..."), myIcon, myContainer, 6000, "same icon");
				cairo_dock_request_icon_animation (myIcon, myContainer, "rotate", 1e3);
			break;
			
			case 3:  // NM_STATE_CONNECTED
				cairo_dock_show_temporary_dialog_with_icon (D_("Network connection has been established."), myIcon, myContainer, 4000, "same icon");
			break;
			
			case 4:  // NM_STATE_DISCONNECTED
				cairo_dock_show_temporary_dialog_with_icon (D_("Network has been disconnected."), myIcon, myContainer, 4000, "same icon");
			break;
		}
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
gboolean cd_NetworkMonitor_connect_to_bus (void)
{
	g_print ("%s ()\n", __func__);
	//\_____________ On verifie la presence de NM sur le bus.
	if (! cairo_dock_dbus_detect_system_application("org.freedesktop.NetworkManager"))
		return FALSE;
	
	//\_____________ On recupere l'objet principal de NM.
	myData.dbus_proxy_NM = cairo_dock_create_new_system_proxy (
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager");
	g_return_val_if_fail (DBUS_IS_G_PROXY (myData.dbus_proxy_NM), FALSE);
	myData.dbus_proxy_NM_prop = cairo_dock_create_new_system_proxy (
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties");
	g_return_val_if_fail (DBUS_IS_G_PROXY (myData.dbus_proxy_NM_prop), FALSE);
	
	//\_____________ On se connecte aux signaux de base : wifi active (WirelessEnabled && WirelessHardwareEnabled ), etat de NM (State).
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, G_TYPE_VALUE ,G_TYPE_INVALID);  // enregistrement d'un marshaller specifique au signal (sinon impossible de le recuperer ni de le voir
	
	dbus_g_proxy_add_signal(myData.dbus_proxy_NM, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_NM, "PropertiesChanged",
		G_CALLBACK(onChangeNMProperties), NULL, NULL);
	
	return TRUE;
}


gboolean cd_NetworkMonitor_get_active_connection_info (void)
{
	g_print ("%s ()\n", __func__);
	//\_____________ on reset tout.
	myData.bWiredExt = myData.bWirelessExt = FALSE;
	g_free (myData.cDevice);
	myData.cDevice = NULL;
	g_free (myData.cAccessPoint);
	myData.cAccessPoint = NULL;
	_reset_proxy (myData.dbus_proxy_ActiveConnection);
	_reset_proxy (myData.dbus_proxy_Device);
	_reset_proxy (myData.dbus_proxy_ActiveAccessPoint);
	_reset_proxy (myData.dbus_proxy_WirelessDevice);
	_reset_proxy (myData.dbus_proxy_WiredDevice);
	
	DBusGProxy *dbus_proxy_ActiveConnection_temp = NULL;
	DBusGProxy *dbus_proxy_Device_temp = NULL;
	DBusGProxy *dbus_proxy_ActiveAccessPoint_temp = NULL;
	GError *erreur = NULL;
	
	gint j,k;
	GValue value = { 0 };
	GPtrArray *paActiveConnections = NULL;
	GPtrArray *paDevices = NULL;
	gchar *cActiveConnection, *cDevice;
	
	//\_____________ On recupere la liste des connexions disponibles (ce sont les configs tout-en-un de NM).
	paActiveConnections = (GPtrArray*) cairo_dock_dbus_get_property_as_boxed (myData.dbus_proxy_NM_prop, "org.freedesktop.NetworkManager", "ActiveConnections");
	g_print ("%d connections\n", paActiveConnections->len);
	for (j=0; j<paActiveConnections->len; j++)
	{
		cActiveConnection = (gchar *)g_ptr_array_index(paActiveConnections,j);
		g_print ("Network-Monitor : Active Connection path : %s\n", cActiveConnection);
		
		// on recupere les proprietes de la connexion.
		dbus_proxy_ActiveConnection_temp = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			cActiveConnection,
			"org.freedesktop.DBus.Properties");
		GHashTable *props = cairo_dock_dbus_get_all_properties (dbus_proxy_ActiveConnection_temp, "org.freedesktop.NetworkManager.Connection.Active");
		if (props == NULL)
		{
			g_object_unref (dbus_proxy_ActiveConnection_temp);
			continue;
		}
		
		// on regarde si c'est la connexion par defaut.
		GValue *v = g_hash_table_lookup (props, "Default");
		if (!v || !G_VALUE_HOLDS_BOOLEAN (v) || ! g_value_get_boolean (v))
		{
			g_hash_table_unref (props);
			g_object_unref (dbus_proxy_ActiveConnection_temp);
			continue;
		}
		g_print (" c'est la connexion par defaut\n");
		myData.cActiveConnection = g_strdup (cActiveConnection);
		
		// on recupere le SpecificObject qui contient le point d'acces courant.
		gchar *cAccessPointPath=NULL;
		v = g_hash_table_lookup (props, "SpecificObject");
		if (v && G_VALUE_HOLDS_BOXED (v))
		{
			cAccessPointPath = g_value_get_boxed (v);
			g_print (" cAccessPointPath : %s\n", cAccessPointPath);
			if (cAccessPointPath && strncmp (cAccessPointPath, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
			{
				dbus_proxy_ActiveAccessPoint_temp = cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					cAccessPointPath,
					"org.freedesktop.DBus.Properties");
			}
		}
		
		// on recupere le nom du service.
		const gchar *cServiceName=NULL;
		v = g_hash_table_lookup (props, "ServiceName");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cServiceName = g_value_get_string (v);
			g_print (" cServiceName : %s\n", cServiceName);
		}
		
		// on parcourt la liste des devices associes.
		v = g_hash_table_lookup (props, "Devices");
		if (v && G_VALUE_HOLDS_BOXED (v))
		{
			GPtrArray *paDevices = g_value_get_boxed (v);
			g_print (" %d devices\n", paDevices->len);
			for (k=0; k<paDevices->len; k++)
			{
				// on recupere le device.
				cDevice = (gchar *)g_ptr_array_index(paDevices,k);
				g_print (" device path : %s\n", cDevice);
				dbus_proxy_Device_temp = cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					cDevice,
					"org.freedesktop.DBus.Properties");
				
				// on regarde son type.
				guint iDeviceType = cairo_dock_dbus_get_property_as_uint (dbus_proxy_Device_temp, "org.freedesktop.NetworkManager.Device", "DeviceType");  // 1 : ethernet, 2 : wifi
				g_print (" device type : %d\n", iDeviceType);
				if (iDeviceType != 1 && iDeviceType != 2)  // ne nous insteresse pas.
					continue;
				
				// on recupere son interface.
				gchar *cInterface = cairo_dock_dbus_get_property_as_string (dbus_proxy_Device_temp, "org.freedesktop.NetworkManager.Device", "Interface");
				g_print (" interface :%s\n", cInterface);
				
				myData.cInterface = cInterface;
				myData.cDevice = g_strdup(cDevice);
				myData.cServiceName = g_strdup (cServiceName);
				myData.dbus_proxy_ActiveConnection = dbus_proxy_ActiveConnection_temp;
				myData.dbus_proxy_Device = dbus_proxy_Device_temp;
				if (cAccessPointPath && strncmp (cAccessPointPath, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
				{
					myData.cAccessPoint = g_strdup (cAccessPointPath);
					myData.dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
						"org.freedesktop.NetworkManager",
						cAccessPointPath,
						"org.freedesktop.DBus.Properties");
				}
				
				if (iDeviceType == 1)
				{
					g_print (" => Network-Monitor : Connexion filaire\n");
					myData.bWiredExt = TRUE;
					
					/* Recuperation de l'AP active */
					cd_NetworkMonitor_get_wired_connection_infos();
					
					/* Calcul de la qualite du signal */
					cd_NetworkMonitor_quality();
				}
				else if (iDeviceType == 2)
				{
					g_print (" => Network-Monitor : Connexion sans fil\n");
					myData.bWirelessExt = TRUE;
					
					/* Recuperation de l'AP active */
					cd_NetworkMonitor_get_wireless_connection_infos();
					
					/* Calcul de la qualite du signal */
					cd_NetworkMonitor_quality();
				}
				
				cd_NetworkMonitor_draw_icon ();
				
				break ;
			}  // fin de la liste des devices.
		}
		
		g_hash_table_unref (props);
		break;  // on prend la premierr connexion.
	}
	
	g_ptr_array_free(paActiveConnections,TRUE);
	return (myData.bWiredExt || myData.bWirelessExt);
}


// les proprietes d'un AccessPoint sont :
// Flags - u - (read)  (NM_802_11_AP_FLAGS)
//     Flags describing the capabilities of the access point.
// WpaFlags - u - (read) (NM_802_11_AP_SEC)
//     Flags describing the access point's capabilities according to WPA (Wifi Protected Access).
// RsnFlags - u - (read) (NM_802_11_AP_SEC)
//     Flags describing the access point's capabilities according to the RSN (Robust Secure Network) protocol.
// Ssid - ay - (read)
//     The Service Set Identifier identifying the access point.
// Frequency - u - (read)
//     The radio channel frequency in use by the access point, in MHz.
// HwAddress - s - (read)
//     The hardware address (BSSID) of the access point.
// Mode - u - (read) (NM_802_11_MODE)
//     Describes the operating mode of the access point.
// MaxBitrate - u - (read)
//     The maximum bitrate this access point is capable of, in kilobits/second (Kb/s).
// Strength - y - (read)
//     The current signal quality of the access point, in percent.
static inline void _get_access_point_properties (GHashTable *hProperties)
{
	GValue *v;
	v = (GValue *)g_hash_table_lookup (hProperties, "Strength");
	if (v != NULL && G_VALUE_HOLDS_UCHAR (v))
	{
		myData.iPercent = (gint) g_value_get_uchar (v);
		g_print ("Network-Monitor : Force du signal : %d\n", myData.iPercent);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (v != NULL && G_VALUE_HOLDS_STRING (v))
	{
		myData.cAccessPointHwAdress = g_strdup(g_value_get_string (v));
		g_print ("Network-Monitor : Adresse physique de l'AP active : %s", myData.cAccessPointHwAdress);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "Ssid");
	if (v != NULL && G_VALUE_HOLDS_BOXED (v))
	{
		g_print ("Network-Monitor : got boxed SSID\n");
		//char* temp = g_value_get_boxed (vProperties);
		//g_print ("Network-Monitor : SSID = '%s'\n", temp);
		
		GByteArray *a = g_value_get_boxed (v);
		g_print (" length : %d; data : %x\n ", a->len, a->data);
		myData.cESSID = g_new0 (gchar, a->len+1);
		for (int i = 0; i < a->len; i ++)
		{
			g_print ("%c", a->data[i]);
			myData.cESSID[i] = a->data[i];
		}
		g_print ("\n");
	}

	v = (GValue *)g_hash_table_lookup (hProperties, "MaxBitrate");  // in kilobits/second (Kb/s).
	if (v != NULL && G_VALUE_HOLDS_UINT (v))
	{
		myData.iSpeed = (gint) g_value_get_uint (v) / 8;  // Ko/s
		cd_debug("Network-Monitor : Max Bitrate au demarrage : %d",myData.iSpeed);
	}
}

void cd_NetworkMonitor_get_wireless_connection_infos (void)
{
	g_print ("%s ()\n", __func__);
	GHashTable *hProperties;
	
	//\_____________ On recupere le chemin du point d'acces courant.
	gchar *cAccessPoint = cairo_dock_dbus_get_property_as_object_path (myData.dbus_proxy_Device, "org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint");
	g_print ("Network-Monitor : cAccessPoint : %s (%s)\n", cAccessPoint, myData.cAccessPoint);
	
	if (! myData.cAccessPoint)
	{
		g_print (" pas encore de point d'acces, on le recupere du device\n");
		myData.cAccessPoint = g_strdup (cAccessPoint);
		myData.dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cAccessPoint,
			"org.freedesktop.DBus.Properties");
	}
	
	g_return_if_fail (myData.dbus_proxy_ActiveAccessPoint != NULL);
	
	//\_____________ On recupere les proprietes associees.
	hProperties = cairo_dock_dbus_get_all_properties (myData.dbus_proxy_ActiveAccessPoint, "org.freedesktop.NetworkManager.AccessPoint");
	g_return_if_fail (hProperties != NULL);
	
	_get_access_point_properties (hProperties);
	
	g_hash_table_unref (hProperties);
}

void cd_NetworkMonitor_get_wired_connection_infos (void)
{
	g_print ("%s ()\n", __func__);
	GHashTable *hProperties;
	GValue *v;
	
	//\_____________ On recupere les proprietes du device "wired" (ici il n'y en a qu'un seul, donc pas besoin de recuperer le courant d'abord.
	hProperties = cairo_dock_dbus_get_all_properties (myData.dbus_proxy_Device, "org.freedesktop.NetworkManager.Device.Wired");
	g_return_if_fail (hProperties != NULL);
	
	v = (GValue *)g_hash_table_lookup (hProperties, "Speed");
	if (v != NULL && G_VALUE_HOLDS_UINT (v))
	{
		myData.iSpeed = g_value_get_uint (v);
		cd_debug("Network-Monitor : Vitesse de connexion : %d",myData.iSpeed);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (v != NULL && G_VALUE_HOLDS_STRING (v))
	{
		myData.cAccessPointHwAdress = g_strdup(g_value_get_string (v));
		cd_debug("Network-Monitor : Adresse physique : %s",myData.cAccessPointHwAdress);
	}
	
	g_hash_table_unref(hProperties);
}


void cd_NetworkMonitor_quality (void)
{
	if (myData.bWirelessExt)
	{
		if (myData.iPercent <= 0)
			myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
		else if (myData.iPercent < 20)
			myData.iQuality = WIFI_QUALITY_VERY_LOW;
		else if (myData.iPercent < 40)
			myData.iQuality = WIFI_QUALITY_LOW;
		else if (myData.iPercent < 60)
			myData.iQuality = WIFI_QUALITY_MIDDLE;
		else if (myData.iPercent < 80)
			myData.iQuality = WIFI_QUALITY_GOOD;
		else
			myData.iQuality = WIFI_QUALITY_EXCELLENT;
	}
	else if (myData.bWiredExt)
		myData.iQuality = WIRED_CONNECTION;
}



void onChangeAccessPointProperties (DBusGProxy *dbus_proxy, GHashTable *hProperties, gpointer data)
{
	g_print ("%s ()\n", __func__);
	_get_access_point_properties (hProperties);
	
	cd_NetworkMonitor_draw_icon ();
}

void onChangeWirelessDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *AP_properties, gpointer data)
{
	g_print ("%s ()\n", __func__);
	GValue *value;
	
	value = g_hash_table_lookup (AP_properties, "ActiveAccessPoint");
	if (G_VALUE_HOLDS (value, DBUS_TYPE_G_OBJECT_PATH))
	{
		g_print ("Network-Monitor : New active point : %s\n", g_value_get_boxed (value));
		gchar *cAccessPointPath = g_value_get_boxed (value);
		if (cAccessPointPath && strncmp (cAccessPointPath, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
		{
			g_free (myData.cAccessPoint);
			myData.cAccessPoint = g_strdup (cAccessPointPath);
			
			if (myData.dbus_proxy_ActiveAccessPoint)
			{
				dbus_g_proxy_disconnect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
					G_CALLBACK(onChangeAccessPointProperties), NULL);
				g_object_unref (myData.dbus_proxy_ActiveAccessPoint);
			}
			
			myData.dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
				"org.freedesktop.NetworkManager",
				myData.cAccessPoint,
				"org.freedesktop.DBus.Properties");
			dbus_g_proxy_add_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
				G_CALLBACK(onChangeAccessPointProperties), NULL, NULL);
			
			cd_NetworkMonitor_get_wireless_connection_infos ();
		}
	}
}

void onChangeWiredDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *AP_properties, gpointer data)
{
	g_print ("%s ()\n", __func__);
	GValue *v;
	v = g_hash_table_lookup (AP_properties, "Carrier");
	if (G_VALUE_HOLDS_BOOLEAN (v))
	{
		gboolean bCablePlugged = g_value_get_boolean (v);
		g_print (">>> Network-Monitor :  cable branche : %d", bCablePlugged);
		cairo_dock_show_temporary_dialog_with_icon (bCablePlugged ? D_("A cable has been pluged") : D_("A cable has been unpluged"), myIcon, myContainer, 3000, "same icon");
	}
}



void cd_NetworkMonitor_connect_signals ()
{
	g_print ("%s ()\n", __func__);
	
	//\_____________ On se connecte aux signaux du wifi/cable.
	if (myData.bWirelessExt)
	{
		g_print (" on se connecte au wifi\n");
		// qualite du signal : Strength.
		if (myData.dbus_proxy_ActiveAccessPoint != NULL)
		{
			dbus_g_proxy_add_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
				G_CALLBACK(onChangeAccessPointProperties), NULL, NULL);
		}
		
		// point d'acces courant : ActiveAccessPoint.
		myData.dbus_proxy_WirelessDevice = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cDevice,
			"org.freedesktop.NetworkManager.Device.Wireless");
		dbus_g_proxy_add_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessDeviceProperties), NULL, NULL);
	}
	else if (myData.cDevice != NULL)
	{
		g_print (" on se connecte a l'ethernet\n");
		// cable branche ou pas : Carrier
		myData.dbus_proxy_WiredDevice = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cDevice,
			"org.freedesktop.NetworkManager.Device.Wired");
		dbus_g_proxy_add_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWiredDeviceProperties), NULL, NULL);
	}
}

void cd_NetworkMonitor_disconnect_signals()
{
	g_print ("%s ()\n", __func__);
	//dbus_g_proxy_disconnect_signal(myData.dbus_proxy_NM, "PropertiesChanged",
	//	G_CALLBACK(onChangeNMProperties), NULL);
	if (myData.bWirelessExt)
	{
		if (myData.dbus_proxy_ActiveAccessPoint != NULL)
			dbus_g_proxy_disconnect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
				G_CALLBACK(onChangeAccessPointProperties), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessDeviceProperties), NULL);
	}
	else
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWiredDeviceProperties), NULL);
	}
}



static void _on_select_access_point (GtkMenuItem *menu_item, gpointer data)
{
	g_return_if_fail (myData.pMenuAccessPoints != NULL);
	int iNumAccessPoint = GPOINTER_TO_INT (data);
	g_return_if_fail (iNumAccessPoint < myData.pMenuAccessPoints->len);
	
	gchar *cAccessPointPath = (gchar *)g_ptr_array_index (myData.pMenuAccessPoints, iNumAccessPoint);
	g_print ("on a choisit %s (%s; %s; %s)\n", cAccessPointPath, myData.cActiveConnection, myData.cDevice, cAccessPointPath);
	
	//ActivateConnection ( s: service_name, o: connection, o: device, o: specific_object )o
	GValue conn = {0};
	GValue dev = {0};
	GValue so = {0};
	
	GError *erreur = NULL;
	gchar *cConnection = NULL;
	dbus_g_proxy_call (myData.dbus_proxy_NM_prop, "ActivateConnection", &erreur,
		G_TYPE_STRING, myData.cServiceName,
		DBUS_TYPE_G_OBJECT_PATH, myData.cActiveConnection,
		DBUS_TYPE_G_OBJECT_PATH, myData.cDevice,
		DBUS_TYPE_G_OBJECT_PATH, cAccessPointPath,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cConnection,
		G_TYPE_INVALID);
	g_print ("connection set (-> %s)\n", cConnection);
	g_free (cConnection);
	g_ptr_array_free (myData.pMenuAccessPoints, TRUE);
	myData.pMenuAccessPoints = NULL;
	g_print ("ok\n");
}
GtkWidget * cd_NetworkMonitor_build_menu_with_access_points (void)
{
	g_return_val_if_fail (myData.dbus_proxy_WirelessDevice != NULL, NULL);
	GError *erreur = NULL;
	GPtrArray *pAccessPoints = NULL;
	
	dbus_g_proxy_call (myData.dbus_proxy_WirelessDevice, "GetAccessPoints", &erreur,
		G_TYPE_INVALID,
		dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &pAccessPoints,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	if (!pAccessPoints || pAccessPoints->len == 0)
		return NULL;
	
	GtkWidget *pMenu = gtk_menu_new ();
	gchar *cAccessPointPath;
	DBusGProxy *dbus_proxy_ActiveAccessPoint;
	GHashTable *hProperties;
	GValue *v;
	guint iPercent;
	gchar *cSsid;
	GtkWidget *pHBox;
	int i;
	for (i = 0; i < pAccessPoints->len; i ++)
	{
		cAccessPointPath = (gchar *)g_ptr_array_index (pAccessPoints, i);
		g_print ("%d) %s\n", i, cAccessPointPath);
		
		dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			cAccessPointPath,
			"org.freedesktop.DBus.Properties");
		
		hProperties = cairo_dock_dbus_get_all_properties (dbus_proxy_ActiveAccessPoint, "org.freedesktop.NetworkManager.AccessPoint");
		if (hProperties == NULL)
			continue;
		
		v = (GValue *)g_hash_table_lookup (hProperties, "Strength");
		if (v != NULL && G_VALUE_HOLDS_UCHAR (v))
		{
			iPercent = (gint) g_value_get_uchar (v);
		}
		
		v = (GValue *)g_hash_table_lookup (hProperties, "Ssid");
		if (v != NULL && G_VALUE_HOLDS_BOXED (v))
		{
			GByteArray *a = g_value_get_boxed (v);
			cSsid = g_strndup (a->data, a->len);
		}
		
		g_object_unref (dbus_proxy_ActiveAccessPoint);
		
		gchar *cImage = NULL;
		if (iPercent > 80)
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-5.svg";
		else if (iPercent > 60)
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-4.svg";
		else if (iPercent > 40)
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-3.svg";
		else if (iPercent > 20)
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-2.svg";
		else if (iPercent > 0)
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-1.svg";
		else
			cImage = MY_APPLET_SHARE_DATA_DIR"/link-0.svg";
			
		cairo_dock_add_in_menu_with_stock_and_data (cSsid, cImage, _on_select_access_point, pMenu, GINT_TO_POINTER (i));
		
		/// recuperer les flags, wpa flags, et rsn flags -> encrypted.
		/// et le mode -> ad_hoc
		/// et mettre une icone asociee dans une hbox...
	}
	if (myData.pMenuAccessPoints)
	{
		g_ptr_array_free (myData.pMenuAccessPoints, TRUE);
		myData.pMenuAccessPoints = NULL;
	}
	myData.pMenuAccessPoints = pAccessPoints;
	
	return pMenu;
}

/*
#!/usr/bin/env python

################################################################################
# OpenWifiAutoConnect - Stephane PUYBAREAU (puyb <at> puyb <dot> net) - 2008   # 
# Fully automated authentication to capture portal based wifi networks.        # 
# Config file: ~/.OpenWifiAutoConnect - Format: ini                            #
# Section are network SSID (names)                                             #
# key / value pair define user submited information (based on the html form)   #
#                                                                              # 
# This software is provided under the terms of the GPL v3 licence.             # 
# See http://www.gnu.org/licenses/gpl.html for more information                # 
#                                                                              # 
# This software use python dbus bindings, pynotify and BeautifulSoup modules   #
################################################################################


# The url the program will try to open hoping to be redirected to the portal
URL = 'http://perdu.com/'

import sys
import gobject
import dbus
import dbus.mainloop.glib
import urllib2, urllib
from BeautifulSoup import BeautifulSoup
import re
import ConfigParser
import os
import pynotify

def properties_changed_signal_handler(props):
    if not props.has_key('ActiveConnections'):
        return
    for device_path in props['ActiveConnections']:
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
            device.connect_to_signal("PropertiesChanged", device_properties_changed_signal_handler(ssid), dbus_interface="org.freedesktop.NetworkManager.Connection.Active")

def device_properties_changed_signal_handler(ssid):
    def handler(props):
        if not props.has_key('State'):
            return
        if props['State'] != 2:
            return
        print ssid
        section = dict(config.items(ssid))

        if login(section):
            txt = "Successfully logged on " + ssid
        else:
            txt = "Failled to log on " + ssid
        n = pynotify.Notification("Open Wifi Auto Connect", txt, "dialog-warning")
        n.set_urgency(pynotify.URGENCY_NORMAL)
        n.set_timeout(10)
        #n.add_action("clicked","Button text", callback_function, None)
        n.show()
    return handler


def login(values):
    # build an http fetcher that support cookies
    opener = urllib2.build_opener( urllib2.HTTPCookieProcessor() )
    urllib2.install_opener(opener)

    # try to open the portal page
    f = opener.open(URL)
    data = f.read()
    redirect_url = f.geturl().split('?')[0]
    f.close()
    if redirect_url == URL:
        return # Our request wasn't hijacked by the portal (maybe the wifi network isn't the default connection)

    # parse the portal page
    soup = BeautifulSoup(data)
    form = soup.find('form')
    if not form:
        return # There's no form on this page
    
    # creating the post values
    login_post = {}
    for input in form.findAll('input'):
        if input.has_key('name'):
            default = ''
            if input.has_key('type') and input['type'] == 'checkbox':
                default = 'on'
            login_post[input['name']] = input.has_key('value') and input['value'] or default

    login_post.update(values)

    # guessing the post url
    if not form.has_key('action'):
        url = redirect_url
    elif not form['action'].startswith('/'):
        url = '/'.join(redirect_url.split('/')[:-1]) + '/' + form['action']
    else:
        url = '/'.join(redirect_url.split('/')[:3]) + form['action']

    # GET ou POST ?
    postBody = None
    if form.has_key('method') and form['method'].lower() == 'post':
        postBody = urllib.urlencode(login_post)
    else:
        url += '?' + urllib.urlencode(login_post).replace(' ', '+')

    # submit the form
    f = opener.open(url, postBody)
    data = f.read()
    f.close()

    # Test if the login was a success
    f = opener.open(URL)
    data = f.read()
    status = f.geturl() == URL
    f.close()
    return status

if __name__ == '__main__':
    config = ConfigParser.RawConfigParser()
    config.read(os.path.expanduser('~/.OpenWifiAutoConnect'))

    pynotify.init( "Open Wifi Auto Connect" )

    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SystemBus()
    nm = bus.get_object("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager")
    nm.connect_to_signal("PropertiesChanged", properties_changed_signal_handler, dbus_interface="org.freedesktop.NetworkManager")

    loop = gobject.MainLoop()
    loop.run()
*/

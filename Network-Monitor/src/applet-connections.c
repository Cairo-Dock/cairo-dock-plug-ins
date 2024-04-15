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
// http://projects.gnome.org/NetworkManager/developers/spec-08.html

#include <unistd.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-dbus-callbacks.h"
#include "applet-connections.h"

#define _reset_proxy(p) if (p) {\
	g_object_unref (p);\
	p = NULL; }

static void g_cclosure_marshal_VOID__GHashTable_GHashTable (GClosure *c, GValue *r, guint n, const GValue *p, gpointer i, gpointer d)
{
	
}

gboolean cd_NetworkMonitor_connect_to_bus (void)
{
	cd_debug ("%s ()", __func__);
	//\_____________ On verifie la presence de NM sur le bus.
	if (! cairo_dock_dbus_detect_system_application("org.freedesktop.NetworkManager"))
		return FALSE;
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__GHashTable_GHashTable,
		G_TYPE_NONE, CD_DBUS_TYPE_HASH_TABLE_OF_HASH_TABLE, G_TYPE_INVALID);  // pour la methode GetSettings (il faut le faire avant de recuperer tout proxy, sinon les signaux ne passent plus !)
	
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
		G_TYPE_NONE, G_TYPE_HASH_TABLE ,G_TYPE_INVALID);  // enregistrement d'un marshaller specifique au signal (sinon impossible de le recuperer ni de le voir
	
	dbus_g_proxy_add_signal(myData.dbus_proxy_NM, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_NM, "PropertiesChanged",
		G_CALLBACK(onChangeNMProperties), NULL, NULL);
	
	//\_____________ On recupere l'objet des connections.
	myData.cServiceName = g_strdup ("org.freedesktop.NetworkManagerUserSettings");
	
	myData.dbus_proxy_Settings = cairo_dock_create_new_system_proxy (
		myData.cServiceName,
		"/org/freedesktop/NetworkManagerSettings",
		"org.freedesktop.NetworkManagerSettings");
	dbus_g_proxy_add_signal(myData.dbus_proxy_Settings, "NewConnection", DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_Settings, "NewConnection",
		G_CALLBACK(onNewConnection), NULL, NULL);
	
	return TRUE;
}

gboolean cd_NetworkMonitor_get_device (void)
{
	cd_debug ("%s ()", __func__);
	//\_____________ On recupere la liste des devices.
	GPtrArray *paDevices = cairo_dock_dbus_get_array (myData.dbus_proxy_NM, "GetDevices");
	g_return_val_if_fail (paDevices != NULL, FALSE);
	
	//\_____________ On choisit celui defini en conf, ou un par defaut (wifi de preference).
	cd_debug ("%d devices", paDevices->len);
	DBusGProxy *dbus_proxy_Device_prop;
	gchar *cDevice;
	uint i;
	for (i = 0; i < paDevices->len; i++)
	{
		// on recupere le device.
		cDevice = (gchar *)g_ptr_array_index(paDevices, i);
		dbus_proxy_Device_prop = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			cDevice,
			"org.freedesktop.DBus.Properties");
		if (!DBUS_IS_G_PROXY (dbus_proxy_Device_prop))
			continue;
		cd_debug (" device %s", cDevice);
		
		// on regarde son type.
		guint iDeviceType = cairo_dock_dbus_get_property_as_uint (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device", "DeviceType");  // 1 : ethernet, 2 : wifi
		cd_debug (" device type : %d", iDeviceType);
		if (iDeviceType != 1 && iDeviceType != 2)  // ne nous insteresse pas.
		{
			cd_debug (" useless device type\n");
			g_object_unref (dbus_proxy_Device_prop);
			continue;
		}
		
		// on recupere son interface.
		gchar *cInterface = cairo_dock_dbus_get_property_as_string (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device", "Interface");
		cd_debug (" interface :%s", cInterface);
		
		// on regarde si on doit le selectionner ou pas.
		if (myData.cDevice != NULL)  // on a deja trouve un device, on regarde si celui-ci convient mieux.
		{
			if (myConfig.cInterface && cInterface && strcmp (cInterface, myConfig.cInterface) == 0)  // c'est celui qu'on veut.
			{
				g_free (myData.cDevice);
				myData.cDevice = NULL;
				g_free (myData.cInterface);
				myData.cInterface = NULL;
				g_object_unref (myData.dbus_proxy_Device_prop);
				myData.dbus_proxy_Device_prop = NULL;
			}
			else if (iDeviceType == 2 && myData.bWiredExt)  // c'est un wifi alors que celui qu'on a deja est un ethernet, on le prend a sa place.
			{
				g_free (myData.cDevice);
				myData.cDevice = NULL;
				g_free (myData.cInterface);
				myData.cInterface = NULL;
				g_object_unref (myData.dbus_proxy_Device_prop);
				myData.dbus_proxy_Device_prop = NULL;
			}
		}
		if (myData.cDevice == NULL)  // aucun autre device, on selectionne celui-la.
		{
			cd_debug (" on selectionne ce device\n");
			myData.dbus_proxy_Device_prop = dbus_proxy_Device_prop;
			myData.cInterface = cInterface;
			myData.cDevice = g_strdup (cDevice);
			myData.bWiredExt = (iDeviceType == 1);
			myData.bWirelessExt = (iDeviceType == 2);
			if (myConfig.cInterface && cInterface && strcmp (cInterface, myConfig.cInterface) == 0)
			{
				cd_debug ("  c'est l'interface qu'on veut\n");
				break;
			}
		}
		else
		{
			g_free (cInterface);
			g_object_unref (dbus_proxy_Device_prop);
		}
	}
	g_ptr_array_free (paDevices, TRUE);  // on suppose qu'une GDestroyFunc a ete assignee au tableau.
	g_return_val_if_fail (myData.cDevice != NULL, FALSE);
	
	//\_____________ On complete le device.
	myData.dbus_proxy_Device = cairo_dock_create_new_system_proxy (
		"org.freedesktop.NetworkManager",
		myData.cDevice,
		"org.freedesktop.NetworkManager.Device");
	/// se conecter au changement de propriete State ?...
	
	if (myData.bWirelessExt)
	{
		// on se connecte au changement de la propriete ActiveAccessPoint.
		myData.dbus_proxy_WirelessDevice = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cDevice,
			"org.freedesktop.NetworkManager.Device.Wireless");
		dbus_g_proxy_add_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessDeviceProperties), NULL, NULL);
		
		cd_NetworkMonitor_get_wireless_connection_infos ();
	}
	else
	{
		// on se connecte au changement de la propriete Carrier.
		myData.dbus_proxy_WiredDevice = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cDevice,
			"org.freedesktop.NetworkManager.Device.Wired");
		dbus_g_proxy_add_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged",
			G_CALLBACK(onChangeWiredDeviceProperties), NULL, NULL);
		
		cd_NetworkMonitor_get_wired_connection_infos ();
	}
	
	return TRUE;
}

gboolean cd_NetworkMonitor_get_connection (void)
{
	cd_debug ("%s ()", __func__);
	
	myData.cServiceName = g_strdup ("org.freedesktop.NetworkManagerUserSettings");
	
	//\_____________ On recupere la liste des connexions disponibles (ce sont les configs tout-en-un de NM).
	DBusGProxy *dbus_proxy_Settings = cairo_dock_create_new_system_proxy (
		myData.cServiceName,
		"/org/freedesktop/NetworkManagerSettings",
		"org.freedesktop.NetworkManagerSettings");
	GPtrArray *paConnections = cairo_dock_dbus_get_array (dbus_proxy_Settings, "ListConnections");
	cd_debug ("%d connections", paConnections->len);
	
	//\_____________ On en choisit une.
	gchar *cConnection;
	uint i;
	for (i = 0; i < paConnections->len; i++)
	{
		cConnection = (gchar *)g_ptr_array_index(paConnections, i);
		cd_debug (" Connection path : %s", cConnection);
		
		myData.cConnection = g_strdup (cConnection);
	}
	
	g_ptr_array_free (paConnections, TRUE);
	g_object_unref (dbus_proxy_Settings);
	
	return (myData.cConnection != NULL);
}

gboolean cd_NetworkMonitor_get_active_connection_info (void)
{
	cd_debug ("%s ()", __func__);
	//\_____________ on reset tout.
	myData.bWiredExt = myData.bWirelessExt = FALSE;
	g_free (myData.cDevice);
	myData.cDevice = NULL;
	g_free (myData.cInterface);
	myData.cInterface = NULL;
	g_free (myData.cAccessPoint);
	myData.cAccessPoint = NULL;
	_reset_proxy (myData.dbus_proxy_ActiveConnection);
	_reset_proxy (myData.dbus_proxy_ActiveConnection_prop);
	_reset_proxy (myData.dbus_proxy_Device);
	_reset_proxy (myData.dbus_proxy_Device_prop);
	_reset_proxy (myData.dbus_proxy_ActiveAccessPoint);
	_reset_proxy (myData.dbus_proxy_ActiveAccessPoint_prop);
	_reset_proxy (myData.dbus_proxy_WirelessDevice);
	_reset_proxy (myData.dbus_proxy_WiredDevice);
	
	DBusGProxy *dbus_proxy_ActiveConnection_prop = NULL;
	DBusGProxy *dbus_proxy_Device_prop = NULL;
	
	uint j,k;
	GPtrArray *paActiveConnections = NULL;
	gchar *cActiveConnection, *cDevice, *cAccessPointPath, *cConnection;
	const gchar *cServiceName;
	
	//\_____________ On recupere la liste des connexions actives (ce sont les configs tout-en-un de NM qui sont actuellement utilisees).
	paActiveConnections = (GPtrArray*) cairo_dock_dbus_get_property_as_boxed (myData.dbus_proxy_NM_prop, "org.freedesktop.NetworkManager", "ActiveConnections");
	cd_debug ("%d connections", paActiveConnections->len);
	for (j=0; j < paActiveConnections->len; j++)
	{
		cActiveConnection = (gchar *)g_ptr_array_index(paActiveConnections,j);
		cd_debug ("Network-Monitor : Active Connection path : %s", cActiveConnection);
		
		// on recupere les proprietes de la connexion.
		dbus_proxy_ActiveConnection_prop = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			cActiveConnection,
			"org.freedesktop.DBus.Properties");
		GHashTable *props = cairo_dock_dbus_get_all_properties (dbus_proxy_ActiveConnection_prop, "org.freedesktop.NetworkManager.Connection.Active");
		if (props == NULL)
		{
			g_object_unref (dbus_proxy_ActiveConnection_prop);
			continue;
		}
		
		// on regarde si c'est la connexion par defaut.
		GValue *v = g_hash_table_lookup (props, "Default");
		if (!v || !G_VALUE_HOLDS_BOOLEAN (v) || ! g_value_get_boolean (v))
		{
			g_hash_table_unref (props);
			g_object_unref (dbus_proxy_ActiveConnection_prop);
			continue;
		}
		cd_debug (" c'est la connexion par defaut\n");
		myData.cActiveConnection = g_strdup (cActiveConnection);
		
		// on recupere le SpecificObject qui contient le point d'acces courant.
		cAccessPointPath=NULL;
		v = g_hash_table_lookup (props, "SpecificObject");
		if (v && G_VALUE_HOLDS_BOXED (v))
		{
			cAccessPointPath = g_value_get_boxed (v);
			cd_debug (" cAccessPointPath : %s", cAccessPointPath);
		}
		
		// on recupere le nom du service qui fournit cette connexion.
		cServiceName=NULL;
		v = g_hash_table_lookup (props, "ServiceName");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cServiceName = g_value_get_string (v);
			cd_debug (" cServiceName : %s", cServiceName);
		}
		
		// on recupere le chemin de la connection.
		cConnection=NULL;
		v = g_hash_table_lookup (props, "Connection");
		if (v && G_VALUE_HOLDS (v, DBUS_TYPE_G_OBJECT_PATH))
		{
			cConnection = g_value_get_boxed (v);
			cd_debug (" cConnectionPath : %s", cConnection);
		}
		
		// on parcourt la liste des devices associes.
		v = g_hash_table_lookup (props, "Devices");
		if (v && G_VALUE_HOLDS_BOXED (v))
		{
			GPtrArray *paDevices = g_value_get_boxed (v);
			cd_debug (" %d devices", paDevices->len);
			for (k=0;  k < paDevices->len; k++)
			{
				// on recupere le device.
				cDevice = (gchar *)g_ptr_array_index(paDevices,k);
				cd_debug (" device path : %s", cDevice);
				dbus_proxy_Device_prop = cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					cDevice,
					"org.freedesktop.DBus.Properties");
				
				// on regarde son type.
				guint iDeviceType = cairo_dock_dbus_get_property_as_uint (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device", "DeviceType");  // 1 : ethernet, 2 : wifi
				cd_debug (" device type : %d", iDeviceType);
				if (iDeviceType != 1 && iDeviceType != 2)  // ne nous insteresse pas.
				{
					g_object_unref (dbus_proxy_Device_prop);
					continue;
				}
				
				// on recupere son interface.
				gchar *cInterface = cairo_dock_dbus_get_property_as_string (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device", "Interface");
				cd_debug (" interface :%s", cInterface);
				
				// on garde toutes les infos en memoire.
				myData.cInterface = cInterface;
				myData.cDevice = g_strdup(cDevice);
				myData.cServiceName = g_strdup (cServiceName);
				myData.cConnection = g_strdup (cConnection);
				myData.dbus_proxy_ActiveConnection_prop = dbus_proxy_ActiveConnection_prop;
				myData.dbus_proxy_ActiveConnection =  cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					myData.cActiveConnection,
					"org.freedesktop.NetworkManager.Connection.Active");
				dbus_g_proxy_add_signal(myData.dbus_proxy_ActiveConnection, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
				dbus_g_proxy_connect_signal(myData.dbus_proxy_ActiveConnection, "PropertiesChanged",
					G_CALLBACK(onChangeActiveConnectionProperties), NULL, NULL);
				
				myData.dbus_proxy_Device_prop = dbus_proxy_Device_prop;
				myData.dbus_proxy_Device = cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					cDevice,
					"org.freedesktop.NetworkManager.Device");
				
				if (cAccessPointPath && strncmp (cAccessPointPath, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
				{
					myData.cAccessPoint = g_strdup (cAccessPointPath);
					myData.dbus_proxy_ActiveAccessPoint_prop = cairo_dock_create_new_system_proxy (
						"org.freedesktop.NetworkManager",
						cAccessPointPath,
						"org.freedesktop.DBus.Properties");
					myData.dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
						"org.freedesktop.NetworkManager",
						cAccessPointPath,
						"org.freedesktop.NetworkManager.AccessPoint");
					dbus_g_proxy_add_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
					dbus_g_proxy_connect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
						G_CALLBACK(onChangeAccessPointProperties), NULL, NULL);
				}
				
				if (iDeviceType == 1)
				{
					cd_debug (" => Network-Monitor : Connexion filaire\n");
					myData.bWiredExt = TRUE;
					
					// on se connecte au changement de la propriete Carrier.
					myData.dbus_proxy_WiredDevice = cairo_dock_create_new_system_proxy (
						"org.freedesktop.NetworkManager",
						myData.cDevice,
						"org.freedesktop.NetworkManager.Device.Wired");
					dbus_g_proxy_add_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
					dbus_g_proxy_connect_signal(myData.dbus_proxy_WiredDevice, "PropertiesChanged",
						G_CALLBACK(onChangeWiredDeviceProperties), NULL, NULL);
					
					// on recupere les proprietes de la carte reseau, et de son etat connecte ou non.
					cd_NetworkMonitor_get_wired_connection_infos();
				}
				else
				{
					cd_debug (" => Network-Monitor : Connexion sans fil\n");
					myData.bWirelessExt = TRUE;
					
					// on se connecte au changement de la propriete ActiveAccessPoint.
					myData.dbus_proxy_WirelessDevice = cairo_dock_create_new_system_proxy (
						"org.freedesktop.NetworkManager",
						myData.cDevice,
						"org.freedesktop.NetworkManager.Device.Wireless");
					dbus_g_proxy_add_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
					dbus_g_proxy_connect_signal(myData.dbus_proxy_WirelessDevice, "PropertiesChanged",
						G_CALLBACK(onChangeWirelessDeviceProperties), NULL, NULL);
					
					// Recuperation de l'AP active.
					cd_NetworkMonitor_get_wireless_connection_infos();
					
					// Calcul de la qualite du signal
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


void cd_NetworkMonitor_get_wireless_connection_infos (void)
{
	cd_debug ("%s ()", __func__);
	GHashTable *hProperties;
	GValue *v;
	
	g_free (myData.cAccessPointHwAdress);
	myData.cAccessPointHwAdress = NULL;
	myData.iSpeed = 0;
	
	//\_____________ On recupere les proprietes du device "wireless".
	hProperties = cairo_dock_dbus_get_all_properties (myData.dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device.Wireless");
	g_return_if_fail (hProperties != NULL);
	
	v = (GValue *)g_hash_table_lookup (hProperties, "Bitrate");
	if (v && G_VALUE_HOLDS_UINT (v))
	{
		myData.iSpeed = g_value_get_uint (v);
		cd_debug ("  Bitrate : %d\n",myData.iSpeed);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		myData.cAccessPointHwAdress = g_strdup(g_value_get_string (v));
		cd_debug ("  Adresse physique : %s\n",myData.cAccessPointHwAdress);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "ActiveAccessPoint");
	if (v && G_VALUE_HOLDS (v, DBUS_TYPE_G_OBJECT_PATH))
	{
		gchar *cAccessPoint = g_value_get_boxed (v);
		cd_debug ("  Access point : %s", cAccessPoint);
		if (cAccessPoint && strncmp (cAccessPoint, "/org/freedesktop/NetworkManager/AccessPoint/", 44) == 0)
		{
			g_free (myData.cAccessPoint);
			myData.cAccessPoint = g_strdup (cAccessPoint);
		}
	}
	
	g_hash_table_unref (hProperties);
	
	//\_____________ On recupere le point d'acces courant sur le bus.
	myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
	if (myData.cAccessPoint != NULL)
	{
		cd_NetworkMonitor_get_new_access_point ();
	}
}

void cd_NetworkMonitor_get_wired_connection_infos (void)
{
	cd_debug ("%s ()", __func__);
	GHashTable *hProperties;
	GValue *v;
	
	//\_____________ On recupere les proprietes du device "wired"
	hProperties = cairo_dock_dbus_get_all_properties (myData.dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device.Wired");
	g_return_if_fail (hProperties != NULL);
	
	v = (GValue *)g_hash_table_lookup (hProperties, "Speed");
	if (v != NULL && G_VALUE_HOLDS_UINT (v))
	{
		myData.iSpeed = g_value_get_uint (v);
		cd_debug("  Vitesse de connexion : %d",myData.iSpeed);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (v != NULL && G_VALUE_HOLDS_STRING (v))
	{
		myData.cAccessPointHwAdress = g_strdup(g_value_get_string (v));
		cd_debug("  Adresse physique : %s",myData.cAccessPointHwAdress);
	}
	
	myData.iQuality = WIRED_NO_CONNECTION;
	v = (GValue *)g_hash_table_lookup (hProperties, "Carrier");
	if (v != NULL && G_VALUE_HOLDS_BOOLEAN (v))
	{
		if (g_value_get_boolean (v))
			myData.iQuality = WIRED_CONNECTION;
		cd_debug("  cable branche : %d", g_value_get_boolean (v));
	}
	
	g_hash_table_unref (hProperties);
}


  //////////////////
 // ACCESS POINT //
//////////////////

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
void cd_NetworkMonitor_fetch_access_point_properties (GHashTable *hProperties)
{
	GValue *v;
	v = (GValue *)g_hash_table_lookup (hProperties, "Strength");
	if (v != NULL && G_VALUE_HOLDS_UCHAR (v))
	{
		myData.iPercent = MIN (100, (gint) g_value_get_uchar (v));  // pas clair si c'est deja des % ou s'il faut convertir par 100/255, des fois on se chope des 255 ...
		cd_debug ("Network-Monitor : Force du signal : %d %%", myData.iPercent);
		cd_NetworkMonitor_quality ();
		cd_NetworkMonitor_draw_icon ();
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (v != NULL && G_VALUE_HOLDS_STRING (v))
	{
		myData.cAccessPointHwAdress = g_strdup(g_value_get_string (v));
		cd_debug ("Network-Monitor : Adresse physique de l'AP active : %s", myData.cAccessPointHwAdress);
	}
	
	v = (GValue *)g_hash_table_lookup (hProperties, "Ssid");
	if (v != NULL && G_VALUE_HOLDS_BOXED (v))
	{
		GByteArray *a = g_value_get_boxed (v);
		myData.cESSID = g_new0 (gchar, a->len+1);
		for (uint i = 0; i < a->len; i ++)
		{
			myData.cESSID[i] = a->data[i];
		}
		cd_debug ("Network-Monitor : SSID : %s", myData.cESSID);
	}

	v = (GValue *)g_hash_table_lookup (hProperties, "MaxBitrate");  // in kilobits/second (Kb/s).
	if (v != NULL && G_VALUE_HOLDS_UINT (v))
	{
		myData.iSpeed = (gint) g_value_get_uint (v) / 8;  // Ko/s
		cd_debug("Network-Monitor : Max Bitrate au demarrage : %d",myData.iSpeed);
	}
}

void cd_NetworkMonitor_get_access_point_properties (void)
{
	//\_____________ On recupere les proprietes associees.
	GHashTable *hProperties = cairo_dock_dbus_get_all_properties (myData.dbus_proxy_ActiveAccessPoint_prop, "org.freedesktop.NetworkManager.AccessPoint");
	g_return_if_fail (hProperties != NULL);
	
	myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
	cd_NetworkMonitor_fetch_access_point_properties (hProperties);
	
	g_hash_table_unref (hProperties);
}

void cd_NetworkMonitor_get_new_access_point (void)
{
	myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
	if (myData.cAccessPoint != NULL)
	{
		cd_debug (" on recupere le nouveau point d'acces...\n");
		if (myData.dbus_proxy_ActiveAccessPoint)
		{
			dbus_g_proxy_disconnect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
				G_CALLBACK(onChangeAccessPointProperties), NULL);
			g_object_unref (myData.dbus_proxy_ActiveAccessPoint);
		}
		if (myData.dbus_proxy_ActiveAccessPoint_prop)
		{
			g_object_unref (myData.dbus_proxy_ActiveAccessPoint_prop);
		}
		
		myData.dbus_proxy_ActiveAccessPoint_prop = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cAccessPoint,
			"org.freedesktop.DBus.Properties");
		myData.dbus_proxy_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cAccessPoint,
			"org.freedesktop.NetworkManager.AccessPoint");
		dbus_g_proxy_add_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged", CD_DBUS_TYPE_HASH_TABLE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeAccessPointProperties), NULL, NULL);
		
		cd_NetworkMonitor_get_access_point_properties ();
	}
}

/*
#!/usr/bin/env python
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

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

#define _BSD_SOURCE

#include <unistd.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-connections.h"
#include "applet-menu.h"

static GList *cd_NetworkMonitor_get_connections_for_access_point (const gchar *cAccessPoint, const gchar *cDevice, const gchar *cSsid, const gchar *cHwAddress, int iMode, int iWirelessCapabilities, GPtrArray *paConnections, GPtrArray *paSettings)
{
	GList *pConnList = NULL;
	gchar *cConnection;
	GHashTable *pSettings, *pSubSettings;
	GValue *v;
	uint i;
	for (i = 0; i < paConnections->len; i++)
	{
		cConnection = (gchar *)g_ptr_array_index(paConnections, i);
		cd_debug (" Connection path : %s\n", cConnection);
		
		pSettings = g_ptr_array_index (paSettings, i);
		
		pSubSettings = g_hash_table_lookup (pSettings, "connection");
		if (pSubSettings == NULL)
			continue;
		
		const gchar *cType = NULL;
		v = g_hash_table_lookup (pSubSettings, "type");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cType = g_value_get_string (v);
			cd_debug (" type : %s\n", cType);
		}
		if (cType == NULL || strcmp (cType, "802-11-wireless") != 0)  // on veut du wifi.
			continue;
		
		const gchar *cID = NULL;
		v = g_hash_table_lookup (pSubSettings, "id");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cID = g_value_get_string (v);
			cd_debug (" id : %s\n", cID);
		}
		
		pSubSettings = g_hash_table_lookup (pSettings, "802-11-wireless");
		if (pSubSettings == NULL)
			continue;
		const gchar *cMode = NULL;
		v = g_hash_table_lookup (pSubSettings, "mode");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cMode = g_value_get_string (v);
			cd_debug (" mode : %s\n", cMode);
		}
		if (iMode && cMode)
		{
			if (iMode == 1 && strcmp (cMode, "ad-hoc") != 0)
				continue;
			if (iMode == 2 && strcmp (cMode, "infrastructure") != 0)
				continue;
		}
		
		gchar *cAPSsid = NULL;
		v = g_hash_table_lookup (pSubSettings, "ssid");
		if (v && G_VALUE_HOLDS_BOXED (v))
		{
			GByteArray *a = g_value_get_boxed (v);
			cAPSsid = g_strndup (a->data, a->len);
			cd_debug (" ssid : %s\n", cSsid);
		}
		if (cSsid == NULL || (cAPSsid != NULL && strcmp (cAPSsid, cSsid) != 0))  // le SSID est necessaire.
			continue;
		
		const gchar *cMacAddress = NULL;
		v = g_hash_table_lookup (pSubSettings, "mac-address");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cMacAddress = g_value_get_string (v);
			cd_debug (" mac address : %s\n", cMacAddress);
		}
		if (cHwAddress != NULL && cMacAddress != NULL && strcmp (cMacAddress, cHwAddress) != 0)
			continue;
		
		pSubSettings = g_hash_table_lookup (pSettings, "802-11-wireless-security");
		if (pSubSettings)
		{
			/// on verra plus tard ...
			
		}
		
		pConnList = g_list_prepend (pConnList, GINT_TO_POINTER (i));
	}
	
	return pConnList;
}

static GList *cd_NetworkMonitor_get_connections_for_wired_device (const gchar *cDevice, const gchar *cHwAddress, GPtrArray *paConnections)
{
	GList *pConnList = NULL;
	//\_____________ On cherche une connection qui ait le meme type (wifi ou filaire), et soit le meme SSID, soit la meme interface.
	gchar *cConnection;
	uint i;
	for (i = 0; i < paConnections->len; i++)
	{
		cConnection = (gchar *)g_ptr_array_index(paConnections, i);
		cd_debug (" Connection path : %s\n", cConnection);
		
	}
	
	return pConnList;
}


static void _on_select_access_point (GtkMenuItem *menu_item, CDMenuItemData *pItemData)
{
	if (pItemData == NULL || pItemData->cConnection == NULL)
	{
		/// il faut creer une connection ...
		cd_debug ("aucune des connexions existantes ne convient pour ce point d'acces\n");
		
		GHashTable *pSettings = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			(GDestroyNotify) g_hash_table_destroy);  // a table of tables.
		GHashTable *pSubSettings;
		
		// connection: type, id, uuid
		pSubSettings = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_free);
		g_hash_table_insert (pSettings, g_strdup ("connection"), pSubSettings);
		g_hash_table_insert (pSubSettings, g_strdup ("type"), g_strdup ("802-11-wireless"));
		g_hash_table_insert (pSubSettings, g_strdup ("id"), g_strdup_printf ("CD - %s", pItemData->cSsid));
		
		// 802-11-wireless: ssid, mode, seen-bssids
		pSubSettings = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_free);
		g_hash_table_insert (pSettings, g_strdup ("802-11-wireless"), pSubSettings);
		g_hash_table_insert (pSubSettings, g_strdup ("ssid"), g_strdup (pItemData->cSsid));
		g_hash_table_insert (pSubSettings, g_strdup ("mode"), g_strdup ("infrastructure"));
		
		// AddConnection
		DBusGProxy *dbus_proxy_Settings = cairo_dock_create_new_system_proxy (
			myData.cServiceName,
			"/org/freedesktop/NetworkManagerSettings",
			"org.freedesktop.NetworkManagerSettings");
		
		GError *erreur = NULL;
		dbus_g_proxy_call (dbus_proxy_Settings, "AddConnection", &erreur,
			CD_DBUS_TYPE_HASH_TABLE_OF_HASH_TABLE, pSettings,
			G_TYPE_INVALID,
			G_TYPE_INVALID);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			return ;
		}
		
		/// on attend le signal NewConnection ...
		
		
		
		/// on active la connexion...
		
		
		
	}
	else
	{
		cd_debug ("on a choisit (%s; %s; %s)\n", pItemData->cAccessPoint, pItemData->cDevice, pItemData->cConnection);
		
		//ActivateConnection ( s: service_name, o: connection, o: device, o: specific_object )o
		GError *erreur = NULL;
		GValue active_connection_path = G_VALUE_INIT;
		g_value_init (&active_connection_path, DBUS_TYPE_G_OBJECT_PATH);
		
		gchar *cNewActiveConnectionPath = NULL;
		dbus_g_proxy_call (myData.dbus_proxy_NM, "ActivateConnection", &erreur,
			G_TYPE_STRING, myData.cServiceName,
			DBUS_TYPE_G_OBJECT_PATH, pItemData->cConnection,
			DBUS_TYPE_G_OBJECT_PATH, pItemData->cDevice,
			DBUS_TYPE_G_OBJECT_PATH, pItemData->cAccessPoint,
			G_TYPE_INVALID,
			DBUS_TYPE_G_OBJECT_PATH, &cNewActiveConnectionPath,
			G_TYPE_INVALID);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			return ;
		}
		cd_debug (" => new active connection path : %s\n", cNewActiveConnectionPath);
	}
}

GtkWidget * cd_NetworkMonitor_build_menu_with_access_points (void)
{
	//\_____________ On recupere les connections existantes, ainsi que leur settings.
	DBusGProxy *dbus_proxy_Settings = cairo_dock_create_new_system_proxy (
		myData.cServiceName,
		"/org/freedesktop/NetworkManagerSettings",
		"org.freedesktop.NetworkManagerSettings");
	GPtrArray *paConnections = cairo_dock_dbus_get_array (dbus_proxy_Settings, "ListConnections");
	cd_debug ("%d connection(s)\n", paConnections ? paConnections->len : 0);
	g_object_unref (dbus_proxy_Settings);
	
	GPtrArray *paSettings = NULL;
	gchar *cConnection;
	if (paConnections != NULL && paConnections->len > 0)
	{
		paSettings = g_ptr_array_sized_new (paConnections->len);
		g_ptr_array_set_size (paSettings, paConnections->len);
		DBusGProxy *dbus_proxy_ConnectionSettings;
		GError *erreur = NULL;
		GHashTable *pSettingsTable;
		uint i;
		for (i = 0; i < paConnections->len; i++)
		{
			cConnection = (gchar *)g_ptr_array_index(paConnections, i);
			cd_debug (" Connection path : %s\n", cConnection);
			
			dbus_proxy_ConnectionSettings = cairo_dock_create_new_system_proxy (
				"org.freedesktop.NetworkManagerUserSettings",
				cConnection,
				"org.freedesktop.NetworkManagerSettings.Connection");
			erreur = NULL;
			pSettingsTable = NULL;
			dbus_g_proxy_call (dbus_proxy_ConnectionSettings, "GetSettings", &erreur,
				G_TYPE_INVALID,
				CD_DBUS_TYPE_HASH_TABLE_OF_HASH_TABLE, &pSettingsTable,
				G_TYPE_INVALID);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			paSettings->pdata[i] = pSettingsTable;
			
			g_object_unref (dbus_proxy_ConnectionSettings);
		}
	}
	
	//\_____________ On recupere la liste des devices.
	GPtrArray *paDevices = cairo_dock_dbus_get_array (myData.dbus_proxy_NM, "GetDevices");
	g_return_val_if_fail (paDevices != NULL, FALSE);
	cd_debug ("%d device(s)\n", paDevices->len);
	
	GtkWidget *pMenu = gtk_menu_new ();
	
	//\_____________ On parcourt tous les devices.
	GHashTable *pSsidTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		NULL);
	gchar *cDevice;
	DBusGProxy *dbus_proxy_Device_prop;
	guint iDeviceType;
	DBusGProxy *dbus_proxy_WirelessDevice, *dbus_proxy_WiredDevice;
	DBusGProxy *dbus_proxy_AccessPoint_prop;
	gchar *cAccessPointPath;
	GHashTable *hProperties;
	GValue *v;
	gint iPercent;
	gchar *cSsid = NULL;
	const gchar *cHwAddress;
	int iMode, iWirelessCapabilities;
	CDMenuItemData *pItemData;
	GtkWidget *pHBox;
	uint i, j;
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
		cd_debug (" device %s\n", cDevice);
		
		// on regarde son type.
		iDeviceType = cairo_dock_dbus_get_property_as_uint (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device", "DeviceType");  // 1 : ethernet, 2 : wifi
		cd_debug (" device type : %d\n", iDeviceType);
		if (iDeviceType != 1 && iDeviceType != 2)  // ne nous insteresse pas.
		{
			cd_debug (" useless device type\n");
			g_object_unref (dbus_proxy_Device_prop);
			continue;
		}
		
		if (iDeviceType == 2)
		{
			// On recupere ses proprietes.
			hProperties = cairo_dock_dbus_get_all_properties (dbus_proxy_Device_prop, "org.freedesktop.NetworkManager.Device.Wireless");
			
			const gchar *cAccessPointHwAdress = NULL;
			v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
			if (v && G_VALUE_HOLDS_STRING (v))
			{
				cAccessPointHwAdress = g_value_get_string (v);
			}
			
			int iMode = 0;
			v = (GValue *)g_hash_table_lookup (hProperties, "Mode");
			if (v && G_VALUE_HOLDS_UINT (v))
			{
				iMode = g_value_get_uint (v);
			}
			
			int iWirelessCapabilities = 0;
			v = (GValue *)g_hash_table_lookup (hProperties, "WirelessCapabilities");
			if (v && G_VALUE_HOLDS_UINT (v))
			{
				iWirelessCapabilities = g_value_get_uint (v);
			}
			
			// On recupere la liste des points d'acces.
			dbus_proxy_WirelessDevice = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
				cDevice,
				"org.freedesktop.NetworkManager.Device.Wireless");
			GError *erreur = NULL;
			GPtrArray *pAccessPoints = NULL;
			dbus_g_proxy_call (dbus_proxy_WirelessDevice, "GetAccessPoints", &erreur,
				G_TYPE_INVALID,
				dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &pAccessPoints,
				G_TYPE_INVALID);
			g_object_unref (dbus_proxy_WirelessDevice);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				g_object_unref (dbus_proxy_Device_prop);
				continue;
			}
			if (!pAccessPoints || pAccessPoints->len == 0)
			{
				cd_debug ("  aucun point d'acces\n");
				g_object_unref (dbus_proxy_Device_prop);
				/// ajouter une entree pour dire si le wifi est desactive ...
				
				continue;
			}
			
			// on insere chaque point d'acces dans le menu.
			for (j = 0; j < pAccessPoints->len; j ++)
			{
				// on recupere le point d'acces.
				cAccessPointPath = (gchar *)g_ptr_array_index (pAccessPoints, j);
				dbus_proxy_AccessPoint_prop = cairo_dock_create_new_system_proxy (
				"org.freedesktop.NetworkManager",
					cAccessPointPath,
					"org.freedesktop.DBus.Properties");
				
				// on recupere ses proprietes.
				hProperties = cairo_dock_dbus_get_all_properties (dbus_proxy_AccessPoint_prop, "org.freedesktop.NetworkManager.AccessPoint");
				if (hProperties == NULL)
				{
					g_object_unref (dbus_proxy_AccessPoint_prop);
					continue;
				}
				
				iPercent = 0;
				v = (GValue *)g_hash_table_lookup (hProperties, "Strength");
				if (v != NULL && G_VALUE_HOLDS_UCHAR (v))
				{
					iPercent = g_value_get_uchar (v);
				}
				
				v = (GValue *)g_hash_table_lookup (hProperties, "Ssid");
				if (v != NULL && G_VALUE_HOLDS_BOXED (v))
				{
					GByteArray *a = g_value_get_boxed (v);
					cSsid = g_strndup (a->data, a->len);
				}
				
				// on empeche les doublons.
				pItemData = (cSsid ? g_hash_table_lookup (pSsidTable, cSsid) : NULL);
				if (pItemData != NULL)
				{
					if (pItemData->iPercent > iPercent)
					{
						g_free (cSsid);
						g_object_unref (dbus_proxy_AccessPoint_prop);
					}
					else
					{
						g_free (pItemData->cAccessPoint);
						pItemData->cAccessPoint = g_strdup (cAccessPointPath);
					}
					continue;
				}
				
				cHwAddress = NULL;
				v = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
				if (v != NULL && G_VALUE_HOLDS_STRING (v))
				{
					cHwAddress = g_value_get_string (v);
				}
				
				iMode = 0;
				v = (GValue *)g_hash_table_lookup (hProperties, "Mode");
				if (v != NULL && G_VALUE_HOLDS_UINT (v))
				{
					iMode = g_value_get_uint (v);
				}
				
				iWirelessCapabilities = 0;
				v = (GValue *)g_hash_table_lookup (hProperties, "WpaFlags");
				if (v != NULL && G_VALUE_HOLDS_UINT (v))
				{
					iWirelessCapabilities = g_value_get_uint (v);
				}
				
				cd_debug ("%d) %s : %s (%s, %d%%)\n", j, cSsid, cAccessPointPath, cHwAddress, iPercent);
				
				const gchar *cImage = NULL;
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
				
				/// recuperer les flags, wpa flags, et rsn flags -> encrypted.
				/// et le mode -> ad_hoc
				/// et mettre une icone asociee dans une hbox...
				
				// on cherche une connection qui convienne.
				GList *pConnList = cd_NetworkMonitor_get_connections_for_access_point (cAccessPointPath, cDevice, cSsid, cHwAddress, iMode, iWirelessCapabilities, paConnections, paSettings);
				
				cd_debug ("%d connexion(s) satisfont a ce point d'acces\n", g_list_length (pConnList));
				
				if (pConnList == NULL || pConnList->next == NULL)
				{
					if (pItemData == NULL)
					{
						pItemData = g_new0 (CDMenuItemData, 1);
						g_hash_table_insert (pSsidTable, g_strdup (cSsid), pItemData);
					}
					else
					{
						g_free (pItemData->cAccessPoint);
					}
					pItemData->cDevice = g_strdup (cDevice);
					pItemData->iPercent = iPercent;
					pItemData->cAccessPoint = g_strdup (cAccessPointPath);
					pItemData->cSsid = g_strdup (cSsid);
					if (pConnList)
					{
						int n = GPOINTER_TO_INT (pConnList->data);
						pItemData->cConnection = g_strdup (g_ptr_array_index (paConnections, n));
					}
					cairo_dock_add_in_menu_with_stock_and_data (cSsid, cImage, G_CALLBACK (_on_select_access_point), pMenu, pItemData);
				}
				else
				{
					GtkWidget *pSubMenu = cairo_dock_create_sub_menu (cSsid, pMenu, cImage);
					GList *c;
					for (c = pConnList; c != NULL; c = c->next)
					{
						int n = GPOINTER_TO_INT (c->data);
						GHashTable *h = g_ptr_array_index (paSettings, n);
						if (!h)
							continue;
						GHashTable *hh = g_hash_table_lookup (h, "connection");
						if (!hh)
							continue;
						v = g_hash_table_lookup (hh, "id");
						if (v && G_VALUE_HOLDS_STRING (v))
						{
							const gchar *cID = g_value_get_string (v);
							
							pItemData = g_new0 (CDMenuItemData, 1);
							pItemData->cConnection = g_strdup (g_ptr_array_index (paConnections, n));
							pItemData->cDevice = g_strdup (cDevice);
							pItemData->cAccessPoint = g_strdup (cAccessPointPath);
							
							cairo_dock_add_in_menu_with_stock_and_data (cID, NULL, G_CALLBACK (_on_select_access_point), pSubMenu, GINT_TO_POINTER (n));
						}
					}
				}
				g_list_free (pConnList);
				
				g_object_unref (dbus_proxy_AccessPoint_prop);
			}
			g_ptr_array_free (pAccessPoints, TRUE);
		}
		else
		{
			/// si non connecte : ajouter une entree grisee.
			
			/// sinon ajouter une entree pour (des)activer ce device.
			
		}
	}
	g_ptr_array_free (paDevices, TRUE);
	g_ptr_array_free (paConnections, TRUE);
	g_hash_table_destroy (pSsidTable);
	
	return pMenu;
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
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


#define _pick_string(cValueName, cValue) \
	str = g_strstr_len (cOneInfopipe, -1, cValueName);\
	if (str) {\
		str += strlen (cValueName) + 1;\
		if (*str == ' ')\
			str ++;\
		if (*str == '"') {\
			str ++;\
			str2 = strchr (str, '"'); }\
		else {\
			str2 = strchr (str, ' '); }\
		if (str2) {\
			cValue = g_strndup (str, str2 - str);\
			cd_debug ("%s : %s", cValueName, cValue); } }
#define _pick_value(cValueName, iValue, iMaxValue)\
	str = g_strstr_len (cOneInfopipe, -1, cValueName);\
	if (str) {\
		str += strlen (cValueName) + 1;\
		iValue = atoi (str);\
		str2 = strchr (str, '/');\
		if (str2)\
			iMaxValue = atoi (str2+1);\
		cd_debug ("%s : %d (/%d)", cValueName, iValue, iMaxValue); }

void cd_wifi_get_data (gpointer data)
{
	myData.iPreviousQuality = myData.iQuality;
	myData.iQuality = -1;
	myData.iPrevPercent = myData.iPercent;
	myData.iPercent = -1;
	myData.iPrevSignalLevel = myData.iSignalLevel;
	myData.iSignalLevel = -1;
	myData.iPrevNoiseLevel = myData.iNoiseLevel;
	myData.iNoiseLevel = -1;
	g_free (myData.cESSID);
	myData.cESSID = NULL;
	g_free (myData.cInterface);
	myData.cInterface = NULL;
	g_free (myData.cAccessPoint);
	myData.cAccessPoint = NULL;
	
	/*myData.iPercent = g_random_int_range (0, 100);
	myData.iQuality = 5 * myData.iPercent/100;
	myData.cInterface = g_strdup ("toto");
	return;*/
	
	gchar *cResult = cairo_dock_launch_command_sync (MY_APPLET_SHARE_DATA_DIR"/wifi");
	if (cResult == NULL || *cResult == '\0')  // erreur a l'execution d'iwconfig (probleme de droit d'execution ou iwconfig pas installe) ou aucune interface wifi presente
	{ 
		g_free (cResult);
		return ;
	}
	
	gchar **cInfopipesList = g_strsplit (cResult, "\n", -1);
	g_free (cResult);
	gchar *cOneInfopipe, *str, *str2;
	int i, iMaxValue;
	for (i = 0; cInfopipesList[i] != NULL; i ++)
	{
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0' || *cOneInfopipe == '\n' )
			continue;
		
		if (myData.cInterface != NULL && *cOneInfopipe != ' ')  // nouvelle interface, on n'en veut qu'une.
			break ;
		
		if (myData.cInterface == NULL && *cOneInfopipe != ' ')
		{
			str = cOneInfopipe;  // le nom de l'interface est en debut de ligne.
			str2 = strchr (str, ' ');
			if (str2)
			{
				myData.cInterface = g_strndup (cOneInfopipe, str2 - str);
				cd_debug ("interface : %s", myData.cInterface);
			}
		}
		
		if (myData.cESSID == NULL)
		{
			_pick_string ("ESSID", myData.cESSID);  // eth1 IEEE 802.11g ESSID:"bla bla bla"
		}
		/*if (myData.cNickName == NULL)
		{
			_pick_string ("Nickname", myData.cNickName);
		}*/
		if (myData.cAccessPoint == NULL)
		{
			_pick_string ("Access Point", myData.cAccessPoint);
		}
		
		if (myData.iQuality == -1)  // Link Quality=54/100 Signal level=-76 dBm Noise level=-78 dBm OU Link Quality:5  Signal level:219  Noise level:177
		{
			iMaxValue = 0;
			_pick_value ("Link Quality", myData.iQuality, iMaxValue);
			if (iMaxValue != 0)  // vieille version, qualite indiquee en %
			{
				myData.iPercent = 100. * myData.iQuality / iMaxValue;
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
			else
			{
				myData.iPercent = 100. * myData.iQuality / (CONNECTION_NB_QUALITY-3);
			}
		}
		if (myData.iSignalLevel == -1)
		{
			_pick_value ("Signal level", myData.iSignalLevel, iMaxValue);
		}
		if (myData.iNoiseLevel == -1)
		{
			_pick_value ("Noise level", myData.iNoiseLevel, iMaxValue);
		}
	}
	g_strfreev (cInfopipesList);
}


gboolean cd_wifi_update_from_data (gpointer data)
{
	if (myData.cInterface != NULL)
	{
		myData.bWirelessExt = TRUE;
		cd_wifi_draw_icon ();
		cairo_dock_set_normal_task_frequency (myData.pTask);
	}
	else
	{
		myData.bWirelessExt = FALSE;
		cd_wifi_draw_no_wireless_extension ();
		cairo_dock_downgrade_task_frequency (myData.pTask);
	}
	return TRUE;
}


static void cd_NetworkMonitor_get_wireless_connection_infos (void)
{
	DBusGProxy *dbus_proxy;
	GError *erreur = NULL;

	GValue vActiveAccessPoints = { 0 };
	GPtrArray *paActiveAccessPoints = NULL;

	GValue *vProperties = { 0 };
	GHashTable *hProperties;
	
	dbus_g_proxy_call(myData.dbus_proxy_Device, "Get", &erreur,
		G_TYPE_STRING,"org.freedesktop.NetworkManager.Device.Wireless",
		G_TYPE_STRING,"ActiveAccessPoint",
		G_TYPE_INVALID,
		G_TYPE_VALUE, &vActiveAccessPoints,
		G_TYPE_INVALID);
						
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	/* Recuperation des infos sur l'AP active */
	if (G_VALUE_HOLDS (&vActiveAccessPoints, DBUS_TYPE_G_OBJECT_PATH))
	{	
		myData.cActiveAccessPoint = g_strdup (g_value_get_boxed (&vActiveAccessPoints));			
		cd_debug("Network-Monitor : AP active : %s",myData.cActiveAccessPoint);

		dbus_proxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cActiveAccessPoint,
			"org.freedesktop.DBus.Properties");
		
		if (dbus_proxy == NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}

		erreur= NULL;
		
		dbus_g_proxy_call(dbus_proxy, "GetAll", &erreur,
		G_TYPE_STRING,"org.freedesktop.NetworkManager.AccessPoint",
		G_TYPE_INVALID,
		(dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE)), &hProperties,
		G_TYPE_INVALID);
		
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}

		vProperties = (GValue *)g_hash_table_lookup (hProperties, "Strength");
		if (vProperties != NULL && G_VALUE_HOLDS_UCHAR (vProperties))
		{	
			myData.iPercent = (gint) g_value_get_uchar (vProperties);
			cd_debug("Network-Monitor : Force du signal au démarrage : %d",myData.iPercent);
		}
		
		vProperties = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
		if (vProperties != NULL && G_VALUE_HOLDS_STRING (vProperties))
		{	
			myData.cAccessPoint = g_strdup(g_value_get_string (vProperties));
			cd_debug("Network-Monitor : Adresse physique de l'AP active : %s",myData.cAccessPoint);
		}
		
		vProperties = (GValue *)g_hash_table_lookup (hProperties, "Ssid");
		if (vProperties != NULL && G_VALUE_HOLDS_BOXED (vProperties))
		{	
			char* temp = g_value_get_boxed (vProperties);
			gint len = strlen (temp);
			cd_debug("Network-Monitor : Taille du SSID : %d", len);
		}

		vProperties = (GValue *)g_hash_table_lookup (hProperties, "MaxBitrate");
		if (vProperties != NULL && G_VALUE_HOLDS_UINT (vProperties))
		{	
			myData.iSpeed = (gint) g_value_get_uint (vProperties);
			cd_debug("Network-Monitor : Max Bitrate au démarrage : %d",myData.iSpeed);
		}
		
		g_value_unset(vProperties);
		g_error_free(erreur);
		g_object_unref(dbus_proxy);
		g_hash_table_unref(hProperties);
	}
	
}

/* A finir si besoin */
static void cd_NetworkMonitor_get_wired_connection_infos (void)
{
	DBusGProxy *dbus_proxy;
	GError *erreur = NULL;
	
	GHashTable *hProperties;
	GValue *vProperties = { 0 };
	
	//cairo_dock_dbus_get_properties(dbus_proxy_Device, "Get", "org.freedesktop.NetworkManager.Device.Wired", "HwAddress", &vDevices);
	dbus_g_proxy_call(myData.dbus_proxy_Device, "GetAll", &erreur,
		G_TYPE_STRING,"org.freedesktop.NetworkManager.Device.Wired",
		//G_TYPE_STRING,"HwAddress",
		G_TYPE_INVALID,
		(dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE)), &hProperties,
		G_TYPE_INVALID);
						
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	
	vProperties = (GValue *)g_hash_table_lookup (hProperties, "Speed");
	if (vProperties != NULL && G_VALUE_HOLDS_UINT (vProperties))
	{	
		myData.iSpeed = g_value_get_uint (vProperties);
		cd_debug("Network-Monitor : Vitesse de connexion : %d",myData.iSpeed);
	}
	
	vProperties = (GValue *)g_hash_table_lookup (hProperties, "HwAddress");
	if (vProperties != NULL && G_VALUE_HOLDS_STRING (vProperties))
	{	
		myData.cAccessPoint = g_value_get_string (vProperties);
		cd_debug("Network-Monitor : Adresse physique : %s",myData.cAccessPoint);
	}
	
	g_error_free(erreur);
	g_object_unref(dbus_proxy);
	g_value_unset(vProperties);
	g_hash_table_unref(hProperties);
}



static void cd_NetworkMonitor_quality (void)
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


void onChangeWirelessProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data)
{
	GValue *value;
	
	value = g_hash_table_lookup (properties, "Strength");
	if (value != NULL && G_VALUE_HOLDS_UCHAR (value))
	{
		myData.iPercent = (gint) g_value_get_uchar(value);
		cd_debug("Network-Monitor : Nouvelle valeur de Strength : %u",myData.iPercent);
	}
	
	value = g_hash_table_lookup (properties, "MaxBitrate");
	if (value != NULL && G_VALUE_HOLDS_UINT (value))
	{
		myData.iSpeed = g_value_get_uint (value);
		cd_debug("Network-Monitor : Nouvelle valeur de MaxBitrate : %u",myData.iSpeed);
	}
	cd_NetworkMonitor_draw_icon ();
}


void cd_NetworkMonitor_connect_signals ()
{
	DBusGProxy *dbus_proxy_AP;
	
	if (myData.bWirelessExt)
	{
		dbus_proxy_AP = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cActiveAccessPoint,
			"org.freedesktop.NetworkManager.AccessPoint");
						
		dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__BOXED,
							G_TYPE_NONE, G_TYPE_VALUE ,G_TYPE_INVALID);			
					
		dbus_g_proxy_add_signal(dbus_proxy_AP, "PropertiesChanged",dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),G_TYPE_INVALID);

		dbus_g_proxy_connect_signal(dbus_proxy_AP, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessProperties), NULL, NULL);
		
		g_object_unref (dbus_proxy_AP);
	}
	
}


gboolean cd_NetworkMonitor_get_active_connection_info (void)
{	
	DBusGProxy *dbus_proxy_NM;
	DBusGProxy *dbus_proxy_ActiveConnection_temp;
	DBusGProxy *dbus_proxy_Device_temp;
	GError *erreur = NULL;
	
	gint i,j,k,l;
	
	char *cActiveConnection;
	GValue vConnections = { 0 };
	GPtrArray *paActiveConnections = NULL;
	
	char *cDevice;
	GValue vDevices = { 0 };
	GPtrArray *paDevices = NULL;

	GValue vInterface = { 0 };
	char *cInterface = NULL;
	
	GValue vType = { 0 };
	gint iDeviceType = 0;

	/* Recuperation de la liste des connexions actives */
	dbus_proxy_NM = cairo_dock_create_new_system_proxy (
          "org.freedesktop.NetworkManager",
          "/org/freedesktop/NetworkManager",
          "org.freedesktop.DBus.Properties");
    
	cairo_dock_dbus_get_properties(dbus_proxy_NM, "Get", "org.freedesktop.NetworkManager", "ActiveConnections", &vConnections);
	
	paActiveConnections = g_value_get_boxed (&vConnections);
	
	/* Parcours de la liste des connexions actives */	
	for (j=0; j<paActiveConnections->len; j++) 
	{
			/* Recuperation de la liste des Devices (HAL) */
			cActiveConnection = (gchar *)g_ptr_array_index(paActiveConnections,j);
			//cd_debug("Network-Monitor : Active Connection : %s",cActiveConnection);

			dbus_proxy_ActiveConnection_temp = cairo_dock_create_new_system_proxy (
				"org.freedesktop.NetworkManager",
				cActiveConnection,
				"org.freedesktop.DBus.Properties");
			
			cairo_dock_dbus_get_properties(dbus_proxy_ActiveConnection_temp, "Get", "org.freedesktop.NetworkManager.Connection.Active", "Devices", &vDevices);
			
			paDevices = g_value_get_boxed (&vDevices);
			/* Parcours de la liste des Devices */
			for (k=0; k<paDevices->len; k++) 
			{
				cDevice = (gchar *)g_ptr_array_index(paDevices,k);
				//cd_debug("Network-Monitor : Path associe : %s",cDevice);
				dbus_proxy_Device_temp = cairo_dock_create_new_system_proxy (
					"org.freedesktop.NetworkManager",
					cDevice,
					"org.freedesktop.DBus.Properties");

				cairo_dock_dbus_get_properties(dbus_proxy_Device_temp, "Get", "org.freedesktop.NetworkManager.Device", "Interface", &vInterface);
				cInterface = g_value_get_string (&vInterface);
				
				cairo_dock_dbus_get_properties(dbus_proxy_Device_temp, "Get", "org.freedesktop.NetworkManager.Device", "DeviceType", &vType);				
				iDeviceType = g_value_get_uint (&vType);
				
				/* Action selon le type de carte detectee */
				if (iDeviceType == 1)
				{
					cd_debug("Network-Monitor : Connexion filaire detectee");
					cd_debug("Network-Monitor : Device : %s",cDevice);
					myData.bWiredExt = TRUE;
					myData.cInterface = cInterface;
					myData.cDevice = g_strdup(cDevice);
					
					/* On recupere le path de la connection active ainsi que du device */
					myData.dbus_proxy_ActiveConnection = dbus_proxy_ActiveConnection_temp;
					myData.dbus_proxy_Device = dbus_proxy_Device_temp;
					
					/* Recuperation de l'AP active */
					cd_NetworkMonitor_get_wired_connection_infos();
					
					/* Calcul de la qualite du signal */	
					cd_NetworkMonitor_quality();
					
					
					cd_NetworkMonitor_draw_icon ();
				}
				else if (iDeviceType == 2)
				{
					cd_debug("Network-Monitor : Connexion sans fil detectee");
					cd_debug("Network-Monitor : Device : %s",cDevice);
					myData.bWirelessExt = TRUE;
					myData.cInterface = cInterface;
					myData.cDevice = g_strdup(cDevice);
					
					/* On recupere le path de la connection active ainsi que du device */
					myData.dbus_proxy_ActiveConnection = dbus_proxy_ActiveConnection_temp;
					myData.dbus_proxy_Device = dbus_proxy_Device_temp;
					
					/* Recuperation de l'AP active */
					cd_NetworkMonitor_get_wireless_connection_infos();
					cd_debug("Network-Monitor : apres recup des infos : %s", myData.cAccessPoint);
					/* Calcul de la qualite du signal */					
					cd_NetworkMonitor_quality();							
				}
				else
					cd_debug("Network-Monitor : Unknown card type ?");
			}
			g_ptr_array_free(paDevices,TRUE);
		}
	
	if (myData.bWiredExt && myData.bWirelessExt)
		myData.bWiredExt = FALSE; // Dans le cas où on a 2 connections, on force le wireless
		
	g_ptr_array_free(paActiveConnections,TRUE);
	g_object_unref (dbus_proxy_Device_temp);
	g_object_unref (dbus_proxy_ActiveConnection_temp);
	g_object_unref (dbus_proxy_NM);
	
	cd_debug("Network-Monitor : fin fonction d'init: %s", myData.cAccessPoint);

	
	return TRUE;
	
}

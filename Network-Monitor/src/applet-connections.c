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
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-connections.h"


DBusGProxy *dbus_proxy_signal_Device;
DBusGProxy *dbus_proxy_signal_AccessPoint;
DBusGProxy *dbus_proxy_signal_New_ActiveAccessPoint;


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

void cd_NetworkMonitor_get_data (gpointer data)
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


gboolean cd_NetworkMonitor_update_from_data (gpointer data)
{
	if (myData.cInterface != NULL)
	{
		myData.bWirelessExt = TRUE;
		cd_NetworkMonitor_draw_icon();
		cairo_dock_set_normal_task_frequency (myData.pTask);
	}
	else
	{
		myData.bWirelessExt = FALSE;
		cd_NetworkMonitor_draw_no_wireless_extension();
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
		myData.cAccessPoint = g_strdup(g_value_get_string (vProperties));
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


void onChangeDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data)
{
	GValue *value;
	
	value = g_hash_table_lookup (properties, "ActiveConnections");  // device path, qui donnera un device object, qui contient des proprietes.
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
	cd_debug("Network-Monitor :  Changement des connexions detectes");
	if (value != NULL && G_VALUE_HOLDS_BOXED (value))
	{
		cd_debug("Network-Monitor : Changement des connexions detectes et c est bien un BOXED");
	}

	cd_NetworkMonitor_get_active_connection_info();
	cd_NetworkMonitor_draw_icon ();
}

void onChangeActiveAccessPoint (DBusGProxy *dbus_proxy, GHashTable *AP_properties, gpointer data)
{
	GValue *value;
	
	value = g_hash_table_lookup (AP_properties, "ActiveAccessPoint");
	cd_debug("Network-Monitor :  Changement de l'active ap detecte");
	if (G_VALUE_HOLDS (value, DBUS_TYPE_G_OBJECT_PATH))
	{
		cd_debug("Network-Monitor : New AP : %s",g_value_get_string(value));
		cd_debug("Network-Monitor : Changement des connexions detectes et c est bien un BOXED");
	}

	cd_NetworkMonitor_get_active_connection_info();
	cd_NetworkMonitor_disconnect_signals();
	cd_NetworkMonitor_connect_signals();
	//cd_NetworkMonitor_draw_icon ();
}

void cd_NetworkMonitor_connect_signals ()
{
	/* Enregistrement d'un marshaller specifique au signal (sinon impossible de le récupérer ni de le voir */
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__BOXED,
										G_TYPE_NONE, G_TYPE_VALUE ,G_TYPE_INVALID);	
	
	/* Connexion au signal nous permettant de detecter si une nouvelle connexion physique active */
	dbus_proxy_signal_Device = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			"org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager");
	
	dbus_g_proxy_add_signal(dbus_proxy_signal_Device, "PropertiesChanged",dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),G_TYPE_INVALID);
	
	dbus_g_proxy_connect_signal(dbus_proxy_signal_Device, "PropertiesChanged",
			G_CALLBACK(onChangeDeviceProperties), NULL, NULL);
	
	if (myData.bWirelessExt)
	{
		/* Connexion au signal pour récupérer les nouvelles valeurs d'un signal WiFi */
		dbus_proxy_signal_AccessPoint = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cActiveAccessPoint,
			"org.freedesktop.NetworkManager.AccessPoint");	
								
		dbus_g_proxy_add_signal(dbus_proxy_signal_AccessPoint, "PropertiesChanged",dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),G_TYPE_INVALID);

		dbus_g_proxy_connect_signal(dbus_proxy_signal_AccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessProperties), NULL, NULL);
			
		/* Connexion au signal pour récupérer la nouvelle connexion active */	
		dbus_proxy_signal_New_ActiveAccessPoint = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			myData.cDevice,
			"org.freedesktop.NetworkManager.Device.Wireless");	
								
		dbus_g_proxy_add_signal(dbus_proxy_signal_New_ActiveAccessPoint, "PropertiesChanged",dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),G_TYPE_INVALID);

		dbus_g_proxy_connect_signal(dbus_proxy_signal_New_ActiveAccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeActiveAccessPoint), NULL, NULL);	
	}
}


void cd_NetworkMonitor_disconnect_signals()
{
	dbus_g_proxy_disconnect_signal(dbus_proxy_signal_Device, "PropertiesChanged",
			G_CALLBACK(onChangeDeviceProperties), NULL);
	if (myData.bWirelessExt)
	{
		dbus_g_proxy_disconnect_signal(dbus_proxy_signal_AccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessProperties), NULL);
		dbus_g_proxy_disconnect_signal(dbus_proxy_signal_New_ActiveAccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeActiveAccessPoint), NULL);	
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
		g_print ("Network-Monitor : Active Connection : %s\n", cActiveConnection);

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
			cInterface = g_strdup(g_value_get_string (&vInterface));
			
			cairo_dock_dbus_get_properties(dbus_proxy_Device_temp, "Get", "org.freedesktop.NetworkManager.Device", "DeviceType", &vType);				
			iDeviceType = g_value_get_uint (&vType);
			
			/* Action selon le type de carte detectee */
			g_print ("Network-Monitor : Device : %s, cInterface : %s\n",cDevice, cInterface);
			if (iDeviceType == 1)
			{
				g_print ("=> Network-Monitor : Connexion filaire\n");
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
				g_print ("Network-Monitor : Connexion sans fil\n");
				myData.bWirelessExt = TRUE;
				myData.cInterface = g_strdup(cInterface);
				myData.cDevice = g_strdup(cDevice);
				
				/* On recupere le path de la connection active ainsi que du device */
				myData.dbus_proxy_ActiveConnection = dbus_proxy_ActiveConnection_temp;
				myData.dbus_proxy_Device = dbus_proxy_Device_temp;
				
				/* Recuperation de l'AP active */
				cd_NetworkMonitor_get_wireless_connection_infos();
				/* Calcul de la qualite du signal */					
				cd_NetworkMonitor_quality();						
			}
			else
			{
				cd_debug("Network-Monitor : Unknown card type ?");
				// Dessin pour aucune connexion trouvee à faire
			}
		}
		g_object_unref (dbus_proxy_Device_temp);
		g_object_unref (dbus_proxy_ActiveConnection_temp);
		g_ptr_array_free(paDevices,TRUE);
	}
	
	if (myData.bWiredExt && myData.bWirelessExt)
		myData.bWiredExt = FALSE; // Dans le cas où on a 2 connections, on force le wireless
		
	g_ptr_array_free(paActiveConnections,TRUE);
	g_object_unref (dbus_proxy_NM);
	
	return TRUE;
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

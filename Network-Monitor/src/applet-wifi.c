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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-netspeed.h"
#include "applet-wifi.h"


#define _pick_string(cValueName, cValue) \
	str = g_strstr_len (cOneInfopipe, -1, cValueName);\
	if (str) {\
		str += strlen (cValueName) + 1;\
		while (*str == ' ')\
			str ++;\
		if (*str == '"') {\
			str ++;\
			str2 = strchr (str, '"'); }\
		else {\
			str2 = strchr (str, ' '); }\
		if (str2) {\
			cValue = g_strndup (str, str2 - str); }\
		else {\
			cValue = g_strdup (str); }\
		cd_debug ("%s : %s", cValueName, cValue); }
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
	myData.wifi._iPreviousQuality = myData.wifi._iQuality;
	myData.wifi._iQuality = -1;  // it's an uint, so actually it's a big number; we don't care, we just want to know if we already got a value or not.
	myData.wifi._iPrevPercent = myData.wifi._iPercent;
	myData.wifi._iPercent = -1;
	myData.wifi._iPrevSignalLevel = myData.wifi._iSignalLevel;
	myData.wifi._iSignalLevel = -1;
	myData.wifi._iPrevNoiseLevel = myData.wifi._iNoiseLevel;
	myData.wifi._iNoiseLevel = -1;
	g_free (myData.wifi._cESSID);
	myData.wifi._cESSID = NULL;
	g_free (myData.wifi._cInterface);
	myData.wifi._cInterface = NULL;
	g_free (myData.wifi._cAccessPoint);
	myData.wifi._cAccessPoint = NULL;
	
	/*myData.wifi._iPercent = g_random_int_range (0, 100);  // test
	myData.wifi._iQuality = 5 * myData.wifi._iPercent/100;
	myData.wifi._cInterface = g_strdup ("toto");
	return;*/
	
	gchar *cResult = cairo_dock_launch_command_sync (MY_APPLET_SHARE_DATA_DIR"/wifi");
	if (cResult == NULL || *cResult == '\0')  // erreur a l'execution d'iwconfig (probleme de droit d'execution ou iwconfig pas installe) ou aucune interface wifi presente
	{
		g_free (cResult);
		return ;
		/*cResult = g_strdup ("lo no wireless extensions.\n\
\n\
eth0 no wireless extensions.\n\
\n\
wlan0 IEEE 802.11abg ESSID:\"NETXHO\"\n\
Mode:Managed Frequency:2.452 GHz Access Point: 00:24:2B:48:07:21\n\
Bit Rate=54 Mb/s Tx-Power=15 dBm\n\
Retry long limit:7 RTS thr:off Fragment thr:off\n\
Encryption key:C6DA-6974-1612-DA99-3049-FDC0-1399-23BA-894E-67B0-9C8B-72C7-EE5B-5876-5C58-331B [2]\n\
Power Management:off\n\
Link Quality=52/70 Signal level=-58 dBm Noise level=-127 dBm\n\
Rx invalid nwid:0 Rx invalid crypt:0 Rx invalid frag:0\n\
Tx excessive retries:0 Invalid misc:0 Missed beacon:0\n\
\n\
vboxnet0 no wireless extensions.\n\
\n\
pan0 no wireless extensions.");*/
	}
	
	// types de sortie possibles :
	//eth0 no wireless extensions.
	//
	//eth2 IEEE 802.11 Nickname:""
	//Access Point: Not-Associated
	//
	//wlan0 IEEE 802.11abg ESSID:"NETXHO"
	//Mode:Managed Frequency:2.452 GHz Access Point: 00:24:2B:48:07:21
	//Bit Rate=54 Mb/s Tx-Power=15 dBm
	//Retry long limit:7 RTS thr:off Fragment thr:off
	//Encryption key:C6DA-6974-1612-DA99-3049-FDC0-1399-23BA-894E-67B0-9C8B-72C7-EE5B-5876-5C58-331B [2]
	//Power Management:off
	//Link Quality=52/70 Signal level=-58 dBm Noise level=-127 dBm
	//Rx invalid nwid:0 Rx invalid crypt:0 Rx invalid frag:0
	//Tx excessive retries:0 Invalid misc:0 Missed beacon:0
	gchar **cInfopipesList = g_strsplit (cResult, "\n", -1);
	g_free (cResult);
	gchar *cOneInfopipe, *str, *str2;
	int i, iMaxValue = 0;
	for (i = 0; cInfopipesList[i] != NULL; i ++)
	{
		cOneInfopipe = cInfopipesList[i];
		//g_print (" > %s\n", cOneInfopipe);
		if (*cOneInfopipe == '\0' || *cOneInfopipe == '\n' )  // saut de ligne signalant une nouvelle interface.
		{
			if (myData.wifi._cInterface != NULL)  // comme on n'en veut qu'une on quitte.
				break;  /// il faudra prendre celle en conf...
			else
				continue;
		}
		
		if (myData.wifi._cInterface == NULL)  // on n'a pas encore d'interface valable.
		{
			str = strchr (cOneInfopipe, ' ');  // le nom de l'interface est en debut de ligne.
			if (str)
			{
				str2 = str + 1;
				while (*str2 == ' ')
					str2 ++;
				if (strncmp (str2, "no wireless", 11) != 0)
					myData.wifi._cInterface = g_strndup (cOneInfopipe, str - cOneInfopipe);
			}
			cd_debug ("interface : %s", myData.wifi._cInterface);
			if (myData.wifi._cInterface == NULL)  // cette ligne ne nous a rien apporte, on passe a la suivante.
				continue;
		}
		
		if (myData.wifi._cESSID == NULL)
		{
			_pick_string ("ESSID", myData.wifi._cESSID);  // eth1 IEEE 802.11g ESSID:"bla bla bla"
		}
		/*if (myData.wifi._cNickName == NULL)
		{
			_pick_string ("Nickname", myData.wifi._cNickName);
		}*/
		if (myData.wifi._cAccessPoint == NULL)
		{
			_pick_string ("Access Point", myData.wifi._cAccessPoint);
		}

		if (myData.wifi._iQuality == (uint)-1)  // Link Quality=54/100 Signal level=-76 dBm Noise level=-78 dBm OU Link Quality:5  Signal level:219  Noise level:177
		{
			_pick_value ("Link Quality", myData.wifi._iQuality, iMaxValue);
			if (iMaxValue != 0)  // vieille version, qualite indiquee en %
			{
				myData.wifi._iPercent = 100. * myData.wifi._iQuality / iMaxValue;
				if (myData.wifi._iPercent <= 0)
					myData.wifi._iQuality = WIFI_QUALITY_NO_SIGNAL;
				else if (myData.wifi._iPercent < 20)
					myData.wifi._iQuality = WIFI_QUALITY_VERY_LOW;
				else if (myData.wifi._iPercent < 40)
					myData.wifi._iQuality = WIFI_QUALITY_LOW;
				else if (myData.wifi._iPercent < 60)
					myData.wifi._iQuality = WIFI_QUALITY_MIDDLE;
				else if (myData.wifi._iPercent < 80)
					myData.wifi._iQuality = WIFI_QUALITY_GOOD;
				else
					myData.wifi._iQuality = WIFI_QUALITY_EXCELLENT;
			}
			else
			{
				myData.wifi._iPercent = 100. * myData.wifi._iQuality / (WIFI_QUALITY_EXCELLENT);
			}
		}
		if (myData.wifi._iSignalLevel == -1)
		{
			_pick_value ("Signal level", myData.wifi._iSignalLevel, iMaxValue);
		}
		if (myData.wifi._iNoiseLevel == -1)
		{
			_pick_value ("Noise level", myData.wifi._iNoiseLevel, iMaxValue);
		}
	}
	g_strfreev (cInfopipesList);
}

#define cd_wifi_draw_icon cd_NetworkMonitor_draw_icon
#define cd_wifi_draw_no_wireless_extension cd_NetworkMonitor_draw_no_wireless_extension

gboolean cd_wifi_update_from_data (gpointer data)
{
	// recopie des infos qu'on veut pouvoir exploiter en dehors de la tache periodique.
	myData.wifi.iQuality = myData.wifi._iQuality;
	g_free (myData.wifi.cInterface);
	myData.wifi.cInterface = myData.wifi._cInterface;
	myData.wifi._cInterface = NULL;
	g_free (myData.wifi.cAccessPoint);
	myData.wifi.cAccessPoint = myData.wifi._cAccessPoint;
	myData.wifi._cAccessPoint = NULL;
	g_free (myData.wifi.cESSID);
	myData.wifi.cESSID = myData.wifi._cESSID;
	myData.wifi.cESSID = NULL;
	
	if (myData.wifi.cInterface != NULL)
	{
		cd_debug ("wifi sur %s", myData.wifi.cInterface);
		myData.wifi.bWirelessExt = TRUE;
		cd_wifi_draw_icon ();
		cairo_dock_set_normal_task_frequency (myData.wifi.pTask);
	}
	else
	{
		cd_debug ("no wifi\n");
		myData.wifi.bWirelessExt = FALSE;
		cd_wifi_draw_no_wireless_extension ();
		cairo_dock_downgrade_task_frequency (myData.wifi.pTask);
	}
	return TRUE;
}


void cd_netmonitor_launch_wifi_task (GldiModuleInstance *myApplet)
{
	cd_netmonitor_free_netspeed_task (myApplet);
	myData.iPreviousQuality = -2;
	
	if (myData.wifi.pTask == NULL)  // la tache n'existe pas, on la cree et on la lance.
	{
		myData.wifi.pTask = cairo_dock_new_task (myConfig.iWifiCheckInterval,
			(CairoDockGetDataAsyncFunc) cd_wifi_get_data,
			(CairoDockUpdateSyncFunc) cd_wifi_update_from_data,
			myApplet);
		cairo_dock_launch_task (myData.wifi.pTask);
	}
	else  // la tache existe, on la relance immediatement, avec la nouvelle frequence eventuellement.
	{
		cairo_dock_relaunch_task_immediately (myData.wifi.pTask, myConfig.iWifiCheckInterval);
	}
}

void cd_netmonitor_free_wifi_task (GldiModuleInstance *myApplet)
{
	if (myData.wifi.pTask != NULL)
	{
		cairo_dock_free_task (myData.wifi.pTask);
		myData.wifi.pTask = NULL;
	}
}

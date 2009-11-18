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
#include "applet-wifi.h"


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
	myData.wifi._iPreviousQuality = myData.wifi._iQuality;
	myData.wifi._iQuality = -1;
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
		
		if (myData.wifi._cInterface != NULL && *cOneInfopipe != ' ')  // nouvelle interface, on n'en veut qu'une.
			break ;  /// il faudra prendre celle en conf...
		
		if (myData.wifi._cInterface == NULL && *cOneInfopipe != ' ')
		{
			str = cOneInfopipe;  // le nom de l'interface est en debut de ligne.
			str2 = strchr (str, ' ');
			if (str2)
			{
				myData.wifi._cInterface = g_strndup (cOneInfopipe, str2 - str);
				cd_debug ("interface : %s", myData.wifi._cInterface);
			}
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
		
		if (myData.wifi._iQuality == -1)  // Link Quality=54/100 Signal level=-76 dBm Noise level=-78 dBm OU Link Quality:5  Signal level:219  Noise level:177
		{
			iMaxValue = 0;
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
	
	if (myData.wifi._cInterface != NULL)
	{
		myData.wifi.bWirelessExt = TRUE;
		cd_wifi_draw_icon ();
		cairo_dock_set_normal_task_frequency (myData.wifi.pTask);
	}
	else
	{
		myData.wifi.bWirelessExt = FALSE;
		cd_wifi_draw_no_wireless_extension ();
		cairo_dock_downgrade_task_frequency (myData.wifi.pTask);
	}
	return TRUE;
}


void cd_netmonitor_launch_wifi_task (CairoDockModuleInstance *myApplet)
{
	if (myData.netSpeed.pTask == NULL)  // la tache n'existe pas, on la cree et on la lance.
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

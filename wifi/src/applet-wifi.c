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

gchar *cairo_dock_launch_command_sync_stderr (const gchar *cCommand)
{
	gchar *standard_error=NULL;
	gint exit_status=0;
	GError *erreur = NULL;
	gboolean r = g_spawn_command_line_sync (cCommand,
		NULL,
		&standard_error,
		&exit_status,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		g_free (standard_error);
		return NULL;
	}
	if (standard_error != NULL && *standard_error == '\0')
	{
		g_free (standard_error);
		return NULL;
	}
	if (standard_error[strlen (standard_error) - 1] == '\n')
		standard_error[strlen (standard_error) - 1] ='\0';
	return standard_error;
}

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
	
	/* for tests:
	myData.iPercent = g_random_int_range (0, 100);
	myData.iQuality = 5 * myData.iPercent/100;
	myData.cInterface = g_strdup ("toto");
	return;*/
	
	///gchar *cResult = cairo_dock_launch_command_sync (MY_APPLET_SHARE_DATA_DIR"/wifi");
	gchar *cResult = cairo_dock_launch_command_sync_stderr ("iwconfig");  // iwconfig prints on stderr...
	g_print ("cResult: %s\n", cResult);
	if (cResult == NULL || *cResult == '\0')  // erreur a l'execution d'iwconfig (probleme de droit d'execution ou iwconfig pas installe) ou aucune interface wifi presente
	{ 
		g_free (cResult);
		return ;
	}
	
	//eth0 no wireless extensions.
	//eth2 IEEE 802.11 Nickname:""
	//Access Point: Not-Associated
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
	int i, iMaxValue=1;  // gcc rale si on n'initialise pas.
	for (i = 0; cInfopipesList[i] != NULL; i ++)
	{
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0' || *cOneInfopipe == '\n' )  // saut de ligne signalant une nouvelle interface.
		{
			if (myData.cInterface != NULL)  // comme on n'en veut qu'une on quitte.
				break;
			else
				continue;
		}
		
		if (myData.cInterface == NULL)  // on n'a pas encore d'interface valable.
		{
			str = strchr (cOneInfopipe, ' ');  // le nom de l'interface est en debut de ligne.
			if (str)
			{
				str2 = str + 1;
				while (*str2 == ' ')
					str2 ++;
				if (strncmp (str2, "no wireless", 11) != 0)
					myData.cInterface = g_strndup (cOneInfopipe, str - cOneInfopipe);
			}
			cd_debug ("interface : %s", myData.cInterface);
			if (myData.cInterface == NULL)  // cette ligne ne nous a rien apporte, on passe a la suivante.
				continue;
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
		
		if (myData.iQuality >= WIFI_NB_QUALITY)  // Link Quality=54/100 Signal level=-76 dBm Noise level=-78 dBm OU Link Quality:5  Signal level:219  Noise level:177
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
				myData.iPercent = 100. * myData.iQuality / (WIFI_NB_QUALITY-1);
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

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
	
	/* iwconfig prints wireless interface on stdout
	 * and other interfaces (e.g. eth0 -> no wireless extensions.) on stderr...
	 */
	gchar *cResult = cairo_dock_launch_command_sync_with_stderr (myData.cIWConfigPath, FALSE); // to avoid warnings... or with "sh -c \"iwconfig 2> /dev/null\"" but it uses 2 process each time...
	if (cResult == NULL || *cResult == '\0')  // error when launching iwconfig: rights problem, iwconfig not available or no wifi interface?
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
		if (*cOneInfopipe == '\0' || *cOneInfopipe == '\n' )  // EOL: a new interface.
		{
			if (myData.cInterface != NULL)  // we only want one, break
				break;
			else
				continue;
		}
		
		if (myData.cInterface == NULL)  // we don't still have any interface
		{
			str = strchr (cOneInfopipe, ' ');  // interface's name is at the beginning of the line.
			if (str)
			{
				str2 = str + 1;
				while (*str2 == ' ')
					str2 ++;
				if (strncmp (str2, "no wireless", 11) != 0)
					myData.cInterface = g_strndup (cOneInfopipe, str - cOneInfopipe);
			}
			cd_debug ("interface : %s", myData.cInterface);
			if (myData.cInterface == NULL)  // no new info, skip it
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
			if (iMaxValue != 0)  // old version, quality in %
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
		gldi_task_set_normal_frequency (myData.pTask);
	}
	else
	{
		myData.bWirelessExt = FALSE;
		cd_wifi_draw_no_wireless_extension ();
		gldi_task_downgrade_frequency (myData.pTask);
	}
	return TRUE;
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
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
	
	gchar *cResult = cairo_dock_launch_command_sync (MY_APPLET_SHARE_DATA_DIR"/wifi");
	if (cResult == NULL || *cResult == '\0')  //   // erreur a l'execution d'iwconfig (probleme de droit d'execution ou iwconfig pas installe) ou aucune interface wifi presente
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

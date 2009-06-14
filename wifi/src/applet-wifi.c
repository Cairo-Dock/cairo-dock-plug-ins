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

static char  *s_cTmpFile = NULL;
static char  *s_cTmpFileAccessPoint = NULL;


static float pourcent(float x, float y) {
  float p = 0;
  if (x > y) {
    x = y;
  }
  
  else if (x < 0) {
   x = 0;
  }
  
  p = (x / y) *100;
  return p;
}

static gboolean _wifi_get_values_from_file (gchar *cContent, int *iFlink, int *iMlink, int *iPercentage, CDWifiQuality *iQuality) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	gchar *cOneInfopipe;
	gchar *cESSID = NULL, *cQuality = NULL, *cConnName = NULL;
	int flink=0, mlink=0, i=0, prcnt=0;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0')
			continue;
		
		if ((i == 0) && (strcmp(cOneInfopipe,"Wifi") == 0)) {
			g_strfreev (cInfopipesList);
			myData.bWirelessExt = FALSE; //On n'a pas de device wifi d'activé
			return FALSE;
		}
		else if (cESSID == NULL) {
			cESSID = g_strstr_len (cOneInfopipe, -1, "ESSID");  // eth1 IEEE 802.11g ESSID:"bla bla bla" 
			if (cESSID != NULL) {
				cESSID += 6;  // on saute le ':' avec.
				if (*cESSID == '"') { // on enleve les guillemets.
					cESSID ++;
					gchar *str = strchr (cESSID, '"');
					if (str != NULL)
						*str = '\0';
				}
				else {
					cESSID = NULL;
				}
			}
		}
		else { // on a deja trouve l'EESID qui vient en 1er, on peut donc chercher le reste.
			cQuality = g_strstr_len (cOneInfopipe, -1, "Link Quality");
			if (cQuality != NULL) { //Link Quality=54/100 Signal level=-76 dBm Noise level=-78 dBm 
				cQuality += 13;  // on saute le '=' avec.
				gchar *str = strchr (cQuality, '/');
				if (str != NULL) {
					*str = '\0';
					flink = atoi(cQuality);
					mlink = atoi(str+1);
					prcnt = pourcent (flink, mlink);
					myData.bWirelessExt = TRUE; //On a un device wifi activé
				}
				break; //Les autres lignes ne nous importent peu.
			}
		}
		if (cConnName == NULL && i == 0) {
			cConnName = g_strdup (cOneInfopipe); // wlan0     IEEE 802.11g  ESSID:"WANADOO-21C8" 
			gchar *str = strchr (cConnName, ' ');
			if (str != NULL)
				*str = '\0';
		}
	}
	
	cd_debug ("Wifi - Name: %s - ESSID: %s - Signal Quality: %d/%d", cConnName, cESSID, flink, mlink);
	
	if (cESSID == NULL)
		cESSID = D_("Unknown");
	g_free (myData.cESSID);
	myData.cESSID = g_strdup (cESSID);
	
	if (cConnName == NULL)
		cConnName = D_("Unknown");
	g_free (myData.cConnName);
	myData.cConnName = g_strdup (cConnName);
	
	*iFlink = flink;
	*iMlink = mlink;
	if (prcnt <= 0) {
		*iQuality = WIFI_QUALITY_NO_SIGNAL;
	}
	else if (prcnt < 20) {
		*iQuality = WIFI_QUALITY_VERY_LOW;
	}
	else if (prcnt < 40) {
		*iQuality = WIFI_QUALITY_LOW;
	}
	else if (prcnt < 60) {
		*iQuality = WIFI_QUALITY_MIDDLE;
	}
	else if (prcnt < 80) {
		*iQuality = WIFI_QUALITY_GOOD;
	}
	else {
		*iQuality = WIFI_QUALITY_EXCELLENT;
	}
	*iPercentage = prcnt;
	
	g_strfreev (cInfopipesList);  // on le libere a la fin car cESSID pointait dessus.
	return TRUE;
}

void cd_wifi_read_data (void)
{
	gchar *cCommand = g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	
	/*if (myData.cConnName != NULL) {
		cCommand = g_strdup_printf("bash %s/access-point %s", MY_APPLET_SHARE_DATA_DIR, myData.cConnName, s_cTmpFileAccessPoint);
		gchar *cResult2 = cairo_dock_launch_command_sync (cCommand);
		g_free (cCommand);
	}*/  /// Il faudrait en faire quelque chose ...
	
	if (cResult == NULL)
	{
		cd_warning ("wifi : couldn't retrieve infos\nIt may happen if 'iwconfig' needs root priviledges.");
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		gboolean bAcquisitionOK = _wifi_get_values_from_file (cResult, &myData.flink, &myData.mlink, &myData.prcnt, &myData.iQuality);
		g_free (cResult);
		
		if (! bAcquisitionOK || myData.prcnt <= 0) {
			myData.bAcquisitionOK = FALSE;
			myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
			myData.prcnt = 0;
		}
		else {
			myData.bAcquisitionOK = TRUE;
		}
	}
}


gboolean cd_wifi_update_from_data (void) {
	if (myData.bAcquisitionOK) {
		cd_wifi_draw_icon ();
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
	}
	else {
		cd_wifi_draw_no_wireless_extension ();
		cairo_dock_downgrade_frequency_state (myData.pMeasureTimer);
	}
	return TRUE;
}

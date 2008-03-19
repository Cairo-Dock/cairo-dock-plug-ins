#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wifi.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

#define WIFI_TMP_FILE "/tmp/wifi"

static gchar *s_cLevelQualityName[WIFI_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Exellent")};


gboolean cd_wifi(gchar *origine) {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
  
  cd_debug("Wifi: Execution called from %s\n", origine);
  
  GError *erreur = NULL;
  gchar *cCommand = g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR);
  g_spawn_command_line_async (cCommand, &erreur);
  if (erreur != NULL) {
	  cd_warning ("Attention : when trying to execute 'iwconfig", erreur->message);
    g_error_free (erreur);
	}
	g_free (cCommand);
	
	if (myData.strengthTimer == 0)
		myData.strengthTimer = g_timeout_add (500, (GSourceFunc) cd_wifi_getStrength, (gpointer) NULL);
	
  bBusy = FALSE;
  if (myData.isWirelessDevice == 0) {
	  return FALSE;
	}
	else {
	  return TRUE;
	}
}


static gboolean _wifi_get_values_from_file (gchar *cContent, int *iFlink, int *iMlink, int *iPercentage, CDWifiQuality *iQuality)
{
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	g_free(cContent);
	gchar *cOneInfopipe;
	//gchar **tcnt,**bcnt;
	const gchar *levelName;
	int flink=0, mlink=0, i=0;
	int prcnt=0;
	for (i = 0; cInfopipesList[i] != NULL; i ++)
	{
		cOneInfopipe = cInfopipesList[i];
		if ((i == 0) && (strcmp(cOneInfopipe,"Wifi") == 0))
		{
			g_strfreev (cInfopipesList);
			return FALSE;
		}
		else if (i == 5)
		{
			myData.isWirelessDevice = 1; //Wireless Devices found
			
			//tcnt = g_strsplit(cOneInfopipe," ", -1);
			int c = 0, iNbSpace = 0;
			gchar *cUtilInfo = NULL;
			while (cOneInfopipe[c] != '\0')
			{
				if (cOneInfopipe[c] == ' ')
				{
					iNbSpace ++;
					if (iNbSpace == 11)
					{
						cUtilInfo = &cOneInfopipe[c+1];
						break ;
					}
				}
				c ++;
			}
			
			/*bcnt = g_strsplit(tcnt[11],"=", -1);
			if (bcnt[1] == NULL) {
			bcnt = g_strsplit(tcnt[11],":", -1);
			}
			tcnt = g_strsplit(bcnt[1],"/", -1);
			flink = atoi(tcnt[0]);
			mlink = atoi(tcnt[1]);*/
			if (cUtilInfo != NULL)
			{
				gchar *str = strchr (cUtilInfo, '=');
				if (str == NULL)
					str = strchr (cUtilInfo, ':');
				if (str != NULL)
				{
					cUtilInfo = str + 1;
					str = strchr (cUtilInfo, '/');
					if (str != NULL)
					{
						*str = '\0';
						flink = atoi (cUtilInfo);
						mlink = atoi (str+1);
						
						//Thanks to Ahmad Baitalmal & Brian Elliott Finley for thoses values (extracted from wifi-radar phyton script)
						cd_debug("Signal Quality: %d / %d", flink, mlink);
						prcnt = pourcent(flink,mlink);
					}
				}
			}
		}
		
	}
	g_strfreev (cInfopipesList);
	
	*iFlink = flink;
	*iMlink = mlink;
	if (prcnt == 0) {
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
	return TRUE;
}

gboolean cd_wifi_getStrength(void) {
	static gboolean bBusy = FALSE;
	
	if (bBusy)
		return FALSE;
	bBusy = TRUE;
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(WIFI_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)  // cContent ne peut pas etre NULL sans provoquer une erreur.
	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[WIFI_QUALITY_NO_SIGNAL]);
	}
	else {
		int flink,mlink;
		int prcnt;
		CDWifiQuality iQuality;
		gboolean bAcquisitionOK = _wifi_get_values_from_file (cContent, &flink, &mlink, &prcnt, &iQuality);
		
		if (! bAcquisitionOK)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
			CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[WIFI_QUALITY_NO_SIGNAL]);
			cd_message("No wifi device found, timer stopped.");
			myData.isWirelessDevice = 0;
			bBusy = FALSE;
			myData.strengthTimer = 0;
			return FALSE;
		}
		
		switch (myConfig.quickInfoType) {
			case WIFI_INFO_NONE :
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(NULL);
			break;
			case WIFI_INFO_SIGNAL_STRENGTH_LEVEL :
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(s_cLevelQualityName[iQuality]);
			break;
			case WIFI_INFO_SIGNAL_STRENGTH_PERCENT :
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", prcnt);
			break;
			case WIFI_INFO_SIGNAL_STRENGTH_DB :
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("%d/%d", flink, mlink);
			break;
		}
		
		cairo_surface_t *pSurface = myData.pSurfaces[iQuality];
		if (pSurface != NULL)
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		}
	}
	
	bBusy = FALSE;
	return FALSE;
}

float pourcent(float x, float y) {
  float p=0;
  if (x > y) {
    x = y;
  }
  else if (x < 0) {
   x = 0;
  }
  p = (x / y) *100;
  return p;
}

/*void cd_wifi_init(gchar *origine) {
  cd_debug("Wifi: Initialisation called from %s\n", origine);
	myData.isWirelessDevice = 1;
	cd_wifi("Wifi_Init");
  myData.checkTimer = g_timeout_add (10000, (GSourceFunc) cd_wifi, (gpointer) origine);
}*/

void cd_wifi_wait(gchar *origine) {
  cd_debug("Wifi: Check called from %s", origine);
  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("Check...");
	CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[WIFI_QUALITY_NO_SIGNAL]);
	myData.isWirelessDevice = 1;
  myData.checkTimer = g_timeout_add (10000, (GSourceFunc) cd_wifi, (gpointer) origine);
}


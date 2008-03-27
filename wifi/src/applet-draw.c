#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


static gchar *s_cLevelQualityName[WIFI_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Excellent")};


void cd_wifi_draw_no_wireless_extension (void)
{
	if (myData.iPreviousQuality != myData.iQuality)
	{
		myData.iPreviousQuality = myData.iQuality;
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		if (myConfig.hollowIcon) { //Blank Icon
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			CD_APPLET_SET_SURFACE_ON_MY_ICON (NULL);
		}
		else {
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
			cd_wifi_set_surface (WIFI_QUALITY_NO_SIGNAL);
		}
	}
	
	myData.checkedTime ++;
	if (myData.checkedTime == 1) {
	  myConfig.iCheckInterval = 60000; //check 1min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	else if (myData.checkedTime == 2) {
	  myConfig.iCheckInterval = 180000; //check 3min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	else if (myData.checkedTime >= 3) {
	  myConfig.iCheckInterval = 600000; //check 5min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	
	cd_message("No wifi device found, timer changed %d.",myConfig.iCheckInterval);
}

void cd_wifi_draw_icon (void)
{
	myData.checkedTime = 0; // Reset the check counter
	gboolean bNeedRedraw = FALSE;
	switch (myConfig.quickInfoType) {
		case WIFI_INFO_NONE :
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(NULL);
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_LEVEL :
			if (myData.iQuality != myData.iPreviousQuality)
			{
				myData.iPreviousQuality = myData.iQuality;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(D_(s_cLevelQualityName[myData.iQuality]));
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_PERCENT :
			if (myData.prev_prcnt != myData.prcnt)
			{
				myData.prev_prcnt = myData.prcnt;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", myData.prcnt);
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_DB :
			if (myData.prev_flink != myData.flink || myData.prev_mlink != myData.mlink)
			{
				myData.prev_flink = myData.flink;
				myData.prev_mlink = myData.mlink;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("%d/%d", myData.flink, myData.mlink);
				bNeedRedraw = TRUE;
			}
		break;
	}
	
	if (myData.iQuality != myData.iPreviousQuality)
	{
		myData.iPreviousQuality = myData.iQuality;
		cd_wifi_set_surface (myData.iQuality);
	}
	else if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON
}

void cd_wifi_set_surface (CDWifiQuality iQuality) {
	g_return_if_fail (iQuality < WIFI_NB_QUALITY);
	
	cairo_surface_t *pSurface = myData.pSurfaces[iQuality];
	if (pSurface == NULL) {
		if (myConfig.cUserImage[iQuality] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iQuality]);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/link-%d.svg", MY_APPLET_SHARE_DATA_DIR, iQuality);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[iQuality]);
	}
	else {
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}

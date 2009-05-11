#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-draw.h"


static gchar *s_cIconName[WIFI_NB_QUALITY] = {"link-0.svg", "link-1.svg", "link-2.svg", "link-3.svg", "link-4.svg", "link-5.svg"};
static gchar *s_cLevelQualityName[WIFI_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Excellent")};


void cd_wifi_draw_no_wireless_extension (void) {
	if (myData.iPreviousQuality != myData.iQuality) {
		if (myDesklet != NULL)
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		
		myData.iPreviousQuality = myData.iQuality;
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		cd_wifi_draw_icon_with_effect (WIFI_QUALITY_NO_SIGNAL);
		
		CD_APPLET_REDRAW_MY_ICON;
	}
}

void cd_wifi_draw_icon (void) {
	gboolean bNeedRedraw = FALSE;
	switch (myConfig.quickInfoType) {
		case WIFI_INFO_NONE :
			if (myIcon->cQuickInfo != NULL) {
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_LEVEL :
			if (myData.iQuality != myData.iPreviousQuality) {
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (D_(s_cLevelQualityName[myData.iQuality]));
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_PERCENT :
			if (myData.prev_prcnt != myData.prcnt) {
				myData.prev_prcnt = myData.prcnt;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", myData.prcnt);
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_DB :
			if (myData.prev_flink != myData.flink || myData.prev_mlink != myData.mlink) {
				myData.prev_flink = myData.flink;
				myData.prev_mlink = myData.mlink;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d/%d", myData.flink, myData.mlink);
				bNeedRedraw = TRUE;
			}
		break;
	}
	
	if (myData.iQuality != myData.iPreviousQuality) {
		myData.iPreviousQuality = myData.iQuality;
		
		//cd_debug ("Wifi - Value have changed, redraw. (Use Gauge: %d)", myConfig.bUseGauge);
		if (myConfig.iDisplay == WIFI_GAUGE) {
			CD_APPLET_RENDER_GAUGE (myData.pGauge, (double) myData.prcnt / 100);
			bNeedRedraw = TRUE;
		}
		else if (myConfig.iDisplay == WIFI_GRAPHIC) {
			CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, (double) myData.prcnt / 100);
		}
		else {
			cd_wifi_draw_icon_with_effect (myData.iQuality);
		}
	}
	else {
		if (myConfig.iDisplay == WIFI_GRAPHIC)
			CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, (double) myData.prcnt / 100); //On veut toujours avoir une valeur
	}
	
	if (myConfig.bESSID && myData.cESSID != NULL && strcmp (myData.cESSID, myIcon->acName))
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cESSID);
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
}

void cd_wifi_draw_icon_with_effect (CDWifiQuality iQuality) {
	cairo_surface_t *pSurface = myData.pSurfaces[iQuality];
	if (pSurface == NULL)	{
		if (myConfig.cUserImage[iQuality] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iQuality]);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[iQuality]);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		pSurface = myData.pSurfaces[iQuality];
	}
	
	switch (myConfig.iEffect) {
		double fAlpha, fScale;
	  case WIFI_EFFECT_NONE:
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	  break;
	  case WIFI_EFFECT_ZOOM:
	  	fScale = .2 + .8 * myData.prcnt / 100.;
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
	  break;
	  case WIFI_EFFECT_TRANSPARENCY:
	  	fAlpha = .2 + .8 * myData.prcnt / 100.;
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
	  break;
	  case WIFI_EFFECT_BAR:
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.prcnt * .01);
	  break;
	  default :
	  break;
	}
}

void cd_wifi_bubble (void) {
	GString *sInfo = g_string_new ("");
	gchar *cIconPath = NULL;
  if (myData.iQuality == WIFI_QUALITY_NO_SIGNAL) {
  	cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "link-0.svg");
  	g_string_printf (sInfo, "%s", D_("Wifi disabled."));
	}
	else {
		cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "link-5.svg");
		g_string_printf (sInfo, "%s %s \n %s %d%%%%", D_("Wifi enabled. \n Connected on:"), myData.cESSID, D_("Signal Strength:"), myData.prcnt);
	}
	
	cd_debug ("%s (%s)", sInfo->str, cIconPath);
	cairo_dock_show_temporary_dialog_with_icon (sInfo->str, myIcon, myContainer, 6000, cIconPath);
	g_string_free (sInfo, TRUE);
	g_free (cIconPath);
}

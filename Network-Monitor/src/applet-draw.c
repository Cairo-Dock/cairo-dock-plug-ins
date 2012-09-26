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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-draw.h"


static const gchar *s_cIconName[CONNECTION_NB_QUALITY] = {"link-0.svg", "link-1.svg", "link-2.svg", "link-3.svg", "link-4.svg", "link-5.svg", "network-not-connected.png", "network-wired.png"};
// static const gchar *s_cLevelQualityName[CONNECTION_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Excellent"), N_("Not connected"), N_("Wired connection")}; // not used

void cd_NetworkMonitor_draw_no_wireless_extension (void)
{
	if (myData.iPreviousQuality != myData.iQuality)
	{
		myData.iPreviousQuality = myData.iQuality;
		if (myConfig.defaultTitle) // has another default name
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		else
			CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		cd_NetworkMonitor_draw_icon_with_effect (WIFI_QUALITY_NO_SIGNAL);
		
		CD_APPLET_REDRAW_MY_ICON;
	}
}

void cd_NetworkMonitor_draw_icon (void)
{
	if (myData.iQuality <= 0)
	{
		cd_NetworkMonitor_draw_no_wireless_extension ();
		return ;
	}
	gboolean bNeedRedraw = FALSE;
	if (myData.iPrevPercent != myData.iPercent) {
		myData.iPrevPercent = myData.iPercent;
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", myData.iPercent);
		bNeedRedraw = TRUE;
	}
	
	CDRenderType iRenderType = (myConfig.bModeWifi ? myConfig.wifiRenderer.iRenderType : myConfig.netSpeedRenderer.iRenderType);
	if (myData.iQuality != myData.iPreviousQuality || iRenderType == CD_EFFECT_GRAPH) {
		myData.iPreviousQuality = myData.iQuality;
		//cd_debug ("Wifi - Value have changed, redraw. (Use Gauge: %d)", myConfig.bUseGauge);
		if (iRenderType == CD_EFFECT_ICON) {
			cd_NetworkMonitor_draw_icon_with_effect (myData.iQuality);
		}
		else
		{
			double fValue = (double) myData.iPercent / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fValue);
		}
	}
	
	if (/*myConfig.bESSID && */myData.cESSID != NULL && cairo_dock_strings_differ (myData.cESSID, myIcon->cName))
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cESSID);
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
}

void cd_NetworkMonitor_draw_icon_with_effect (CDConnectionQuality iQuality) {
	if (iQuality >= CONNECTION_NB_QUALITY)
		iQuality = WIFI_QUALITY_NO_SIGNAL;
	
	cairo_surface_t *pSurface = myData.pSurfaces[iQuality];
	if (pSurface == NULL) {
		if (myConfig.wifiRenderer.cUserImage[iQuality] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.wifiRenderer.cUserImage[iQuality]);
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
	
	switch (myConfig.wifiRenderer.iEffect) {
		double fAlpha, fScale;
	  case WIFI_EFFECT_NONE:
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	  break;
	  case WIFI_EFFECT_ZOOM:
	  	fScale = .2 + .8 * myData.iPercent / 100.;
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
	  break;
	  case WIFI_EFFECT_TRANSPARENCY:
	  	fAlpha = .2 + .8 * myData.iPercent / 100.;
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
	  break;
	  case WIFI_EFFECT_BAR:
	  	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.iPercent / 100.);
	  break;
	  default :
	  break;
	}
}

void cd_NetworkMonitor_bubble (void) {
	if (cairo_dock_task_is_running (myData.wifi.pTask) || cairo_dock_task_is_running (myData.netSpeed.pTask))
	{
		cairo_dock_show_temporary_dialog  (D_("Checking connection...\nPlease retry in a few seconds"), myIcon, myContainer, 3000);
		return ;
	}
	GString *sInfo = g_string_new ("");
	const gchar *cIconPath;
	if (myData.bWiredExt)
	{
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/network-wired.png";
		g_string_assign (sInfo, D_("Wired Connection."));
		g_string_printf (sInfo, "%s : %s\n%s : %d Mbps\n",
			D_ ("Interface"), myData.cInterface,
			D_ ("Speed"), myData.iSpeed);
			//D_ ("Network ID"), myData.cESSID ? myData.cESSID : D_("unknown"),
			//D_ ("Access point"), myData.cAccessPoint,
			//D_ ("Signal Quality"), myData.iQuality, CONNECTION_NB_QUALITY-1);
	}
	else
	{
		cd_debug("Network-Monitor : juste avant affichage : %s", myData.cAccessPoint);
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/link-5.svg";
		g_string_assign (sInfo, D_("Wifi enabled."));
		g_string_printf (sInfo, "%s : %s\n%s : %d Mbps\n%s : %s\n%s : %s\n%s : %d/%d",
			D_ ("Network ID"), myData.cESSID ? myData.cESSID : D_("Unknown"),
			D_ ("Speed"), myData.iSpeed/1000,
			D_ ("Access point"), myData.cAccessPoint,
			D_ ("Interface"), myData.cInterface,
			D_ ("Signal Quality"), myData.iQuality, CONNECTION_NB_QUALITY-3);
	}
		
	//cd_debug ("%s (%s)", sInfo->str, cIconPath);
	cairo_dock_show_temporary_dialog_with_icon (sInfo->str, myIcon, myContainer, 6000, cIconPath);
	g_string_free (sInfo, TRUE);}

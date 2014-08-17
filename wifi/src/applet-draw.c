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
#include "applet-wifi.h"
#include "applet-draw.h"


static const gchar *s_cLevelQualityName[WIFI_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Excellent")};


void cd_wifi_draw_no_wireless_extension (void)
{
	cd_debug ("No Wireless: %d, %d", myData.iPreviousQuality, myData.iQuality);
	if (myData.iPreviousQuality != myData.iQuality)
	{
		if (myDesklet != NULL)
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		myData.iPreviousQuality = myData.iQuality;
		
		// reset label
		if (myConfig.defaultTitle) // has another default name
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		else
			CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		
		// reset quick-info
		if (myConfig.quickInfoType != WIFI_INFO_NONE) // if we want to have a quick info
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		
		// reset the data-renderer
		if (myConfig.iDisplayType == CD_WIFI_BAR)
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cNoSignalIcon, "no-signal.svg");
		double fValue = CAIRO_DATA_RENDERER_UNDEF_VALUE;
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fValue);
	}
	else if (myConfig.iDisplayType == CD_WIFI_GRAPH)
	{
		 double fValue = CAIRO_DATA_RENDERER_UNDEF_VALUE;
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fValue);
	}
}

void cd_wifi_draw_icon (void)
{
	cd_debug ("Draw Wireless: %d, %d", myData.iPreviousQuality, myData.iQuality);
	if (myData.iPercent <= 0)
	{
		cd_wifi_draw_no_wireless_extension (); // not connected
		return;
	}
	
	// update quick-info
	gboolean bNeedRedraw = FALSE;
	switch (myConfig.quickInfoType)
	{
		case WIFI_INFO_SIGNAL_STRENGTH_LEVEL :
			if (myData.iQuality != myData.iPreviousQuality && myData.iQuality < WIFI_NB_QUALITY)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (D_(s_cLevelQualityName[myData.iQuality]));
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_PERCENT :
			if (myData.iPrevPercent != myData.iPercent)
			{
				myData.iPrevPercent = myData.iPercent;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", myData.iPercent);
				bNeedRedraw = TRUE;
			}
		break;
		case WIFI_INFO_SIGNAL_STRENGTH_DB :
			if (myData.iPrevSignalLevel != myData.iSignalLevel || myData.iPrevNoiseLevel != myData.iNoiseLevel)
			{
				myData.iPrevSignalLevel = myData.iSignalLevel;
				myData.iPrevNoiseLevel = myData.iNoiseLevel;
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d/%d", myData.iSignalLevel, myData.iNoiseLevel);
				bNeedRedraw = TRUE;
			}
		break;
		default: // WIFI_INFO_NONE
		break;
	}
	
	if (myData.iQuality != myData.iPreviousQuality || myConfig.iDisplayType == CD_WIFI_GRAPH)
	{
		myData.iPreviousQuality = myData.iQuality;
		//cd_debug ("Wifi - Value have changed, redraw. (Use Gauge: %d)", myConfig.bUseGauge);
		double fValue = (double) myData.iPercent / 100.;
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fValue);
		bNeedRedraw = FALSE;
	}
	
	
	if (myData.cESSID != NULL && myConfig.defaultTitle == NULL && cairo_dock_strings_differ (myData.cESSID, myIcon->cName))
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cESSID);
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
}


void cd_wifi_bubble (void)
{
	if (gldi_task_is_running (myData.pTask))
	{
		gldi_dialog_show_temporary  (D_("Checking connection...\nPlease retry in a few seconds"), myIcon, myContainer, 3000);
		return ;
	}
	GString *sInfo = g_string_new ("");
	const gchar *cIconPath;
	if (! myData.bWirelessExt)
	{
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/no-signal.svg";
		g_string_assign (sInfo, D_("WiFi disabled."));
	}
	else
	{
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/default.svg";
		g_string_assign (sInfo, D_("Wifi enabled."));
		g_string_printf (sInfo, "%s : %s\n%s : %s\n%s : %s\n%s : %d/%d",
			D_ ("Network ID"), myData.cESSID ? myData.cESSID : D_("unknown"),
			D_ ("Access point"), myData.cAccessPoint,
			D_ ("Interface"), myData.cInterface,
			D_ ("Signal Quality"), myData.iQuality, WIFI_NB_QUALITY-1);
	}
		
	//cd_debug ("%s (%s)", sInfo->str, cIconPath);
	gldi_dialog_show_temporary_with_icon (sInfo->str, myIcon, myContainer, 6000, cIconPath);
	g_string_free (sInfo, TRUE);
}

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

#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"


void update_icon (void)
{
	gboolean bNeedRedraw = FALSE;
	cd_message ("%s (on battery: %d -> %d; time:%.1f -> %.1f ; charge:%.1f -> %.1f)", __func__, myData.bPrevOnBattery, myData.bOnBattery, (double)myData.iPrevTime, (double)myData.iTime, (double)myData.iPrevPercentage, (double)myData.iPercentage);
	
	// on prend en compte la nouvelle charge.
	if (myData.bPrevOnBattery != myData.bOnBattery || myData.iPrevPercentage != myData.iPercentage)
	{
		if (myData.bPrevOnBattery != myData.bOnBattery)
		{
			myData.bPrevOnBattery = myData.bOnBattery;
			myData.bAlerted = FALSE;  // On a change de statut, donc on reinitialise les alertes
			myData.bCritical = FALSE;
		}
		
		// on redessine l'icone.
		if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE || myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
		{
			double fPercent = (double) myData.iPercentage / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			bNeedRedraw = FALSE;
		}
		else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
		{
			cd_powermanager_draw_icon_with_effect (myData.bOnBattery);
			bNeedRedraw = FALSE;
		}
		
		// on declenche les alarmes.
		if (myData.bOnBattery)
		{
			// Alert when battery charge is under a configured value in %
			if (myData.iPercentage <= myConfig.lowBatteryValue && ! myData.bAlerted)
			{
				cd_powermanager_alert(POWER_MANAGER_CHARGE_LOW);
				if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW] != NULL)
					cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW]);
			}
			// Alert when battery charge is under 4%
			if (myData.iPercentage <= 4 && ! myData.bCritical)
			{
				myData.bCritical = TRUE;
				cd_powermanager_alert (POWER_MANAGER_CHARGE_CRITICAL);
				if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL] != NULL)
					cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL]);
			}
			// emblem is implicitely erased.
		}
		else
		{
			// Alert when battery is charged
			if(myData.iPercentage == 100 && ! myData.bAlerted)
				cd_powermanager_alert (POWER_MANAGER_CHARGE_FULL);
				
			CD_APPLET_DRAW_EMBLEM_ON_MY_ICON (myData.pEmblem);
		}
		
		if (myConfig.defaultTitle == NULL || *myConfig.defaultTitle == '\0')
		{
			gchar cFormatBuffer[21];
			int iBufferLength = 20;
			if (myData.iTime != 0)
			{
				int time = myData.iTime;
				int hours = time / 3600;
				int minutes = (time % 3600) / 60;
				if (hours != 0)
					snprintf (cFormatBuffer, iBufferLength, "%dh%02d", hours, abs (minutes));
				else
					snprintf (cFormatBuffer, iBufferLength, "%dmn", minutes);
			}
			else
			{
				strncpy (cFormatBuffer, "-:--", iBufferLength);
			}
			CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s: %d%% - %s: %s",
				D_("Charge"),
				(int)myData.iPercentage,
				D_("Time"),
				cFormatBuffer);
		}
		
		myData.bPrevOnBattery = myData.bOnBattery;
		myData.iPrevPercentage = myData.iPercentage;
		myData.iPrevTime = myData.iTime;
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
}

gchar *get_hours_minutes (int iTimeInSeconds)
{
	gchar *cTimeString;
	int h=0, m=0;
	m = iTimeInSeconds / 60;
	h = m / 60;
	m = m - (h * 60);
	if (h > 0) cTimeString = g_strdup_printf("%dh%02dm", h, m);
	else if (m > 0) cTimeString = g_strdup_printf("%dm", m);
	else cTimeString = g_strdup (D_("None"));
	
	return cTimeString;
}

static void _cd_powermanager_dialog (const gchar *cInfo, int iDuration)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	const gchar *cIconPath;
	if (!myData.bOnBattery)
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/sector.svg";
	else
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/default-battery.svg";
	
	cd_debug ("%s (%s)", cInfo, cIconPath);
	cairo_dock_show_temporary_dialog_with_icon (cInfo, myIcon, myContainer, 1000*iDuration, cIconPath);
}

void cd_powermanager_bubble (void)
{
	GString *sInfo = g_string_new ("");
	if (myData.cBatteryStateFilePath != NULL || myData.pProxyStats != NULL)
	{
		gchar *hms = NULL;
		if (myData.iTime > 0.)
			hms = get_hours_minutes (myData.iTime);
		else
			hms = g_strdup_printf ("%s", D_("Unknown"));
		if(myData.bOnBattery)
		{
			g_string_printf (sInfo, "%s %d%% \n %s %s", D_("Laptop on Battery.\n Battery charged at:"), (int)myData.iPercentage, D_("Estimated time with charge:"), hms);
		}
		else
		{
			g_string_printf (sInfo, "%s %d%% \n %s %s", D_("Laptop on Charge.\n Battery charged at:"), (int)myData.iPercentage, D_("Estimated charge time:"), (myData.iPercentage > 99.9 ? "0" : hms));
		}
		g_free (hms);
	}
	else
	{
		g_string_assign (sInfo, D_("No battery found."));
	}
	
	_cd_powermanager_dialog (sInfo->str, 6);
	g_string_free (sInfo, TRUE);
}

gboolean cd_powermanager_alert (MyAppletCharge alert)
{
	cd_debug ("%s", __func__);
	GString *sInfo = g_string_new ("");
	
	gchar *hms = NULL;
	if (myData.iTime > 0.)
		hms = get_hours_minutes (myData.iTime);
	else
		hms = g_strdup (D_("Unknown"));
		
	if ((alert == POWER_MANAGER_CHARGE_LOW && myConfig.lowBatteryWitness) || (alert == POWER_MANAGER_CHARGE_CRITICAL && myConfig.criticalBatteryWitness))
	{
		if (myConfig.iNotificationType != 1)
		{
			g_string_printf (sInfo, "%s (%d%%) \n %s %s \n %s", D_("PowerManager.\nBattery charge seems to be low"), (int)myData.iPercentage, D_("Estimated time with charge:"), hms, D_("Please put your laptop on charge."));
			_cd_powermanager_dialog (sInfo->str, myConfig.iNotificationDuration);
		}
		if (myConfig.iNotificationType != 2)
		{
			CD_APPLET_DEMANDS_ATTENTION (myConfig.cNotificationAnimation, myConfig.iNotificationDuration);
		}
		if (myConfig.cSoundPath[alert] != NULL)
			cairo_dock_play_sound (myConfig.cSoundPath[alert]);
	}
	else if (alert == POWER_MANAGER_CHARGE_FULL && myConfig.highBatteryWitness)
	{
		if (myConfig.iNotificationType != 1)
		{
			g_string_printf (sInfo, "%s (%d%%)", D_("PowerManager.\nYour battery is now charged"), (int)myData.iPercentage);
			_cd_powermanager_dialog (sInfo->str, myConfig.iNotificationDuration);
		}
		if (myConfig.iNotificationType != 2)
		{
			CD_APPLET_DEMANDS_ATTENTION (myConfig.cNotificationAnimation, myConfig.iNotificationDuration);
		}
		if (myConfig.cSoundPath[alert] != NULL)
			cairo_dock_play_sound (myConfig.cSoundPath[alert]);
	}
	
	g_free (hms);
	g_string_free (sInfo, TRUE);
	myData.bAlerted = TRUE;
	return FALSE;
}


void cd_powermanager_draw_icon_with_effect (gboolean bOnBattery)
{
	if (bOnBattery && myData.pSurfaceBattery == NULL)
	{
		gchar *cImagePath;
		if (myConfig.cUserBatteryIconName == NULL)
			cImagePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/default-battery.svg");
		else
			cImagePath = cairo_dock_generate_file_path (myConfig.cUserBatteryIconName);
		
		myData.pSurfaceBattery = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
		g_free (cImagePath);
	}
	else if (! bOnBattery && myData.pSurfaceCharge == NULL)
	{
		gchar *cImagePath;
		if (myConfig.cUserChargeIconName == NULL)
			cImagePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/default-charge.svg");
		else
			cImagePath = cairo_dock_generate_file_path (myConfig.cUserChargeIconName);
		
		myData.pSurfaceCharge = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
		g_free (cImagePath);
	}
	
	cairo_surface_t *pSurface = (bOnBattery ? myData.pSurfaceBattery : myData.pSurfaceCharge);
	
	switch (myConfig.iEffect)
	{
		case POWER_MANAGER_EFFECT_NONE :
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		break;
		case POWER_MANAGER_EFFECT_ZOOM :
			cairo_save (myDrawContext);
			double fScale = .3 + .7 * myData.iPercentage / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
			cairo_restore (myDrawContext);
		break;
		case POWER_MANAGER_EFFECT_TRANSPARENCY :
			cairo_save (myDrawContext);
			double fAlpha = .3 + .7 * myData.iPercentage / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
			cairo_restore (myDrawContext);
		break;
		case POWER_MANAGER_EFFECT_BAR :
			cairo_save (myDrawContext);
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.iPercentage * .01);
			cairo_restore (myDrawContext);
		break;
		default :
		break;
	}
}


void cd_powermanager_format_value (CairoDataRenderer *pRenderer, int iNumValue, gchar *cFormatBuffer, int iBufferLength, CairoDockModuleInstance *myApplet)
{
	double fValue = cairo_data_renderer_get_normalized_current_value_with_latency (pRenderer, iNumValue);
	if(myConfig.quickInfoType == POWER_MANAGER_TIME)
	{
		if (myData.iTime != 0)
		{
			int time = myData.iTime;
			int hours = time / 3600;
			int minutes = (time % 3600) / 60;
			if (hours != 0)
				snprintf (cFormatBuffer, iBufferLength, "%dh%02d", hours, abs (minutes));
			else
				snprintf (cFormatBuffer, iBufferLength, "%dmn", minutes);
		}
		else
		{
			strncpy (cFormatBuffer, "-:--", iBufferLength);
		}
	}
	else if(myConfig.quickInfoType == POWER_MANAGER_CHARGE)
	{
		snprintf (cFormatBuffer, iBufferLength, "%d%%", (int)myData.iPercentage);
	}
	else
		cFormatBuffer[0] = '\0';
}

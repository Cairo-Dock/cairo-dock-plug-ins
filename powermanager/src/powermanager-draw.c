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

/* 3 states:
no battery -> empty gauge (0%), on-sector emblem, no alert
on battery -> gauge + %, no emblem, alert
on sector -> gauge + %, on-sector emblem, alert
*/

void update_icon (void)
{
	gboolean bNeedRedraw = FALSE;
	cd_debug ("%s (on battery: %d -> %d; time:%.1f -> %.1f ; charge:%.1f -> %.1f)", __func__, myData.bPrevOnBattery, myData.bOnBattery, (double)myData.iPrevTime, (double)myData.iTime, (double)myData.iPrevPercentage, (double)myData.iPercentage);
	
	// no information available, draw a default icon.
	if (myData.cBatteryStateFilePath == NULL && myData.pUPowerClient == NULL)
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/sector.svg");
		CD_APPLET_REDRAW_MY_ICON;
		return;
	}
	
	// hide the icon when not on battery
	if (myConfig.bHideNotOnBattery && ! myData.bOnBattery && myDock)
	{
		if (! myData.bIsHidden)
		{ // we remove the icon
			cairo_dock_detach_icon_from_dock (myIcon, myDock);
			myData.bIsHidden = TRUE;
			cairo_dock_update_dock_size (myDock);
			cairo_dock_redraw_container (CAIRO_CONTAINER (myDock)); // dock refresh forced
		}
		return; // no need any redraw if the icon is hidden, and can't display the dialog properly without the icon.
	}
	else if (myData.bIsHidden && myData.bOnBattery && myDock) // if the icon is hidden but we are now on battery, we (re-)insert the icon.
	{
		cd_debug ("Re-insert the icon");
		cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
		cairo_dock_redraw_container (CAIRO_CONTAINER (myDock)); // dock refresh forced
		myData.bIsHidden = FALSE;
	}

	// on prend en compte la nouvelle charge.
	if (myData.bPrevOnBattery != myData.bOnBattery || myData.iPrevPercentage != myData.iPercentage || myData.iTime != myData.iPrevTime)
	{
		// on redessine l'icone.
		if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE || myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
		{
			double fPercent;
			if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE && ! myData.bBatteryPresent)
				fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
			else
				fPercent = (double) myData.iPercentage / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			bNeedRedraw = FALSE;
		}
		else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
		{
			cd_powermanager_draw_icon_with_effect (myData.bOnBattery);
			bNeedRedraw = FALSE;
		}
		
		// add or remove the charge overlay if the 'on_battery' status has changed.
		if (myData.bPrevOnBattery != myData.bOnBattery)
		{
			if (! myData.bOnBattery)
			{
				CD_APPLET_ADD_OVERLAY_ON_MY_ICON (myConfig.cEmblemIconName ? myConfig.cEmblemIconName : MY_APPLET_SHARE_DATA_DIR"/charge.svg", CAIRO_OVERLAY_MIDDLE);
			}
			else
			{
				CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_MIDDLE);
			}
		}
		
		// on declenche les alarmes.
		if (myData.bOnBattery)
		{
			// Alert when battery charge goes under a configured value in %
			if (myData.iPrevPercentage > myConfig.lowBatteryValue && myData.iPercentage <= myConfig.lowBatteryValue)
			{
				cd_powermanager_alert(POWER_MANAGER_CHARGE_LOW);
				if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW] != NULL)
					cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW]);
			}
			// Alert when battery charge is under 4%
			if (myData.iPrevPercentage > 4 && myData.iPercentage <= 4)
			{
				cd_powermanager_alert (POWER_MANAGER_CHARGE_CRITICAL);
				if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL] != NULL)
					cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL]);
			}
		}
		else
		{
			// Alert when battery is charged
			if(myData.iPrevPercentage < 100 && myData.iPercentage == 100)
				cd_powermanager_alert (POWER_MANAGER_CHARGE_FULL);
		}
		
		// update the icon's label.
		if (myConfig.defaultTitle == NULL || *myConfig.defaultTitle == '\0')
		{
			if (! myData.bOnBattery && myData.iPercentage > 99.9)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s (%d%%)",
					D_("Battery charged"),
					(int)myData.iPercentage);
			}
			else
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
				CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s: %s (%d%%)",
					myData.bOnBattery ? D_("Time before empty") : D_("Time before full"),
					cFormatBuffer,
					(int)myData.iPercentage);
			}
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
	int h = iTimeInSeconds / 3600;
	int m = (iTimeInSeconds % 3600) / 60;
	if (h > 0) 		cTimeString = g_strdup_printf ("%dh%02dm", h, m);
	else if (m > 0) cTimeString = g_strdup_printf ("%dm", m);
	else 			cTimeString = g_strdup (D_("None"));
	
	return cTimeString;
}

static void _cd_powermanager_dialog (const gchar *cInfo, int iDuration)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	/**const gchar *cIconPath;
	if (!myData.bOnBattery)
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/sector.svg";
	else
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/default-battery.svg";
	
	cd_debug ("%s (%s)", cInfo, cIconPath);*/
	cairo_dock_show_temporary_dialog_with_icon (cInfo, myIcon, myContainer, 1000*iDuration, "same icon");
}

void cd_powermanager_bubble (void)
{
	GString *sInfo = g_string_new ("");
	if (myData.cBatteryStateFilePath != NULL || myData.pUPowerClient != NULL)
	{
		// time and charge.
		gchar *hms = NULL;
		if (myData.iTime > 0.)
			hms = get_hours_minutes (myData.iTime);
		else
			hms = g_strdup_printf ("%s", D_("Unknown"));
		if (myData.bOnBattery)
		{
			g_string_printf (sInfo, "%s\n"
				"%s %d%%\n"
				"%s %s",
				D_("Laptop on Battery."),
				D_("Battery charged at:"), (int)myData.iPercentage,
				D_("Estimated time before empty:"), hms);
		}
		else
		{
			g_string_printf (sInfo, "%s\n"
				"%s %d%%\n"
				"%s %s",
				D_("Laptop on Charge."),
				D_("Battery charged at:"), (int)myData.iPercentage,
				D_("Estimated time before full:"), (myData.iPercentage > 99.9 ? "0" : hms));
		}
		g_free (hms);
		
		// static info
		if (myData.cVendor != NULL || myData.cModel != NULL)
		{
			g_string_append_printf (sInfo, "\n%s: %s %s", D_("Model"), myData.cVendor ? myData.cVendor : "", myData.cModel ? myData.cModel : "");
		}
		/*if (0&&myData.cTechnology != NULL) // if (0 && (...)) ??? :)
		{
			g_string_append_printf (sInfo, "\n%s: %s", D_("Technology"), myData.cTechnology);
		}*/
		if (myData.fMaxAvailableCapacity != 0)
		{
			g_string_append_printf (sInfo, "\n%s: %d%%", D_("Maximum capacity"), (int)myData.fMaxAvailableCapacity);
		}
	}
	else
	{
		g_string_assign (sInfo, D_("No battery found."));
	}
	
	_cd_powermanager_dialog (sInfo->str, 7);
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
		if (! myData.bIsHidden && myConfig.iNotificationType != 2)
		{
			CD_APPLET_DEMANDS_ATTENTION (myConfig.cNotificationAnimation, myConfig.iNotificationDuration);
		}
		if (myConfig.cSoundPath[alert] != NULL)
			cairo_dock_play_sound (myConfig.cSoundPath[alert]);
	}
	
	g_free (hms);
	g_string_free (sInfo, TRUE);
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
			cd_debug ("time: %d -> %d;%d", time, hours, minutes);
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

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


void update_icon(void)
{
	gboolean bNeedRedraw = FALSE;
	cd_message ("%s (time:%.2f -> %.2f ; charge:%.2f -> %.2f)", __func__, myData.previous_battery_time, myData.battery_time, myData.previous_battery_charge, myData.battery_charge);
	if(myData.battery_present)
	{
		// on reactualise le temps restant en info rapide.
		if (myData.previous_battery_time != myData.battery_time)
		{
			if(myConfig.quickInfoType == POWER_MANAGER_TIME)
			{
				if (myData.battery_time != 0) {
					CD_APPLET_SET_HOURS_MINUTES_AS_QUICK_INFO (myData.battery_time);
				}
				else {
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("-:--");
				}
			}
			else if(myConfig.quickInfoType == POWER_MANAGER_CHARGE)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int)myData.battery_charge);
			}
			else
			{
			  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			}
			
			bNeedRedraw = TRUE;
			myData.previous_battery_time = myData.battery_time;
		}
		
		// on prend en compte la nouvelle charge.
		if (myData.previously_on_battery != myData.on_battery || myData.previous_battery_charge != myData.battery_charge)
		{
			if (myData.previously_on_battery != myData.on_battery)
			{
				myData.previously_on_battery = myData.on_battery;
				myData.alerted = FALSE;  //On a changé de statut, donc on réinitialise les alertes
				myData.bCritical = FALSE;
			}
			
			// on redessine l'icone.
			if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE || myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
			{
				double fPercent = (double) myData.battery_charge / 100.;
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
				bNeedRedraw = FALSE;
			}
			else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
			{
				cd_powermanager_draw_icon_with_effect (myData.on_battery);
				bNeedRedraw = FALSE;
			}
			
			// on declenche les alarmes.
			if(myData.on_battery)
			{
				//Alert when battery charge is under a configured value in %
				if (myData.battery_charge <= myConfig.lowBatteryValue && ! myData.alerted)
				{
					cd_powermanager_alert(POWER_MANAGER_CHARGE_LOW);
					if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW] != NULL)
						cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_LOW]);
				}
				//Alert when battery charge is under 4%
				if (myData.battery_charge <= 4 && ! myData.bCritical)
				{
					myData.bCritical = TRUE;
					cd_powermanager_alert (POWER_MANAGER_CHARGE_CRITICAL);
					if (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL] != NULL)
						cairo_dock_play_sound (myConfig.cSoundPath[POWER_MANAGER_CHARGE_CRITICAL]);
				}
				//Embleme sur notre icône
				//CD_APPLET_DRAW_EMBLEM (CAIRO_DOCK_EMBLEM_BLANK, CAIRO_DOCK_EMBLEM_MIDDLE);
			}
			else
			{
				//Alert when battery is charged
				if(myData.battery_charge == 100 && ! myData.alerted)
					cd_powermanager_alert (POWER_MANAGER_CHARGE_FULL);
					
				//CD_APPLET_DRAW_EMBLEM (CAIRO_DOCK_EMBLEM_CHARGE, CAIRO_DOCK_EMBLEM_MIDDLE);
				CD_APPLET_DRAW_EMBLEM_ON_MY_ICON (myData.pEmblem);
			}
			
			myData.previously_on_battery = myData.on_battery;
			myData.previous_battery_charge = myData.battery_charge;
		}
	}
	else if (myData.prev_battery_present)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg");
		bNeedRedraw = TRUE;
		myData.prev_battery_present = FALSE;
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
	if (!myData.on_battery || !myData.battery_present)
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/sector.svg";
	else
		cIconPath = MY_APPLET_SHARE_DATA_DIR"/default-battery.svg";
	
	cd_debug ("%s (%s)", cInfo, cIconPath);
	cairo_dock_show_temporary_dialog_with_icon (cInfo, myIcon, myContainer, 1000*iDuration, cIconPath);
}

void cd_powermanager_bubble (void)
{
	GString *sInfo = g_string_new ("");
	if(myData.battery_present)
	{
		gchar *hms = NULL;
		if (myData.battery_time > 0.)
			hms = get_hours_minutes (myData.battery_time);
		else
			hms = g_strdup_printf ("%s", D_("Unknown"));
		if(myData.on_battery)
		{
			g_string_printf (sInfo, "%s %.2f%% \n %s %s", D_("Laptop on Battery.\n Battery charged at:"), myData.battery_charge, D_("Estimated time with charge:"), hms);
		}
		else
		{
			g_string_printf (sInfo, "%s %.2f%% \n %s %s", D_("Laptop on Charge.\n Battery charged at:"), myData.battery_charge, D_("Estimated charge time:"), hms);
		}
		g_free (hms);
	}
	else
	{
		g_string_printf (sInfo, "%s", D_("No battery found."));
	}
	
	_cd_powermanager_dialog (sInfo->str, 6000);
	g_string_free (sInfo, TRUE);
}

gboolean cd_powermanager_alert (MyAppletCharge alert)
{
	cd_debug ("%s", __func__);
	GString *sInfo = g_string_new ("");
	
	gchar *hms = NULL;
	if (myData.battery_time > 0.)
		hms = get_hours_minutes (myData.battery_time);
	else
		hms = g_strdup (D_("Unknown"));
		
	if ((alert == POWER_MANAGER_CHARGE_LOW && myConfig.lowBatteryWitness) || (alert == POWER_MANAGER_CHARGE_CRITICAL && myConfig.criticalBatteryWitness))
	{
		if (myConfig.iNotificationType != 1)
		{
			g_string_printf (sInfo, "%s (%.2f%%) \n %s %s \n %s", D_("PowerManager.\nBattery charge seems to be low"), myData.battery_charge, D_("Estimated time with charge:"), hms, D_("Please put your laptop on charge."));
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
			g_string_printf (sInfo, "%s (%.2f%%) \n %s %s ", D_("PowerManager.\nYour battery is now charged"), myData.battery_charge, D_("Estimated time with charge:"), hms);
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
	myData.alerted = TRUE;
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
			double fScale = .3 + .7 * myData.battery_charge / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
			cairo_restore (myDrawContext);
		break;
		case POWER_MANAGER_EFFECT_TRANSPARENCY :
			cairo_save (myDrawContext);
			double fAlpha = .3 + .7 * myData.battery_charge / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
			cairo_restore (myDrawContext);
		break;
		case POWER_MANAGER_EFFECT_BAR :
			cairo_save (myDrawContext);
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.battery_charge * .01);
			cairo_restore (myDrawContext);
		break;
		default :
		break;
	}
}

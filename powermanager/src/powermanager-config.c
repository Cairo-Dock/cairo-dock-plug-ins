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

#include <string.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 10);
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", POWER_MANAGER_TIME);
	
	
	myConfig.lowBatteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "low battery", TRUE);
	
	myConfig.highBatteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "high battery", TRUE);
	
	myConfig.criticalBatteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "critical battery", TRUE);
	
	myConfig.iNotificationType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "notifications", 2) + 1;
	myConfig.cNotificationAnimation = CD_CONFIG_GET_STRING ("Configuration", "battery_animation");
	myConfig.iNotificationDuration = CD_CONFIG_GET_INTEGER ("Configuration", "notif_duration");
	
	myConfig.lowBatteryValue = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "low value", 15);
	myConfig.bUseDBusFallback = CD_CONFIG_GET_BOOLEAN ("Configuration", "use_dbus");
	
	if (! g_key_file_has_key (CD_APPLET_MY_KEY_FILE, "Configuration", "renderer", NULL))  // old version.
	{
		myConfig.iDisplayType = (g_key_file_get_boolean (CD_APPLET_MY_KEY_FILE, "Configuration", "use gauge", NULL) ? CD_POWERMANAGER_GAUGE : CD_POWERMANAGER_ICONS);
		int dummy = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");
	}
	else
		myConfig.iDisplayType = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");
	
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.fBgColor);
	
	myConfig.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "effect", 0);
	myConfig.cUserBatteryIconName = CD_CONFIG_GET_STRING ("Configuration", "battery icon");
	myConfig.cUserChargeIconName = CD_CONFIG_GET_STRING ("Configuration", "charge icon");
	
	
	GString *sKeyName = g_string_new ("");
	int i;
	for (i = 0; i < POWER_MANAGER_NB_CHARGE_LEVEL; i ++) {
		g_string_printf (sKeyName, "sound_%d", i);
		myConfig.cSoundPath[i] = CD_CONFIG_GET_STRING ("Configuration", sKeyName->str);
	}
	g_string_free (sKeyName, TRUE);
	
	myConfig.bUseApprox = CD_CONFIG_GET_BOOLEAN ("Configuration", "use approx");
	
	myConfig.fLastDischargeMeanRate = CD_CONFIG_GET_DOUBLE ("Configuration", "discharge rate");
	myConfig.fLastChargeMeanRate = CD_CONFIG_GET_DOUBLE ("Configuration", "charge rate");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cUserBatteryIconName);
	g_free (myConfig.cUserChargeIconName);
	g_free (myConfig.cNotificationAnimation);
	
	int i;
	for (i = 0; i < POWER_MANAGER_NB_CHARGE_LEVEL; i ++) {
		g_free (myConfig.cSoundPath[i]);
	}
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN

	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	cairo_surface_destroy (myData.pSurfaceBattery);
	cairo_surface_destroy (myData.pSurfaceCharge);
	
	g_free (myData.cBatteryStateFilePath);
	
	cairo_dock_free_emblem (myData.pEmblem);
	
CD_APPLET_RESET_DATA_END

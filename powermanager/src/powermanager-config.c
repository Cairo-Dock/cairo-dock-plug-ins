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
	
	myConfig.batteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "battery witness", TRUE);
	
	myConfig.batteryWitnessAnimation = CD_CONFIG_GET_STRING ("Configuration", "battery_animation");
	
	myConfig.lowBatteryValue = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "low value", 15);
	myConfig.bUseDBusFallback = CD_CONFIG_GET_BOOLEAN ("Configuration", "use_dbus");
	
	
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
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cUserBatteryIconName);
	g_free (myConfig.cUserChargeIconName);
	g_free (myConfig.batteryWitnessAnimation);
	
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
	
CD_APPLET_RESET_DATA_END

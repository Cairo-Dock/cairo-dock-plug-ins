#include <string.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 10);
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", POWER_MANAGER_TIME);
	
	myConfig.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "effect", 0);
	myConfig.cUserBatteryIconName = CD_CONFIG_GET_STRING ("Configuration", "battery icon");
	myConfig.cUserChargeIconName = CD_CONFIG_GET_STRING ("Configuration", "charge icon");
	
	myConfig.lowBatteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "low battery", TRUE);
	
	myConfig.highBatteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "high battery", TRUE);
	
	myConfig.batteryWitness = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "battery witness", TRUE);
	
	myConfig.batteryWitnessAnimation = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "battery animation", 0);
	
	myConfig.lowBatteryValue = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "low value", 15);
	
	myConfig.bUseGauge = CD_CONFIG_GET_BOOLEAN ("Configuration", "use gauge");
	myConfig.cThemePath = cairo_dock_get_gauge_key_value(CD_APPLET_MY_CONF_FILE, pKeyFile, "Configuration", "theme", &bFlushConfFileNeeded, "battery");
	cd_message("gauge : Theme(%s)\n",myConfig.cThemePath);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cThemePath);
	g_free (myConfig.cUserBatteryIconName);
	g_free (myConfig.cUserChargeIconName);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN

	cairo_surface_destroy (myData.pSurfaceBattery);
	cairo_surface_destroy (myData.pSurfaceCharge);
	
	cairo_dock_free_gauge(myData.pGauge);
	
CD_APPLET_RESET_DATA_END


#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	
	GString *sKeyName = g_string_new ("");
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		g_string_printf (sKeyName, "icon_%d", i);
		myConfig.cUserImage[i] = CD_CONFIG_GET_STRING ("Configuration", sKeyName->str);
	}
	g_string_free (sKeyName, TRUE);
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", 1);
	myConfig.iCheckInterval = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 10);
	myConfig.dCheckInterval = myConfig.iCheckInterval;
	myConfig.hollowIcon 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "hollow");
	myConfig.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "effect", 0);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		g_free (myConfig.cUserImage[i]);
	}
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
CD_APPLET_RESET_DATA_END

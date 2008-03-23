
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	
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
CD_APPLET_CONFIG_END


void reset_config (void) {
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		g_free (myConfig.cUserImage[i]);
	}
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void) {
	
	g_source_remove (myData.checkTimer);
	myData.checkTimer = 0;
	
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	
	memset (&myData, 0, sizeof (AppletData));
}

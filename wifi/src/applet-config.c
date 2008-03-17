
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN("Wifi", "default.svg");
	reset_config ();
	
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.enableSSQ 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_ssq");
	
	myConfig.cDefault = CD_CONFIG_GET_STRING ("Configuration", "d icon");
	
	myConfig.c2Surface = CD_CONFIG_GET_STRING ("Configuration", "vl icon");
	myConfig.c4Surface = CD_CONFIG_GET_STRING ("Configuration", "l icon");
	myConfig.c6Surface = CD_CONFIG_GET_STRING ("Configuration", "m icon");
	myConfig.c8Surface	= CD_CONFIG_GET_STRING ("Configuration", "g icon");
	myConfig.c1Surface = CD_CONFIG_GET_STRING ("Configuration", "e icon");
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", 1);
CD_APPLET_CONFIG_END


void reset_config (void) {
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.c2Surface);
	myConfig.c2Surface = NULL;
	g_free (myConfig.c4Surface);
	myConfig.c4Surface = NULL;
	g_free (myConfig.c6Surface);
	myConfig.c6Surface = NULL;
	g_free (myConfig.c8Surface);
	myConfig.c8Surface = NULL;
	g_free (myConfig.c1Surface);
	myConfig.c1Surface = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void) {
	
	gtk_timeout_remove(myData.checkTimer);
	
	memset (&myData, 0, sizeof (AppletData));
}

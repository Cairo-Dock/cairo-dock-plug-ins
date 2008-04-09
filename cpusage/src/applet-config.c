
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_GET_CONFIG_BEGIN

	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 1000);
	myConfig.dCheckInterval = myConfig.iCheckInterval;
	//On charge le theme :
	myConfig.cThemePath = cairo_dock_get_gauge_key_value(CD_APPLET_MY_CONF_FILE, pKeyFile, "Configuration", "theme", &bFlushConfFileNeeded, "turbo-night-fuel");
	cd_message("gauge (cpusage) : Theme(%s)\n",myConfig.cThemePath);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cThemePath);
	myConfig.cThemePath = NULL;
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN	
	g_source_remove (myData.checkTimer);
	myData.checkTimer = 0;
CD_APPLET_RESET_DATA_END


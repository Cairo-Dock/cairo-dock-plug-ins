
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN("netspeed", "default.svg");
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
CD_APPLET_CONFIG_END


void reset_config (void) {
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void) {
	
	gtk_timeout_remove(myData.checkTimer);
	
	memset (&myData, 0, sizeof (AppletData));
}

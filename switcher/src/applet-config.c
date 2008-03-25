
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.

		myConfig.iNbDesks = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nombre de bureau", 8);
		myConfig.iNbRows = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "taille verticale", 1);
		myConfig.iNbCols = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "taille horizontale", 8);
		myConfig.bCurrentView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "complexe view", TRUE);
		myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", FALSE);
		myConfig.iCheckInterval = 60000 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
		myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
		myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
		myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
		cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");

CD_APPLET_CONFIG_END


void reset_config (void)
{
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	
	memset (&myData, 0, sizeof (AppletData));
}

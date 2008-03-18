
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
		myConfig.iNbDesks = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "Nombre de Bureau", 8);
	//myConfig.iNbDesks = MIN (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "Nombre de Bureau", SWITCHER_NB_DESKS_MAX), SWITCHER_NB_DESKS_MAX);
		myConfig.iNbRows = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "Taille Verticale", 1);
		myConfig.iNbCols = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "Taille Horizontale", 8);
		//myConfig.bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Configuration", "use separator");

		myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");

CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.cRenderer);
	myConfig.cRenderer = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	
	memset (&myData, 0, sizeof (AppletData));
}

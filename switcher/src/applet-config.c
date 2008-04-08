
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN
	//reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.

		myConfig.iNbDesks = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nombre de bureau", myData.switcher.ScreenCurrentNums),myData.switcher.ScreenCurrentNums;
		//myConfig.iNbRows = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "taille verticale", 1);
		//myConfig.iNbCols = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "taille horizontale", 8);
		myConfig.bCurrentView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Vue Simple", TRUE);
		myConfig.bShowSubDock = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "SubDock", TRUE);
		myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", FALSE);
		//myConfig.iCheckInterval = 60000 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
		myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
		//myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
		myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
		cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");

CD_APPLET_GET_CONFIG_END

CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cRenderer);

CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN

if (myData.pSurface != NULL)
	{
		cairo_surface_destroy (myData.pSurface);
	}
	if (myData.pBrokenSurface != NULL)
	{
		cairo_surface_destroy (myData.pBrokenSurface);
	}

if (myData.loadaftercompiz != 0)//\_______________________ On Tue le Timer.
{
cd_message ("timer = 0 ");
		g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
}
//if (myIcon->pSubDock != NULL)
//	{
//		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
//		myIcon->pSubDock = NULL;
//	}
	
CD_APPLET_RESET_DATA_END

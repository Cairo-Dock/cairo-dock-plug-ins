
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-load-icons.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN
	//reset_config ();
cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
cd_message ("Viewport X : %d", myData.switcher.iNbViewportX);
myConfig.iNbDesks = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nombre de bureau", myData.switcher.iNbViewportX);
		myConfig.bCurrentView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Vue Simple", TRUE);
		myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", TRUE);
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
		myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
		myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
		cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");

CD_APPLET_GET_CONFIG_END

CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cRenderer);

CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN


if (myData.LoadAfterCompiz != 0)//\_______________________ On Tue le Timer.
{
cd_message ("timer = 0 ");
		g_source_remove (myData.LoadAfterCompiz);
	myData.LoadAfterCompiz = 0;
}

CD_APPLET_RESET_DATA_END

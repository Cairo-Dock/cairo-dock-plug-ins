
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-load-icons.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN


	//\_________________ On recupere toutes les valeurs de notre fichier de conf.

		myConfig.bCurrentView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Vue Simple", TRUE);
		myConfig.bMapWallpaper = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Map Wallpaper", TRUE);
		myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", TRUE);
		myConfig.bInvertIndicator = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Invert Indicator", TRUE);
double inlinesize = 0.300;
		myConfig.cInLineSize = CD_CONFIG_GET_DOUBLE("Configuration", "inlinesize");
	double inlinecouleur[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbinlinecolor",myConfig.RGBInLineColors, inlinecouleur);
	double indcouleur[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbindcolor",myConfig.RGBIndColors, indcouleur);
double linesize = 0.300;
		myConfig.cLineSize = CD_CONFIG_GET_DOUBLE("Configuration", "linesize");
	double linecouleur[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgblinecolor",myConfig.RGBLineColors, linecouleur);
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
		myConfig.cDefaultSDockIcon = CD_CONFIG_GET_STRING ("Configuration", "default subdock icon");
		myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
		myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");

CD_APPLET_GET_CONFIG_END

CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cRenderer);

CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN

/* Fonction plus utile car j'ai enlevé le timer */
/*if (myData.LoadAfterCompiz != 0)//\_______________________ On Tue le Timer.
{
cd_message ("timer = 0 ");
		g_source_remove (myData.LoadAfterCompiz);
	myData.LoadAfterCompiz = 0;
}*/

CD_APPLET_RESET_DATA_END

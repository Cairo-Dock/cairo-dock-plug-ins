
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.bCompactView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Vue Simple", TRUE);
	myConfig.bPreserveScreenRatio = CD_CONFIG_GET_BOOLEAN ("Configuration", "preserve ratio");
	myConfig.bMapWallpaper = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Map Wallpaper", TRUE);
	myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", TRUE);
	
	double inlinesize = 0.300;
	myConfig.iInLineSize = CD_CONFIG_GET_INTEGER("Configuration", "inlinesize");
	double inlinecouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbinlinecolor",myConfig.RGBInLineColors, inlinecouleur);
	double indcouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbindcolor",myConfig.RGBIndColors, indcouleur);
	double linesize = 0.300;
	myConfig.iLineSize = CD_CONFIG_GET_INTEGER("Configuration", "linesize");
	double linecouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgblinecolor",myConfig.RGBLineColors, linecouleur);
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_FILE_PATH ("Configuration", "default icon", "default.svg");
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN ("Configuration", "3D desklet");
	myConfig.iDrawCurrentDesktopMode = CD_CONFIG_GET_INTEGER ("Configuration", "fill current");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cRenderer);
	g_free (myConfig.cDefaultIcon);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL)
	{
		CD_APPLET_DESTROY_MY_SUBDOCK
	}
	cairo_surface_destroy (myData.pDefaultMapSurface);
CD_APPLET_RESET_DATA_END

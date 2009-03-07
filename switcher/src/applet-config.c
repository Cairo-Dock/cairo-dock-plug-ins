/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Cchumi & Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-load-icons.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.bCompactView = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Vue Simple", TRUE);
	myConfig.bPreserveScreenRatio = CD_CONFIG_GET_BOOLEAN ("Configuration", "preserve ratio");
	myConfig.bMapWallpaper = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Map Wallpaper", TRUE);
	myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", TRUE);
	myConfig.bDrawWindows = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Draw Windows", TRUE);
	myConfig.bDisplayHiddenWindows = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Draw hidden Windows", TRUE);
	
	// couleur des lignes interieures
	myConfig.iInLineSize = CD_CONFIG_GET_INTEGER("Configuration", "inlinesize");
	double inlinecouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbinlinecolor",myConfig.RGBInLineColors, inlinecouleur);
	
	// couleur du bureau courant.
	double indcouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbindcolor",myConfig.RGBIndColors, indcouleur);
	myConfig.iDrawCurrentDesktopMode = CD_CONFIG_GET_INTEGER ("Configuration", "fill current");
	
	// couleur des lignes exterieures.
	myConfig.iLineSize = CD_CONFIG_GET_INTEGER("Configuration", "linesize");
	double linecouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgblinecolor",myConfig.RGBLineColors, linecouleur);
	
	// couleur des traits des fenetres.
	double wlinecouleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "rgbwlinecolor",myConfig.RGBWLineColors, wlinecouleur);

	myConfig.cDefaultIcon = CD_CONFIG_GET_FILE_PATH ("Configuration", "default icon", "default.svg");
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN ("Configuration", "3D desklet");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cRenderer);
	g_free (myConfig.cDefaultIcon);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	CD_APPLET_DELETE_MY_ICONS_LIST;
	cairo_surface_destroy (myData.pDefaultMapSurface);
CD_APPLET_RESET_DATA_END

/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

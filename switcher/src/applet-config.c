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
	myConfig.bCompactView = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "view", 1);
	myConfig.bPreserveScreenRatio = CD_CONFIG_GET_BOOLEAN ("Configuration", "preserve ratio");
	myConfig.iIconDrawing = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "icon drawing", 0);
	myConfig.bDisplayNumDesk = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display numero desktop", TRUE);
	myConfig.bDrawWindows = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Draw Windows", TRUE);
	myConfig.bDisplayHiddenWindows = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Draw hidden Windows", TRUE);
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "action on click", 0);
	myConfig.iDesktopsLayout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "layout", SWICTHER_LAYOUT_AUTO);
	myConfig.bDrawIcons = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "Draw icons", TRUE);
	
	myConfig.bUseDefaultColors = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "style", 1) == 0);
	
	if (myConfig.bUseDefaultColors)
	{
		myConfig.iInLineSize = myConfig.iLineSize = myStyleParam.iLineWidth;
	}
	else
	{
		// color of internal lines
		myConfig.iInLineSize = CD_CONFIG_GET_INTEGER("Configuration", "inlinesize");
		double inlinecolor[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgbinlinecolor",myConfig.RGBInLineColors, inlinecolor);
		
		// color of the current desktop
		double indcolor[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgbindcolor",myConfig.RGBIndColors, indcolor);
		// color of external lines
		myConfig.iLineSize = CD_CONFIG_GET_INTEGER("Configuration", "linesize");
		double linecolor[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgblinecolor",myConfig.RGBLineColors, linecolor);
		
		// color of windows' lines
		double wlinecolor[4] = {0., 0., 0.5, 1.};
		CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgbwlinecolor", myConfig.RGBWLineColors, wlinecolor);
		
		// color of the background
		double fillbcolor[4] = {0., 0., 0., 1.};
		CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgbbgcolor", myConfig.RGBBgColors, fillbcolor);
	}
	
	myConfig.iDrawCurrentDesktopMode = CD_CONFIG_GET_INTEGER ("Configuration", "fill current");
	
	// color of windows
	myConfig.bFillAllWindows = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "fill windows", FALSE);
	double fillwcolor[4] = {0.33, 0.33, 0.33, 1.};
	CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Configuration", "rgbfindcolor", myConfig.RGBWFillColors, fillwcolor);
	
	if (myConfig.iIconDrawing == SWICTHER_MAP_IMAGE)
		myConfig.cDefaultIcon = CD_CONFIG_GET_FILE_PATH ("Configuration", "default icon", "default.svg");
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	
	gsize iNbNamesSize;
	myConfig.cDesktopNames = CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT ("Configuration", "desktop names", &iNbNamesSize, "Work;Game;Video;Chat");  // used to be a #U widget, but since it can be set from different ways (the applet's menu or the desktop tools), it's not useful to have it in the config
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
	g_free (myConfig.cDefaultIcon);
	if (myConfig.cDesktopNames != NULL)
		g_strfreev (myConfig.cDesktopNames);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	CD_APPLET_DELETE_MY_ICONS_LIST;
	cairo_surface_destroy (myData.pDefaultMapSurface);
	cairo_surface_destroy (myData.pDesktopBgMapSurface);
	if (myData.cDesktopNames != NULL)
		g_strfreev (myData.cDesktopNames);
CD_APPLET_RESET_DATA_END

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

#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iComicsRadius = CD_CONFIG_GET_INTEGER ("Comics", "corner");
	myConfig.iComicsLineWidth = CD_CONFIG_GET_INTEGER ("Comics", "border");
	CD_CONFIG_GET_COLOR ("Comics", "line color", &myConfig.fComicsLineColor);
	
	myConfig.iModernRadius = CD_CONFIG_GET_INTEGER ("Modern", "corner");
	myConfig.iModernLineWidth = CD_CONFIG_GET_INTEGER ("Modern", "border");
	CD_CONFIG_GET_COLOR ("Modern", "line color", &myConfig.fModernLineColor);
	myConfig.iModernLineSpacing = CD_CONFIG_GET_INTEGER ("Modern", "line space");
	
	/**myConfig.iPlaneRadius = CD_CONFIG_GET_INTEGER ("3D plane", "corner");
	myConfig.iPlaneLineWidth = CD_CONFIG_GET_INTEGER ("3D plane", "border");
	CD_CONFIG_GET_COLOR ("3D plane", "line color", &myConfig.fPlaneLineColor);
	CD_CONFIG_GET_COLOR ("3D plane", "plane color", &myConfig.fPlaneColor);*/
	
	myConfig.iTooltipRadius = CD_CONFIG_GET_INTEGER ("Tooltip", "corner");
	myConfig.iTooltipLineWidth = CD_CONFIG_GET_INTEGER ("Tooltip", "border");
	CD_CONFIG_GET_COLOR ("Tooltip", "line color", &myConfig.fTooltipLineColor);
	CD_CONFIG_GET_COLOR ("Tooltip", "margin color", &myConfig.fTooltipMarginColor);
	
	myConfig.iCurlyRadius = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Curly", "corner", 12);
	myConfig.iCurlyLineWidth = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Curly", "border", 1);
	CD_CONFIG_GET_COLOR ("Curly", "line color", &myConfig.fCurlyLineColor);
	myConfig.fCurlyCurvature = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Curly", "curvature", 1.5);
	myConfig.bCulrySideToo = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Curly", "side too", FALSE);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

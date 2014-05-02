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
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	
	myConfig.cInterface = CD_CONFIG_GET_STRING ("Configuration", "interface");
	// if NULL, monitor all interfaces
	myConfig.iStringLen = myConfig.cInterface ? strlen (myConfig.cInterface) : 0;
	
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");

	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	myConfig.iRotateTheme = CD_CONFIG_GET_INTEGER ("Configuration", "rotate theme");
	/*myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "watermark alpha");
	if (myConfig.fAlpha != 0)
	{
		myConfig.cWatermarkImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "watermark image", MY_APPLET_ICON_FILE);
	}*/
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR_RGBA ("Configuration", "bg color", myConfig.fBgColor);
	//CD_CONFIG_GET_COLOR_RGB ("Configuration", "low color2", myConfig.fLowColor2);
	//CD_CONFIG_GET_COLOR_RGB ("Configuration", "high color2", myConfig.fHigholor2);
	myConfig.bMixGraph = CD_CONFIG_GET_BOOLEAN ("Configuration", "mix graph");
	
	myConfig.cSystemMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "sys monitor");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cInterface);
	g_free (myConfig.cWatermarkImagePath);
	g_free (myConfig.cSystemMonitorCommand);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pPeriodicTask);
	
	if (myData.dbus_proxy_nm != NULL)
		g_object_unref (myData.dbus_proxy_nm);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	g_timer_destroy (myData.pClock);
CD_APPLET_RESET_DATA_END


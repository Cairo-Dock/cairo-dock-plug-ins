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
#include "applet-disks.h"


CD_APPLET_GET_CONFIG_BEGIN
	///\_________________ General
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");

	///\_________________ Renderer
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");

	///\_________________ Gauge
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	myConfig.iRotateTheme = CD_CONFIG_GET_INTEGER ("Configuration", "rotate theme");
		
	///\_________________ Graph
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR_RGBA ("Configuration", "bg color", myConfig.fBgColor);
	myConfig.bMixGraph = CD_CONFIG_GET_BOOLEAN ("Configuration", "mix graph");
	
	///\_________________ Display
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");

	///\_________________ Parameters
	myConfig.cDisks = CD_CONFIG_GET_STRING_LIST ("Configuration", "disks", &myConfig.iNumberDisks);
	myConfig.cParts = CD_CONFIG_GET_STRING_LIST ("Configuration", "partitions", &myConfig.iNumberParts);
	myConfig.cSystemMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "sys monitor");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	g_strfreev (myConfig.cDisks);
	g_free (myConfig.cWatermarkImagePath);
	g_free (myConfig.cSystemMonitorCommand);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pPeriodicTask);
	
	if (myData.dbus_proxy_nm != NULL)
		g_object_unref (myData.dbus_proxy_nm);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	g_timer_destroy (myData.pClock);
	
	cd_disks_reset_parts_list (myApplet);
	cd_disks_reset_disks_list (myApplet);
CD_APPLET_RESET_DATA_END

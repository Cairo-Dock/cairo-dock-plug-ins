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
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 10);
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	
	myConfig.cUserCommand = CD_CONFIG_GET_STRING ("Configuration", "command");
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", WIFI_INFO_SIGNAL_STRENGTH_LEVEL);
	
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "renderer", 0);
	
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RGB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR_RGBA ("Configuration", "bg color", myConfig.fBgColor);
	
	if (! g_key_file_has_key (CD_APPLET_MY_KEY_FILE, "Configuration", "default_icon", NULL))  // new option -> get the previous "excellent" user icon
	{
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "icon_5");
		g_key_file_set_string (CD_APPLET_MY_KEY_FILE, "Configuration", "default_icon", myConfig.cDefaultIcon?myConfig.cDefaultIcon:"");
		myConfig.cNoSignalIcon = CD_CONFIG_GET_STRING ("Configuration", "icon_0");
		g_key_file_set_string (CD_APPLET_MY_KEY_FILE, "Configuration", "no_signal_icon", myConfig.cDefaultIcon?myConfig.cDefaultIcon:"");
	}
	else
	{
		myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default_icon");
		myConfig.cNoSignalIcon = CD_CONFIG_GET_STRING ("Configuration", "no_signal_icon");
	}
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cUserCommand);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	g_free (myData.cESSID);
	g_free (myData.cInterface);
	g_free (myData.cAccessPoint);
	g_free (myData.cIWConfigPath);

CD_APPLET_RESET_DATA_END

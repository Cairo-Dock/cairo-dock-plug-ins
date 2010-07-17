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
#include "applet-load-icons.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cDefaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cImageFile = CD_CONFIG_GET_STRING ("Icon", "image file");
	myConfig.cDirPath = CD_CONFIG_GET_STRING ("Configuration", "dir path");
	if (myConfig.cDirPath && *myConfig.cDirPath == '/')
	{
		gchar *tmp = myConfig.cDirPath;
		myConfig.cDirPath = g_filename_to_uri (myConfig.cDirPath, NULL, NULL);
		g_free (tmp);
	}
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.iNbIcons = CD_CONFIG_GET_INTEGER ("Configuration", "nb icons");
	myConfig.iSortType = CD_CONFIG_GET_INTEGER ("Configuration", "sort type");
	myConfig.bFoldersFirst = CD_CONFIG_GET_BOOLEAN ("Configuration", "folders first");
	myConfig.bShowHiddenFiles = CD_CONFIG_GET_BOOLEAN ("Configuration", "show hidden");
	myConfig.iSubdockViewType = CD_CONFIG_GET_INTEGER ("Icon", "view type");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cImageFile);
	g_free (myConfig.cDefaultTitle);
	
	if (myConfig.cDirPath)
	{
		cairo_dock_fm_remove_monitor_full (myConfig.cDirPath, TRUE, NULL);
		g_free (myConfig.cDirPath);
	}
	
	g_free (myConfig.cRenderer);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_folders_free_all_data (myApplet);
CD_APPLET_RESET_DATA_END

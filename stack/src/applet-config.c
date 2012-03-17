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


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.iDeskletRendererType = CD_CONFIG_GET_INTEGER ("Configuration", "desklet renderer");

	gsize length = 0;
	myConfig.cMimeTypes = CD_CONFIG_GET_STRING_LIST ("Configuration", "mime", &length);
	
	myConfig.bFilter = CD_CONFIG_GET_BOOLEAN ("Configuration", "filter");
	
	myConfig.iSortType = CD_CONFIG_GET_INTEGER ("Configuration", "sort by");
	myConfig.bSelectionClipBoard = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "selection_", FALSE);
	
	myConfig.cTextIcon = CD_CONFIG_GET_FILE_PATH ("Configuration", "text icon", NULL);
	if (myConfig.cTextIcon == NULL)
	{
		myConfig.cTextIcon = cairo_dock_search_icon_s_path ("text-x-generic", MAX (myIcon->iImageWidth, myIcon->iImageHeight));
		if (myConfig.cTextIcon == NULL)
			myConfig.cTextIcon = g_strdup (MY_APPLET_SHARE_DATA_DIR"/"CD_STACK_DEFAULT_TEXT_ICON);
	}
	
	myConfig.cUrlIcon = CD_CONFIG_GET_FILE_PATH ("Configuration", "url icon", NULL);
	if (myConfig.cUrlIcon == NULL)
	{
		myConfig.cUrlIcon = cairo_dock_search_icon_s_path ("text-html", MAX (myIcon->iImageWidth, myIcon->iImageHeight));
		if (myConfig.cUrlIcon == NULL)
			myConfig.cUrlIcon = g_strdup (MY_APPLET_SHARE_DATA_DIR"/"CD_STACK_DEFAULT_TEXT_ICON);
	}
	
	myConfig.cStackDir = CD_CONFIG_GET_STRING ("Configuration", "stack dir");
	
	myConfig.bSeparateTypes = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "separate", TRUE);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	
	g_strfreev (myConfig.cMimeTypes);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cTextIcon);
	g_free (myConfig.cUrlIcon);
	g_free (myConfig.cStackDir);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
CD_APPLET_RESET_DATA_END

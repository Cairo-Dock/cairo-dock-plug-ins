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
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-trashes-manager.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	CD_CONFIG_RENAME_GROUP ("Module", "Configuration");
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "default");
	myConfig.cEmptyUserImage = CD_CONFIG_GET_STRING ("Configuration", "empty image");
	myConfig.cFullUserImage = CD_CONFIG_GET_STRING ("Configuration", "full image");
	
	myConfig.iQuickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick info", CD_DUSTBIN_INFO_NB_TRASHES);
	myConfig.bAskBeforeDelete = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "confirm", TRUE);
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER ("Configuration", "middle click");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cThemePath);
	g_free (myConfig.cEmptyUserImage);
	g_free (myConfig.cFullUserImage);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

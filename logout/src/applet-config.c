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
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "middle-click", 2);
	myConfig.cShortkey = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Super>L");
	myConfig.cShortkey2 = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey2", "<Control>F12");
	myConfig.bConfirmAction = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "confirm action", TRUE);
	myConfig.cUserAction = CD_CONFIG_GET_STRING ("Configuration", "user action");
	myConfig.cUserAction2 = CD_CONFIG_GET_STRING ("Configuration", "user action2");
	myConfig.iShutdownTime = CD_CONFIG_GET_INTEGER ("Configuration", "shutdown time");
	myConfig.cEmblemPath = CD_CONFIG_GET_STRING ("Configuration", "emblem");
	myConfig.cDefaultLabel = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Icon", "icon");
	myConfig.iRebootNeededImage = CD_CONFIG_GET_INTEGER ("Configuration", "replace image");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cUserAction);
	g_free (myConfig.cUserAction2);
	g_free (myConfig.cDefaultLabel);
	g_free (myConfig.cDefaultIcon);
	g_free (myConfig.cEmblemPath);
	g_free (myConfig.cShortkey);
	g_free (myConfig.cShortkey2);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	g_free (myData.cSessionMigrationFileName);
CD_APPLET_RESET_DATA_END

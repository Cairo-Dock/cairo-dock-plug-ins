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
#include "applet-notifications.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bShowDesklets = CD_CONFIG_GET_BOOLEAN ("Configuration", "show desklets");
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "middle click", 1);
	myConfig.cShortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Shift><Ctrl>F4");
	myConfig.cHiddenImage = CD_CONFIG_GET_STRING ("Icon", "icon");
	if (myConfig.cHiddenImage == NULL)
		myConfig.cHiddenImage = g_strdup (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	myConfig.cVisibleImage = CD_CONFIG_GET_STRING ("Icon", "icon visible");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) on_keybinding_pull);
	g_free (myConfig.cShortcut);
	g_free (myConfig.cVisibleImage);
	g_free (myConfig.cHiddenImage);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

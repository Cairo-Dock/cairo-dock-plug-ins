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


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	guint iSteal = CD_CONFIG_GET_INTEGER ("Configuration", "steal");
	myConfig.bDisplayMenu = (iSteal == 0 || iSteal == 2);
	myConfig.bDisplayControls = (iSteal == 1 || iSteal == 2);
	myConfig.cShortkey = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	myConfig.bMenuOnMouse = CD_CONFIG_GET_BOOLEAN ("Configuration", "menu on mouse");
	myConfig.bCompactMode = CD_CONFIG_GET_BOOLEAN ("Configuration", "compact");
	myConfig.iTransitionDuration = 500;
	myConfig.cMinimizeImage = CD_CONFIG_GET_FILE_PATH ("Configuration", "min button", "button-min.svg");
	myConfig.cMaximizeImage = CD_CONFIG_GET_FILE_PATH ("Configuration", "max button", "button-max.svg");
	myConfig.cRestoreImage = CD_CONFIG_GET_FILE_PATH ("Configuration", "restore button", "button-restore.svg");
	myConfig.cCloseImage = CD_CONFIG_GET_FILE_PATH ("Configuration", "close button", "button-close.svg");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cShortkey);
	g_free (myConfig.cMinimizeImage);
	g_free (myConfig.cMaximizeImage);
	g_free (myConfig.cRestoreImage);
	g_free (myConfig.cCloseImage);
CD_APPLET_RESET_CONFIG_END

		
//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

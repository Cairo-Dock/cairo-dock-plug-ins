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
	myConfig.bEnablePopUp 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable pop-up");
	myConfig.bEnableReboot 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable reboot");
	myConfig.bEnableDesklets 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable desklets");
	myConfig.bEnableQuit 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable quit");
	myConfig.bEnableShowDock 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable show dock");
	myConfig.bEnableSetQuickInfo	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set quickinfo");
	myConfig.bEnableSetLabel 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set label");
	myConfig.bEnableSetIcon 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set icon");
	myConfig.bEnableAnimateIcon		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable animate icon");
	myConfig.bEnableAddRemove		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable add remove");
	myConfig.bEnableGetProps		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable get properties");
	myConfig.bEnableSetMenu			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set menu");
	myConfig.bEnableSetProgress		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set progress");
	myConfig.bLaunchLauncherAPIDaemon = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "launcher api daemon", TRUE);
CD_APPLET_GET_CONFIG_END

CD_APPLET_RESET_CONFIG_BEGIN

CD_APPLET_RESET_CONFIG_END


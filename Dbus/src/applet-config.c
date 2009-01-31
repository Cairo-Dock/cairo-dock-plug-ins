/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bEnablePopUp 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable pop-up");
	myConfig.bEnableReboot 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable reboot");
	myConfig.bEnableDesklets 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable desklets");
	myConfig.bEnableReloadModule	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable reload module");
	myConfig.bEnableShowDock 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable show dock");
	myConfig.bEnableLoadLauncher	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable add launcher");
	myConfig.bEnableCreateLauncher	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable add launcher");
	myConfig.bEnableSetQuickInfo	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set quickinfo");
	myConfig.bEnableSetLabel 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable set label");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

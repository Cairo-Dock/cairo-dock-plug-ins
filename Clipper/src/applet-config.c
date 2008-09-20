/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iNbItems = CD_CONFIG_GET_INTEGER ("Configuration", "nb items");
	myConfig.iItemType = CD_CONFIG_GET_INTEGER ("Configuration", "item type");
	myConfig.bPasteInClipboard = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste clipboard");
	myConfig.bPasteInPrimary = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste selection");
	myConfig.bEnableActions = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable actions");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	g_list_foreach (myData.pItems, (GFunc) g_free, NULL);
	g_list_free (myData.pItems);
	
CD_APPLET_RESET_DATA_END

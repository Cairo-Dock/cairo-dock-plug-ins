/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-theme.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iLoadingModifier = (CD_CONFIG_GET_BOOLEAN ("Configuration", "keep ratio") ? CAIRO_DOCK_KEEP_RATIO : 0);
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Classic");
	myConfig.iWinkDelay = CD_CONFIG_GET_INTEGER ("Configuration", "wink delay");
	myConfig.iWinkDuration = CD_CONFIG_GET_INTEGER ("Configuration", "wink duration");
	myConfig.bFastCheck = CD_CONFIG_GET_BOOLEAN ("Configuration", "fast");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cThemePath);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_xeyes_unload_theme (myApplet);
	
CD_APPLET_RESET_DATA_END

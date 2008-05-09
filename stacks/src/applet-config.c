/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	//cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");

	gsize length = 0;
	myConfig.cMimeTypes = CD_CONFIG_GET_STRING_LIST ("Configuration", "mime", &length);

  myConfig.cMonitoredDirectory = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "directory", "/");
	myConfig.bHiddenFiles = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "hidden", FALSE);
	myConfig.bLocalDir = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "local", TRUE);
	myConfig.bFilter = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "filter", FALSE);
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	g_strfreev (myConfig.cMimeTypes);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cMonitoredDirectory);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL) {
		CD_APPLET_DESTROY_MY_SUBDOCK
	}
	
CD_APPLET_RESET_DATA_END

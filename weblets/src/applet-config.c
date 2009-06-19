/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN

	gsize length = 0;
	myConfig.cURI_to_load = CD_CONFIG_GET_STRING ("Configuration", "weblet URI");
	myConfig.bShowScrollbars = CD_CONFIG_GET_BOOLEAN ("Configuration", "show scrollbars");
	myConfig.bIsTransparent = CD_CONFIG_GET_BOOLEAN ("Configuration", "transparent background");
	myConfig.iPosScrollX = CD_CONFIG_GET_INTEGER ("Configuration", "scroll x");
	myConfig.iPosScrollY = CD_CONFIG_GET_INTEGER ("Configuration", "scroll y");
	myConfig.iReloadTimeout = CD_CONFIG_GET_INTEGER ("Configuration", "reload timeout");
	myConfig.cListURI = CD_CONFIG_GET_STRING_LIST ("Configuration", "uri list", &length);
	myConfig.iRightMargin = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "right margin", 5);

	if (myConfig.cListURI == NULL) {
		g_key_file_set_string (CD_APPLET_MY_KEY_FILE, "Configuration", "uri list", "www.cairo-dock.org;www.google.com;m.google.com/mail;www.rememberthemilk.com/services/modules/googleig;https://www.meebo.com/mobile;https://www.pandora.com/radio/tuner_8_7_0_0_pandora.swf;http://digg.com/iphone#_stories;http://www.bashfr.org/?sort=top50;about:plugins");
		cairo_dock_write_keys_to_file (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE);
		myConfig.cListURI = CD_CONFIG_GET_STRING_LIST ("Configuration", "uri list", &length);
	}
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.cURI_to_load);
	g_strfreev (myConfig.cListURI);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	if( myData.pRefreshTimer )
	{
		cairo_dock_free_task( myData.pRefreshTimer );
		myData.pRefreshTimer = NULL;
	}
	cd_weblet_free_uri_list ();
CD_APPLET_RESET_DATA_END

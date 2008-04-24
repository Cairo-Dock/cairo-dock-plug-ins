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


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bListDrives = CD_CONFIG_GET_BOOLEAN ("Module", "list drives");
	myConfig.bListNetwork = CD_CONFIG_GET_BOOLEAN ("Module", "list network");
	myConfig.bListBookmarks = CD_CONFIG_GET_BOOLEAN ("Module", "list bookmarks");
	myConfig.bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Module", "use separator");
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Module", "renderer");
	//cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Module", "renderer");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.cDisksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cDisksURI, FALSE, NULL);
		g_free (myData.cDisksURI);
	}
	if (myData.cNetworkURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cNetworkURI, FALSE, NULL);
		g_free (myData.cNetworkURI);
	}
	if (myData.cBookmarksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cBookmarksURI, FALSE, NULL);
		g_free (myData.cBookmarksURI);
	}
	
	if (myIcon->pSubDock != NULL)
	{
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);  // detruit les icones avec.
		myIcon->pSubDock = NULL;  // normalement inutile.
	}
CD_APPLET_RESET_DATA_END

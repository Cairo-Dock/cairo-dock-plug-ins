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

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN ("Shortcuts", "gnome-main-menu")
	reset_config ();
	
	myConfig.bListDrives = CD_CONFIG_GET_BOOLEAN ("Module", "list drives");
	myConfig.bListNetwork = CD_CONFIG_GET_BOOLEAN ("Module", "list network");
	myConfig.bListBookmarks = CD_CONFIG_GET_BOOLEAN ("Module", "list bookmarks");
	myConfig.bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Module", "use separator");
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Module", "renderer");
	cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_CONF_FILE, "Module", "renderer");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.cRenderer);
	myConfig.cRenderer = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	cairo_dock_fm_remove_monitor_full (cBookmarkFilePath, FALSE, NULL);
	g_free (cBookmarkFilePath);
	
	cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
	myIcon->pSubDock = NULL;  // normalement inutile.
	
	memset (&myData, 0, sizeof (AppletData));
}

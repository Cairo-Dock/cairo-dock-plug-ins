/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
#include <cairo-dock.h>

#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

extern gboolean my_bListDrives;
extern gboolean my_bListNetwork;
extern gboolean my_bListBookmarks;
extern gboolean my_bUseSeparator;
extern gchar *my_cRenderer;

CD_APPLET_CONFIG_BEGIN ("Shortcuts", "gnome-main-menu")
	my_bListDrives = CD_CONFIG_GET_BOOLEAN ("Module", "list drives");
	my_bListNetwork = CD_CONFIG_GET_BOOLEAN ("Module", "list network");
	my_bListBookmarks = CD_CONFIG_GET_BOOLEAN ("Module", "list bookmarks");
	my_bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Module", "use separator");
	
	my_cRenderer = CD_CONFIG_GET_STRING ("Module", "renderer");
	cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_CONF_FILE, "Module", "renderer");
CD_APPLET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	cairo_dock_fm_remove_monitor_full (cBookmarkFilePath, FALSE, NULL);
	g_free (cBookmarkFilePath);
	
	cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
	g_print ("  myIcon->pSubDock <- %x\n", myIcon->pSubDock);
	myIcon->pSubDock = NULL;  // normalement inutile.
	
	g_free (my_cRenderer);
	my_cRenderer = NULL;
CD_APPLET_RESET_DATA_END

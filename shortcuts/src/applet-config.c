
#include <cairo-dock.h>

#include "applet-config.h"

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

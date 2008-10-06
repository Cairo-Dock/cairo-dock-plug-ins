
#ifndef __APPLET_UTIL__
#define  __APPLET_UTIL__

#include <cairo-dock.h>
#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>
#include "applet-struct.h"

#define         string_empty(s)         ((s)==NULL||((char *)(s))[0]=='\0')

#ifdef HAVE_GIO
char * panel_util_get_icon_name_from_g_icon (GIcon *gicon);

GdkPixbuf * panel_util_get_pixbuf_from_g_loadable_icon (GIcon *gicon,
					    int    size);
#endif

void panel_menu_item_activate_desktop_file (GtkWidget  *menuitem,
				       const char *path);

char * panel_util_icon_remove_extension (const char *icon);

void panel_util_set_tooltip_text (GtkWidget  *widget,
			     const char *text);

char * menu_escape_underscores_and_prepend (const char *text);

char * panel_find_icon (GtkIconTheme  *icon_theme,
		 const char    *icon_name,
		 gint           size);

#ifdef HAVE_GIO
GdkPixbuf * panel_util_gdk_pixbuf_load_from_stream (GInputStream  *stream);
#endif

#endif

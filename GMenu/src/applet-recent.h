
#ifndef __APPLET_RECENT__
#define  __APPLET_RECENT__

#include <cairo-dock.h>


void cd_menu_append_recent_to_menu (GtkWidget *top_menu, GtkRecentManager *manager);


void cd_menu_clear_recent (GtkMenuItem *menu_item, gpointer data);


#endif

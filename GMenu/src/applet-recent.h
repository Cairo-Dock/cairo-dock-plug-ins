
#ifndef __APPLET_RECENT__
#define  __APPLET_RECENT__

#include <cairo-dock.h>


void cd_menu_append_recent_to_menu (GtkWidget *top_menu, CairoDockModuleInstance *myApplet);


void cd_menu_clear_recent (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet);


void cd_menu_init_recent (CairoDockModuleInstance *myApplet);

void cd_menu_reset_recent (CairoDockModuleInstance *myApplet);


#endif

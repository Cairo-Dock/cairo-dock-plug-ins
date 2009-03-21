
#ifndef __APPLET_MENU__
#define  __APPLET_MENU__

#include <cairo-dock.h>


GtkWidget *cd_quick_browser_make_menu_from_dir (const gchar *cDirPath, CairoDockModuleInstance *myApplet);


void cd_quick_browser_destroy_menu (CairoDockModuleInstance *myApplet);


void cd_quick_browser_show_menu (CairoDockModuleInstance *myApplet);


#endif

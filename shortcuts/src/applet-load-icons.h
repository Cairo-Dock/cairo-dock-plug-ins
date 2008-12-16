
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__


#include <cairo-dock.h>


void cd_shortcuts_get_shortcuts_data (CairoDockModuleInstance *myApplet);

gboolean cd_shortcuts_build_shortcuts_from_data (CairoDockModuleInstance *myApplet);


#endif

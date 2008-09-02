
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__


#include <cairo-dock.h>

void cd_stack_destroy_icons (CairoDockModuleInstance *myApplet);

Icon *cd_stack_build_one_icon (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile);
Icon *cd_stack_build_one_icon_from_file (CairoDockModuleInstance *myApplet, gchar *cDesktopFilePath);
GList *cd_stack_insert_icon_in_list (CairoDockModuleInstance *myApplet, GList *pIconsList, Icon *pIcon);
GList *cd_stack_build_icons_list (CairoDockModuleInstance *myApplet, gchar *cStackDir);
void cd_stack_build_icons (CairoDockModuleInstance *myApplet);

#endif

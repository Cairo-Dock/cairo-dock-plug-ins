
#ifndef __APPLET_STACK__
#define  __APPLET_STACK__

#include <cairo-dock.h>

GList* cd_stack_mime_filter(GList *pList);

void cd_stack_check_local (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile);
void cd_stack_clear_stack (CairoDockModuleInstance *myApplet);

void cd_stack_remove_item (CairoDockModuleInstance *myApplet, Icon *pIcon);


Icon *cd_stack_create_item (CairoDockModuleInstance *myApplet, const gchar *cStackDirectory, const gchar *cContent);
void cd_stack_create_and_load_item (CairoDockModuleInstance *myApplet, const gchar *cContent);
void cd_stack_set_item_name (const gchar *cDesktopFilePath, const gchar *cName);
void cd_stack_set_item_order (const gchar *cDesktopFilePath, double fOrder);

#endif

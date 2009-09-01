
#ifndef __APPLET_MONITOR__
#define  __APPLET_MONITOR__

#include <cairo-dock.h>

void cd_sysmonitor_get_data (CairoDockModuleInstance *myApplet);

gboolean cd_sysmonitor_update_from_data (CairoDockModuleInstance *myApplet);


#endif

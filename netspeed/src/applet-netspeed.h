#ifndef __APPLET_NETSPEED__
#define  __APPLET_NETSPEED__

#include <cairo-dock.h>

void cd_netspeed_get_data (CairoDockModuleInstance *myApplet);

gboolean cd_netspeed_update_from_data (CairoDockModuleInstance *myApplet);

#endif

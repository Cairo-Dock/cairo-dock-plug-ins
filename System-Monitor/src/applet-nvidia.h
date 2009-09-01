#ifndef __APPLET_NVIDIA__
#define __APPLET_NVIDIA__

#include <cairo-dock.h>


void cd_sysmonitor_get_nvidia_data (CairoDockModuleInstance *myApplet);


void cd_sysmonitor_get_nvidia_info (CairoDockModuleInstance *myApplet);


void cd_nvidia_alert (CairoDockModuleInstance *myApplet);


#endif

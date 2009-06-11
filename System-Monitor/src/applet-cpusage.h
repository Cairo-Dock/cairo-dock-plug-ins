
#ifndef __APPLET_CPUSAGE__
#define  __APPLET_CPUSAGE__

#include <cairo-dock.h>


void cd_sysmonitor_get_uptime (gchar **cUpTime, gchar **cActivityTime);

void cd_sysmonitor_get_cpu_info (CairoDockModuleInstance *myApplet);

void cd_sysmonitor_get_cpu_data (CairoDockModuleInstance *myApplet);


#endif

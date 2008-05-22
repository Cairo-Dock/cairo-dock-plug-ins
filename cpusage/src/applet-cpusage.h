
#ifndef __APPLET_cpusage__
#define  __APPLET_cpusage__

#include <cairo-dock.h>

void cd_cpusage_get_uptime (gchar **cUpTime, gchar **cActivityTime);

void cd_cpusage_get_cpu_info (void);

void cd_cpusage_read_data (void);
void cd_cpusage_update_from_data (void);

#endif

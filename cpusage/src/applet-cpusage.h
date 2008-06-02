
#ifndef __APPLET_cpusage__
#define  __APPLET_cpusage__

#include <cairo-dock.h>

void cd_cpusage_get_uptime (gchar **cUpTime, gchar **cActivityTime);

void cd_cpusage_get_cpu_info (void);

void cd_cpusage_read_data (void);
gboolean cd_cpusage_update_from_data (void);


void cd_cpusage_free_process (CDProcess *pProcess);

void cd_cpusage_get_process_times (double fTime, double fTimeElapsed);

void cd_cpusage_clean_old_processes (double fTime);

void cd_cpusage_clean_all_processes (void);


#endif

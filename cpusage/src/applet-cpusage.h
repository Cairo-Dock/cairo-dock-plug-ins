#ifndef __APPLET_cpusage__
#define  __APPLET_cpusage__

#include <cairo-dock.h>

/*gboolean cd_cpusage_timer(gpointer data);
gboolean cd_cpusage_getUsage(void);
void cd_cpusage_get_data (void);
void cd_cpusage_launch_analyse(void);*/


void cd_cpusage_read_data (void);
void cd_cpusage_update_from_data (void);

#endif

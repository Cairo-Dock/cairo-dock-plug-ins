#ifndef __APPLET_cpusage__
#define  __APPLET_cpusage__

#include <cairo-dock.h>
gboolean inDebug;
gboolean cd_cpusage_timer(gpointer data);
gboolean cd_cpusage_getUsage(void);
void cd_cpusage_get_data (void);
void cd_cpusage_launch_analyse(void);
#endif

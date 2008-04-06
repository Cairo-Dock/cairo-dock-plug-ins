#ifndef __APPLET_COMPIZ__
#define  __APPLET_COMPIZ _


#include <cairo-dock.h>

void _compiz_cmd(gchar *cmd);
void _reload_wm(void);
gboolean cd_compiz_start_wm(void);
void cd_compiz_launch_measure(void);
gpointer cd_compiz_threaded_calculation (gpointer data);
void cd_compiz_get_data(void);
gboolean cd_compiz_isRunning(void);
static void _compiz_get_values_from_file (gchar *cContent);
gboolean cd_compiz_timer(void);
void cd_compiz_check_my_wm(void);

#endif

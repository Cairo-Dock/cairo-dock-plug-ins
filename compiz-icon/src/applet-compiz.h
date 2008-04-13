#ifndef __APPLET_COMPIZ__
#define  __APPLET_COMPIZ _

#include <cairo-dock.h>


void cd_compiz_start_system_wm (void);

void cd_compiz_start_compiz (void);

void cd_compiz_switch_manager(void);

void cd_compiz_start_favorite_decorator (void);
void cd_compiz_start_decorator (compizDecorator iDecorator);


gboolean cd_compiz_timer (gpointer data);
void cd_compiz_launch_measure(void);
gpointer cd_compiz_threaded_calculation (gpointer data);
void cd_compiz_get_data(void);
gboolean cd_compiz_read_data(void);
static void _compiz_get_values_from_file (gchar *cContent);


void _reload_wm(void);
gboolean cd_compiz_start_wm(void);
void cd_compiz_check_my_wm(void);
void cd_compiz_kill_compmgr(void);


#endif

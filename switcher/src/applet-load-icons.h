
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__


#include <cairo-dock.h>

void load_icons (void);
gboolean cd_switcher_timer (gpointer data);
static gboolean _cd_switcher_check_for_redraw (gpointer data);
void cd_switcher_launch_measure (void);


#endif


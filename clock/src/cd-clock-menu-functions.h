
#ifndef __CD_CLOCK_MENU_FUNC__
#define  __CD_CLOCK_MENU_FUNC__


#include <cairo-dock.h>


void cd_clock_launch_time_admin (GtkMenuItem *menu_item, gpointer *data);

void cd_clock_about (GtkMenuItem *menu_item, gpointer *data);


gboolean cd_clock_notification_click_icon (gpointer *data);

gboolean cd_clock_notification_build_menu (gpointer *data);

#endif



#ifndef __LOGOUT_NOTIFICATIONS__
#define  __LOGOUT_NOTIFICATIONS__


#include <cairo-dock.h>


void cd_logout_about (GtkMenuItem *menu_item, gpointer *data);


gboolean cd_logout_notification_click_icon (gpointer *data);

gboolean cd_logout_notification_build_menu (gpointer *data);


#endif



#ifndef __CD_DUSTBIN_MENU_FUNC__
#define  __CD_DUSTBIN_MENU_FUNC__


#include <cairo-dock.h>


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory);


void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory);


void cd_dustbin_about (GtkMenuItem *menu_item, gpointer *data);


gboolean cd_dustbin_notification_click_icon (gpointer *data);


#endif

#ifndef __rhythmbox_MENU_FUNC__
#define  __rhythmbox_MENU_FUNC__

#include <cairo-dock.h>

void rhythmbox_previous (GtkMenuItem *menu_item, gpointer *data);
void rhythmbox_next (GtkMenuItem *menu_item, gpointer *data);
void rhythmbox_pause (GtkMenuItem *menu_item, gpointer *data);
void rhythmbox_play (GtkMenuItem *menu_item, gpointer *data);

void rhythmbox_about (GtkMenuItem *menu_item, gpointer *data);

gboolean rhythmbox_notification_build_menu (gpointer *data);

gboolean rhythmbox_action (gpointer *data);

#endif

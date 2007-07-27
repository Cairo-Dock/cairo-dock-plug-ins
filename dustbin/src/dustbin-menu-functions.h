
#ifndef __CD_DUSTBIN_MENU_FUNC__
#define  __CD_DUSTBIN_MENU_FUNC__


#include <cairo-dock.h>


void dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory);


void dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory);


void dustbin_about (GtkMenuItem *menu_item, gpointer *data);


#endif

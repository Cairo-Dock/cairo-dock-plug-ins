
#ifndef __CD_DUSTBIN_MENU_FUNC__
#define  __CD_DUSTBIN_MENU_FUNC__


#include <cairo-dock.h>


CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_DROP_DATA_H


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory);


void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory);


#endif

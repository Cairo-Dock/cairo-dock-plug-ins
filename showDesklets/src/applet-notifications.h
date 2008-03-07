
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__

#include <cairo-dock.h>


CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

gboolean cd_show_desklet_active_window_changed (Window *pXid);

void cd_show_desklet_on_keybinding_pull (const char *keystring, gpointer user_data);

#endif

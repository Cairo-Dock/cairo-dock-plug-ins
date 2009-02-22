
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_MIDDLE_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

void cd_menu_on_shortkey_menu (const char *keystring, gpointer data);

void cd_menu_on_shortkey_quick_launch (const char *keystring, gpointer data);


#endif

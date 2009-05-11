
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__

#include <cairo-dock.h>


CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

CD_APPLET_ON_SCROLL_H

gboolean on_change_active_window (CairoDockModuleInstance *myApplet, Window *XActiveWindow);

gboolean on_change_desktop (CairoDockModuleInstance *myApplet, gpointer null);

gboolean on_change_screen_geometry (CairoDockModuleInstance *myApplet, gpointer null);

gboolean on_window_configured (CairoDockModuleInstance *myApplet, XConfigureEvent *xconfigure);


#endif

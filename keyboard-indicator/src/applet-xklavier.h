
#ifndef __APPLET_XKLAVIER__
#define  __APPLET_XKLAVIER__


#include <cairo-dock.h>


void cd_xkbd_set_prev_next_group (int iDelta);


void cd_xkbd_set_group (int iNumGroup);


gboolean cd_xkbd_keyboard_state_changed (CairoDockModuleInstance *myApplet, Window *pWindow);


#endif

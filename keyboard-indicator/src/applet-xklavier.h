
#ifndef __APPLET_XKLAVIER__
#define  __APPLET_XKLAVIER__


#include <cairo-dock.h>


void cd_xkbd_set_prev_next_group (int iDelta);

void cd_xkbd_set_group (int iNumGroup);


void cd_xkbd_update_icon (const gchar *cGroupName, const gchar *cShortGroupName, const gchar *cIndicatorName);


gboolean cd_xkbd_keyboard_state_changed (CairoDockModuleInstance *myApplet, Window *pWindow);


gboolean cd_xkbd_render_step_opengl (CairoDockModuleInstance *myApplet);

gboolean cd_xkbd_render_step_cairo (CairoDockModuleInstance *myApplet);


#endif

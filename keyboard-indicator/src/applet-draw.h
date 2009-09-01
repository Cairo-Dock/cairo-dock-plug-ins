
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void cd_xkbd_update_icon (const gchar *cGroupName, const gchar *cShortGroupName, const gchar *cIndicatorName, gboolean bRedrawSurface);


gboolean cd_xkbd_render_step_opengl (CairoDockModuleInstance *myApplet);

gboolean cd_xkbd_render_step_cairo (CairoDockModuleInstance *myApplet);


#endif

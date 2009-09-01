#ifndef __APPLET_TRANSITIONS__
#define  __APPLET_TRANSITIONS__

#include <cairo-dock.h>

#include "applet-struct.h"


void cd_slider_draw_default (CairoDockModuleInstance *myApplet);

gboolean cd_slider_fade (CairoDockModuleInstance *myApplet);

gboolean cd_slider_blank_fade (CairoDockModuleInstance *myApplet);

gboolean cd_slider_fade_in_out (CairoDockModuleInstance *myApplet);

gboolean cd_slider_side_kick (CairoDockModuleInstance *myApplet);

gboolean cd_slider_diaporama (CairoDockModuleInstance *myApplet);

gboolean cd_slider_grow_up (CairoDockModuleInstance *myApplet);

gboolean cd_slider_shrink_down (CairoDockModuleInstance *myApplet);

gboolean cd_slider_cube (CairoDockModuleInstance *myApplet);


#endif

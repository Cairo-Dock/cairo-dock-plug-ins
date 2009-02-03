
#ifndef __APPLET_DECORATOR_TOOLTIP__
#define __APPLET_DECORATOR_TOOLTIP__

#include "cairo-dock.h"

#define MY_APPLET_DECORATOR_TOOLTIP_NAME "tooltip"


void cd_decorator_set_frame_size_tooltip (CairoDialog *pDialog);


void cd_decorator_draw_decorations_tooltip (cairo_t *pCairoContext, CairoDialog *pDialog);


void cd_decorator_register_tooltip (void);


#endif

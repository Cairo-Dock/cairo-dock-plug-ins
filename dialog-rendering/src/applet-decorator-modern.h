
#ifndef __APPLET_DECORATOR_MODERN__
#define __APPLET_DECORATOR_MODERN__

#include "cairo-dock.h"

#define MY_APPLET_DECORATOR_MODERN_NAME "modern"


void cd_decorator_set_frame_size_modern (CairoDialog *pDialog);


void cd_decorator_draw_decorations_modern (cairo_t *pCairoContext, CairoDialog *pDialog);


void cd_decorator_register_modern (void);


#endif

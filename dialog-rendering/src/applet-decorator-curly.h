
#ifndef __APPLET_DECORATOR_CURLY__
#define __APPLET_DECORATOR_CURLY__

#include "cairo-dock.h"

#define MY_APPLET_DECORATOR_CURLY_NAME "curly"


void cd_decorator_set_frame_size_curly (CairoDialog *pDialog);


void cd_decorator_draw_decorations_curly (cairo_t *pCairoContext, CairoDialog *pDialog);


void cd_decorator_register_curly (void);


#endif

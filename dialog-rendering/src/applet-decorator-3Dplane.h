
#ifndef __APPLET_DECORATOR_3DPLANE__
#define __APPLET_DECORATOR_3DPLANE__

#include "cairo-dock.h"

#define MY_APPLET_DECORATOR_3DPLANE_NAME "3Dplane"


void cd_decorator_set_frame_size_3Dplane (CairoDialog *pDialog);


void cd_decorator_draw_decorations_3Dplane (cairo_t *pCairoContext, CairoDialog *pDialog);


void cd_decorator_register_3Dplane (void);


#endif

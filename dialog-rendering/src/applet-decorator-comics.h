
#ifndef __APPLET_DECORATOR_COMICS__
#define __APPLET_DECORATOR_COMICS__

#include "cairo-dock.h"

#define MY_APPLET_DECORATOR_COMICS_NAME N_("comics")


void cd_decorator_set_frame_size_comics (CairoDialog *pDialog);


void cd_decorator_draw_decorations_comics (cairo_t *pCairoContext, CairoDialog *pDialog);


void cd_decorator_register_comics (void);


#endif

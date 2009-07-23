
#ifndef __APPLET_DRAW_
#define  __APPLET_DRAW_


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_render_cairo (CairoDock *pMainDock, cairo_t *pCairoContext);


void cd_do_render_opengl (CairoDock *pMainDock);


#endif

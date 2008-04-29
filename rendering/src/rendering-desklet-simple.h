
#ifndef __RENDERING_DESKLET_SIMPLE__
#define  __RENDERING_DESKLET_SIMPLE__

#include "cairo.h"

#define MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME "Simple"


void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_simple_desklet_renderer (void);


#endif

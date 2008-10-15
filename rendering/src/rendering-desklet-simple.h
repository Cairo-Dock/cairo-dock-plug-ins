
#ifndef __RENDERING_DESKLET_SIMPLE__
#define  __RENDERING_DESKLET_SIMPLE__

#include "cairo.h"

#define MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME "Simple"


typedef struct {
	gdouble fBackGroundAlpha;
	gdouble fForeGroundAlpha;
	gint iLeftSurfaceOffset;
	gint iTopSurfaceOffset;
	gint iRightSurfaceOffset;
	gint iBottomSurfaceOffset;
	cairo_surface_t *pBackGroundSurface;
	cairo_surface_t *pForeGroundSurface;
	gdouble fImageWidth;
	gdouble fImageHeight;
	} CDSimpleParameters;


CDSimpleParameters *rendering_configure_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_simple_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_simple_desklet_renderer (void);


void cd_rendering_register_desklet_decorations (void);

#endif

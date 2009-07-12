
#ifndef __RENDERING_DESKLET_SLIDE__
#define  __RENDERING_DESKLET_SLIDE__

#include "cairo-dock.h"

#define MY_APPLET_SLIDE_DESKLET_RENDERER_NAME "Slide"


typedef struct {
	// from config
	gboolean bRoundedRadius;
	gint iRadius;
	gdouble fLineColor[4];
	gint iLineWidth;
	gint iGapBetweenIcons;
	// computed data
	gdouble fMargin;
	gint iNbIcons;
	gint iIconSize;
	gint iNbLines, iNbColumns;
	} CDSlideParameters;

CDSlideParameters *rendering_configure_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_slide_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_slide_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_slide_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_slide_desklet_renderer (void);


#endif

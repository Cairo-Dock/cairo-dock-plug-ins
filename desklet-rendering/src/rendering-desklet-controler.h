
#ifndef __RENDERING_DESKLET_CONTROLER__
#define  __RENDERING_DESKLET_CONTROLER__

#include "cairo-dock.h"

#define MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME "Controler"


typedef struct {
	gboolean b3D;
	gboolean bCircular;
	gdouble fGapBetweenIcons;
	gint iEllipseHeight;
	gdouble fInclinationOnHorizon;
	gint iFrameHeight;
	gdouble fExtraWidth;
	gint iControlPanelHeight;
	Icon *pClickedIcon;
	} CDControlerParameters;


CDControlerParameters *rendering_configure_controler (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_controler_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_controler_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_controler (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_controler_desklet_renderer (void);


#endif

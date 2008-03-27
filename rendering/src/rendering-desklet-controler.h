
#ifndef __RENDERING_DESKLET_CONTROLER__
#define  __RENDERING_DESKLET_CONTROLER__

#include "cairo-dock.h"

#define MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME "Controler"


typedef struct {
	gboolean b3D;
	gboolean bCircular;
	gint iEllipseHeight;
	gdouble fInclinationOnHorizon;
	gint iFrameHeight;
	gdouble fExtraWidth;
	} CDControlerParameters;


CDControlerParameters *rendering_load_controler (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_controler_parameters (CDControlerParameters *pControler, gboolean bFree);

void rendering_load_icons_for_controler_desklet (CairoDockDesklet *pDesklet);


void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet);


#endif

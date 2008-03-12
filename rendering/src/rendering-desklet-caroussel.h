
#ifndef __RENDERING_DESKLET_CAROUSSEL__
#define  __RENDERING_DESKLET_CAROUSSEL__

#include "cairo-dock.h"

#define MY_APPLET_CAROUSSEL_DESKLET_RENDERER_NAME "Caroussel"


typedef struct {
	gboolean b3D;
	gboolean bRotateIconsOnEllipse;
	gdouble fDeltaTheta;
	gint iEllipseHeight;
	gdouble fInclinationOnHorizon;
	gint iFrameHeight;
	gdouble fExtraWidth;
	gdouble a;
	gdouble b;
	gdouble fRotationAngle;
	guint iSidRotation;
	gint iRotationDirection;
	gint iRotationCount;
	} CDCarousselParameters;


CDCarousselParameters *rendering_load_caroussel (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gboolean bRotateIconsOnEllipse, gboolean b3D);

void rendering_free_caroussel_parameters (CDCarousselParameters *pCaroussel, gboolean bFree);

void rendering_load_icons_for_caroussel_desklet (CairoDockDesklet *pDesklet);


void rendering_draw_caroussel_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet);


#endif

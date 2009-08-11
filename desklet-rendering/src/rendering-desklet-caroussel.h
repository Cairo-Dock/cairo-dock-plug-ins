
#ifndef __RENDERING_DESKLET_CAROUSSEL__
#define  __RENDERING_DESKLET_CAROUSSEL__

#include "cairo-dock.h"


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
	gdouble fCurrentRotationSpeed;
	} CDCarousselParameters;

typedef struct {
	Icon *pIcon;
	double fTheta;
	} _CarousselPositionedIcon;

CDCarousselParameters *rendering_configure_caroussel (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_caroussel_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_caroussel_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_caroussel (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_caroussel_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_caroussel_desklet_renderer (void);


#endif

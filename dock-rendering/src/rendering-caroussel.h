
#ifndef __RENDERING_CAROUSSEL__
#define  __RENDERING_CAROUSSEL__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_caroussel (CairoDock *pDock);


void cd_rendering_calculate_construction_parameters_caroussel (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxIconHeight, int iMaxIconWidth, int iEllipseHeight, gboolean bDirectionUp, double fExtraWidth, double fLinearWidth, double fXFirstIcon);

void cd_rendering_render_icons_caroussel (cairo_t *pCairoContext, CairoDock *pDock);

void cd_rendering_render_caroussel (cairo_t *pCairoContext, CairoDock *pDock);

Icon *cd_rendering_calculate_icons_caroussel (CairoDock *pDock);


void cd_rendering_register_caroussel_renderer (const gchar *cRendererName);


#endif

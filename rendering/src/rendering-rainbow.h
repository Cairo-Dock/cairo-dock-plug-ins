
#ifndef __RENDERING_ARC_EN_CIEL_VIEW__
#define  __RENDERING_ARC_EN_CIEL_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_RAINBOW_VIEW_NAME "Rainbow"


void cd_rendering_calculate_max_dock_size_rainbow (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_rainbow (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY);

cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight);


void cd_rendering_render_rainbow (cairo_t *pCairoContext, CairoDock *pDock);


Icon *cd_rendering_calculate_icons_rainbow (CairoDock *pDock);


void cd_rendering_register_rainbow_renderer (void);


#endif

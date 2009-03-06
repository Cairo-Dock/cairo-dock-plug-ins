
#ifndef __RENDERING_PARABOLE__
#define  __RENDERING_PARABOLE__

#include "cairo-dock.h"


void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pParentDock);


void cd_rendering_calculate_reference_parabole (double alpha);


void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock);


void cd_rendering_calculate_construction_parameters_parabole (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iFlatDockWidth, gboolean bDirectionUp, double fAlign, gboolean bHorizontalDock);

void cd_rendering_render_icons_parabole (cairo_t *pCairoContext, CairoDock *pDock, double fRatio);

void cd_rendering_render_parabole (cairo_t *pCairoContext, CairoDock *pDock);

Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock);


void cd_rendering_register_parabole_renderer (const gchar *cRendererName);

void cd_rendering_render_parabole_opengl (CairoDock *pDock);


#endif

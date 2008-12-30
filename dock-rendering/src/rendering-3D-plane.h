
#ifndef __RENDERING_3D_PLANE2_VIEW__
#define  __RENDERING_3D_PLANE2_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY);


void cd_rendering_render_3D_plane (cairo_t *pCairoContext, CairoDock *pDock);


void cd_rendering_render_optimized_3D_plane (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock);


void cd_rendering_register_3D_plane_renderer (const gchar *cRendererName);


void cd_rendering_draw_flat_separator_opengl (Icon *icon, CairoDock *pDock);


#endif

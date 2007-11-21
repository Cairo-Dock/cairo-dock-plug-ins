
#ifndef __RENDERING_3D_PLANE_VIEW__
#define  __RENDERING_3D_PLANE_VIEW__

#include "cairo-dock.h"

#define CD_RENDERING_3D_PLANE_VIEW_NAME "3D plane"


void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY);

void cd_rendering_render_3D_plane (CairoDock *pDock);


void cd_rendering_render_optimized_3D_plane (CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock);


void cd_rendering_register_3D_plane_renderer (void);


#endif

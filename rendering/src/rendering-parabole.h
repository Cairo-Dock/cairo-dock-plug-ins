
#ifndef __RENDERING_PARABOLE__
#define  __RENDERING_PARABOLE__

#include "cairo-dock.h"

#define CD_RENDERING_PARABOLIC_VIEW_NAME "Parabolic"


void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pParentDock);


void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock);


void cd_rendering_calculate_construction_parameters_parabole (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iFlatDockWidth, gboolean bDirectionUp);

void cd_rendering_render_icons_parabole (cairo_t *pCairoContext, CairoDock *pDock, double fRatio);

void cd_rendering_render_parabole (CairoDock *pDock);

CairoDockMousePositionType cd_rendering_check_if_mouse_inside_parabole (CairoDock *pDock);

Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock);


void cd_rendering_register_parabole_renderer (void);


#endif

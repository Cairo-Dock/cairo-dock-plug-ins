
#ifndef __RENDERING_DIAPO_SIMPLE_VIEW__
#define  __RENDERING_DIAPO_SIMPLE_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_DIAPO_SIMPLE_VIEW_NAME "SimpleSlide"


void cd_rendering_calculate_max_dock_size_diapo_simple (CairoDock *pDock);


void cd_rendering_render_diapo_simple (CairoDock *pDock);


void cd_rendering_render_diapo_simple_optimized (CairoDock *pDock);


Icon *cd_rendering_calculate_icons_diapo_simple (CairoDock *pDock);


void cd_rendering_register_diapo_simple_renderer (void);


guint cairo_dock_rendering_diapo_simple_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY);


Icon *  cairo_dock_calculate_icons_position_for_diapo_simple(CairoDock* pDock, guint nRowsX, guint nRowsY, gint Mx, gint My);


void cairo_dock_calculate_wave_with_position_diapo_simple(GList *pIconList, gint Mx, gint My, guint nRowsX); 


void cairo_dock_rendering_diapo_simple_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY);


guint cairo_dock_rendering_diapo_simple_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY);



#endif

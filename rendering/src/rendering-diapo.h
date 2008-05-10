
#ifndef __RENDERING_DIAPO_VIEW__
#define  __RENDERING_DIAPOL_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_DIAPO_VIEW_NAME "Diapositive"


void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock);


void cd_rendering_render_diapo (CairoDock *pDock);



Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock);


void cd_rendering_register_diapo_renderer (void);

void cairo_dock_calculate_icons_position_for_diapo(CairoDock* pDock, guint nRowsX, guint nRowsY);

guint cairo_dock_rendering_diapo_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY);


Icon * cairo_dock_calculate_wave_with_position_diapo(GList *pIconList, gint Mx, gint My, guint nRowsX); 

void cairo_dock_calculate_icons_positions_at_rest_diapo (GList *pIconList, gint* Wmin, gint* Hmin, guint nRowsX);

void cairo_dock_rendering_diapo_calculate_max_icon_size(GList *pIconList, gint* maxWidth, gint* maxHeight, guint nRowsX,  guint nRowsY);

void cairo_dock_rendering_diapo_calculate_max_dock_size (GList *pIconList, gint Wmin, gint Hmin, gint* Wmax, gint* Hmax, guint nRowsX, guint nRowsY);

void cairo_dock_rendering_diapo_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY);

guint cairo_dock_rendering_diapo_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY);

#endif

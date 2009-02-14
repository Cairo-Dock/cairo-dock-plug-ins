
#ifndef __RENDERING_DIAPO_VIEW__
#define  __RENDERING_DIAPO_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock);


void cd_rendering_render_diapo (cairo_t *pCairoContext, CairoDock *pDock);



Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock);


void cd_rendering_register_diapo_renderer (const gchar *cRendererName);

Icon *  cairo_dock_calculate_icons_position_for_diapo(CairoDock* pDock, guint nRowsX, guint nRowsY, gint Mx, gint My);

guint cairo_dock_rendering_diapo_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY);


void cairo_dock_calculate_wave_with_position_diapo(GList *pIconList, gint Mx, gint My, guint nRowsX); 

void cairo_dock_calculate_icons_positions_at_rest_diapo (GList *pIconList, gint* Wmin, gint* Hmin, guint nRowsX);

void cairo_dock_rendering_diapo_calculate_max_icon_size(GList *pIconList, gint* maxWidth, gint* maxHeight, guint nRowsX,  guint nRowsY);

void cairo_dock_rendering_diapo_calculate_max_dock_size (GList *pIconList, gint Wmin, gint Hmin, gint* Wmax, gint* Hmax, guint nRowsX, guint nRowsY);

void cairo_dock_rendering_diapo_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY);

guint cairo_dock_rendering_diapo_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY);



static void cairo_dock_draw_frame_horizontal_for_diapo (cairo_t *pCairoContext, CairoDock *pDock);

static void cairo_dock_draw_frame_vertical_for_diapo (cairo_t *pCairoContext, CairoDock *pDock);

void cairo_dock_draw_frame_for_diapo (cairo_t *pCairoContext, CairoDock *pDock);

void cairo_dock_render_decorations_in_frame_for_diapo (cairo_t *pCairoContext, CairoDock *pDock);



#endif

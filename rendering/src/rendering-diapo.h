
#ifndef __RENDERING_DIAPO_VIEW__
#define  __RENDERING_DIAPOL_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_DIAPO_VIEW_NAME "Diapositive"


void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock);


void cd_rendering_render_diapo (CairoDock *pDock);



Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock);


void cd_rendering_register_diapo_renderer (void);

Icon * cairo_dock_calculate_wave_with_position_diapo (GList *pIconList, GList *pFirstDrawnElementGiven, int x_abs, gdouble fMagnitude, double fFlatDockWidth, int iWidth, int iHeight, double fAlign, double fFoldingFactor, gboolean bDirectionUp);

GList *cairo_dock_calculate_icons_positions_at_rest_diapo (GList *pIconList, double fFlatDockWidth, int iXOffset);

guint cairo_dock_rendering_diapo_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY);

#endif

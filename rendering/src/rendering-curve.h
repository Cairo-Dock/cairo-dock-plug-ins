
#ifndef __RENDERING_CURVE_VIEW__
#define  __RENDERING_CURVE_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_curve (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_curve (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY, double yCurve);


void cd_rendering_render_curve (cairo_t *pCairoContext, CairoDock *pDock);


void cd_rendering_render_optimized_curve (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_curve (CairoDock *pDock);


void cd_rendering_register_curve_renderer (const gchar *cRendererName);


void cairo_dock_draw_curved_frame (cairo_t *pCairoContext, double fFrameWidth, double fControlHeight, double fDockOffsetX, double fDockOffsetY, gboolean bHorizontal, int sens);


void cd_rendering_calculate_reference_curve (double alpha);

double cd_rendering_interpol_curve_parameter (double x);

double cd_rendering_interpol_curve_height (double x);


#endif

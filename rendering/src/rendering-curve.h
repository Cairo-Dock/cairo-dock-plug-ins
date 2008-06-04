
#ifndef __RENDERING_CURVE_VIEW__
#define  __RENDERING_CURVE_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_CURVE_VIEW_NAME "Curve"


void cd_rendering_calculate_max_dock_size_curve (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_curve (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY, double yCurve);


void cd_rendering_render_curve (cairo_t *pCairoContext, CairoDock *pDock);


void cd_rendering_render_optimized_curve (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_curve (CairoDock *pDock);


void cd_rendering_register_curve_renderer (void);

void cairo_dock_draw_curved_frame (cairo_t *pCairoContext, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, gboolean bHorizontal, int sens);

void cairo_dock_draw_curved_frame_vertical (cairo_t *pCairoContext, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens);

void cairo_dock_draw_curved_frame_horizontal (cairo_t *pCairoContext, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens);

#endif

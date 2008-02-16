
#ifndef __CD_CLOCK_DRAW__
#define  __CD_CLOCK_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_free_alarm (CDClockAlarm *pAlarm);


void cd_clock_draw_in_desklet (cairo_t *pCairoContext, gpointer data);

gboolean cd_clock_update_with_time (Icon *icon);


void cd_clock_draw_text (cairo_t *pCairoContext, int width, int height, double fMaxScale, struct tm *pTime);



void draw_background (cairo_t* pDrawingContext, int iWidth, int iHeight);

void draw_foreground (cairo_t* pDrawingContext, int iWidth, int iHeight);

cairo_surface_t* update_surface (cairo_surface_t* pOldSurface,
								 cairo_t* pSourceContext,
								 int iWidth,
								 int iHeight,
								 SurfaceKind kind);

void cd_clock_draw_old_fashionned_clock (cairo_t *pCairoContext, int width, int height, double fMaxScale, struct tm *pTime);



#endif



#ifndef __CD_CLOCK_DRAW__
#define  __CD_CLOCK_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_free_alarm (CDClockAlarm *pAlarm);


//void cd_clock_draw_in_desklet (cairo_t *pCairoContext, gpointer data);

gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet);


void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


cairo_surface_t* update_surface (CairoDockModuleInstance *myApplet, 
						cairo_surface_t* pOldSurface,
						cairo_t* pSourceContext,
						int iWidth,
						int iHeight,
						SurfaceKind kind);

void cd_clock_draw_analogic (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);



#endif



#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void switcher_draw_main_dock_icon (cairo_surface_t *pSurface);
gboolean _cd_switcher_check_for_redraw_cairo (gpointer data);
void switcher_draw_subdock_icon (cairo_surface_t *pSurface);


#endif


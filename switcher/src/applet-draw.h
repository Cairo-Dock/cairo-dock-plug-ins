
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer);
gboolean switcher_draw_main_dock_icon (gpointer data);
gboolean _cd_switcher_check_for_redraw_cairo (gpointer data);


#endif


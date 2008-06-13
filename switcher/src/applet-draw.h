
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer);
gboolean switcher_draw_main_dock_icon (void);


void cd_switcher_draw_main_icon_compact_mode (void);

void cd_switcher_draw_main_icon_expanded_mode (void);

void cd_switcher_draw_main_icon (void);


#endif



#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer);
gboolean switcher_draw_main_dock_icon (void);
//void cd_switcher_draw_windows_on_each_viewports();
void cd_switcher_draw_windows_on_each_viewports(double Xposition, double Yposition, double Xsize, double Ysize);
void cd_switcher_draw_main_icon_compact_mode (void);

void cd_switcher_draw_main_icon_expanded_mode (void);

void cd_switcher_draw_main_icon (void);


void cd_switcher_draw_desktops_bounding_box (CairoDesklet *pDesklet);
void cd_switcher_extract_viewport_coords_from_picked_object (CairoDesklet *pDesklet, int *iCoordX, int *iCoordY);


#endif



#ifndef __APPLET_ANIMATION__
#define  __APPLET_ANIMATION__

#include <cairo-dock.h>


#define penguin_get_current_animation() (myData.iCurrentAnimation >= 0 ? &myData.pAnimations[myData.iCurrentAnimation] : NULL);

#define penguin_is_resting(pAnimation) ((pAnimation) == NULL || (pAnimation)->iNbFrames <= 1 && (pAnimation)->iSpeed == 0)

#define penguin_remove_notfications() do {\
	cairo_dock_remove_notification_func_on_icon (myIcon, CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) penguin_update_icon, myApplet);\
	cairo_dock_remove_notification_func_on_container (CAIRO_CONTAINER (g_pMainDock), CAIRO_DOCK_UPDATE_DOCK_SLOW, (CairoDockNotificationFunc) penguin_update_container, myApplet);\
	cairo_dock_remove_notification_func_on_container (CAIRO_CONTAINER (g_pMainDock), CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) penguin_render_on_container, myApplet); } while (0)

void penguin_move_in_dock (CairoDockModuleInstance *myApplet);
gboolean penguin_render_on_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext);
void penguin_draw_on_dock_opengl (CairoDockModuleInstance *myApplet, CairoContainer *pContainer);
void penguin_draw_on_dock (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext);


void penguin_move_in_icon (CairoDockModuleInstance *myApplet);


gboolean penguin_update_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation);
gboolean penguin_update_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation);






void penguin_calculate_new_position (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation, int iXMin, int iXMax, int iHeight);

void penguin_advance_to_next_frame (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation);

int penguin_choose_movement_animation (CairoDockModuleInstance *myApplet);
int penguin_choose_go_up_animation (CairoDockModuleInstance *myApplet);
int penguin_choose_beginning_animation (CairoDockModuleInstance *myApplet);
int penguin_choose_ending_animation (CairoDockModuleInstance *myApplet);
int penguin_choose_resting_animation (CairoDockModuleInstance *myApplet);
int penguin_choose_next_animation (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation);

void penguin_set_new_animation (CairoDockModuleInstance *myApplet, int iNewAnimation);

void penguin_start_animating (CairoDockModuleInstance *myApplet);
void penguin_start_animating_with_delay (CairoDockModuleInstance *myApplet);


#endif


#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


gboolean cd_icon_effect_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation);
gboolean cd_icon_effect_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState);


gboolean cd_icon_effect_pre_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock);


gboolean cd_icon_effect_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext);


gboolean cd_icon_effect_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation);


gboolean cd_icon_effect_free_data (gpointer pUserData, Icon *pIcon);


#endif

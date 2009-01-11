
#ifndef __APPLET_NOTIFICATIONS__
#define __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


gboolean cd_illusion_on_remove_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock);

gboolean cd_illusion_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext);


gboolean cd_illusion_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation);


gboolean cd_illusion_free_data (gpointer pUserData, Icon *pIcon);


#endif


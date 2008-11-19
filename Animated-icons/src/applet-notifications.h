
#ifndef __APPLET_NOTIFICATIONS__
#define __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


gboolean cd_animations_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation);


gboolean cd_animations_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered);
gboolean cd_animations_post_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered);


gboolean cd_animations_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation);


gboolean cd_animations_free_data (gpointer pUserData, Icon *pIcon);


#endif

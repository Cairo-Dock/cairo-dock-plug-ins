
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


gboolean cd_motion_blur_pre_render (gpointer pUserData, CairoDock *pDock);

gboolean cd_motion_blur_post_render (gpointer pUserData, CairoDock *pDock);

gboolean cd_motion_blur_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation);

gboolean cd_motion_blur_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation);

#endif

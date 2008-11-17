
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


gboolean cd_drop_indicator_render (gpointer pUserData, CairoDock *pDock);


gboolean cd_drop_indicator_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation);


gboolean cd_drop_indicator_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation);


void cd_drop_indicator_load_drop_indicator (gchar *cImagePath, cairo_t* pSourceContext, int iWidth, int iHeight);


#endif


#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>

void cd_xmms_draw_in_dock (gchar *cQuickInfo);
void cd_xmms_draw_in_desklet (cairo_t *pCairoContext, gchar *cQuickInfo);
Icon *cd_xmms_create_icon_for_desklet (cairo_t *pSourceContext, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName);

#endif

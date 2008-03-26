
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>

#include <applet-struct.h>

void cd_xmms_draw_icon (void);

void cd_xmms_draw_in_desklet (cairo_t *pCairoContext, gchar *cQuickInfo);
Icon *cd_xmms_create_icon_for_desklet (cairo_t *pSourceContext, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName);

void cd_xmms_animate_icon(int animationLength);
void cd_xmms_new_song_playing(void);

void cd_xmms_set_surface (MyPlayerStatus iStatus);

#endif

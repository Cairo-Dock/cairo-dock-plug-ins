
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>

#include "applet-struct.h"


gboolean cd_musicplayer_draw_icon (gpointer data);


void cd_musicplayer_popup_info (void);
void cd_musicplayer_animate_icon (int animationLength);

void cd_musicplayer_set_surface (MyPlayerStatus iStatus);

gboolean cd_musicplayer_check_size_is_constant (const gchar *cFilePath);

gboolean cd_musiplayer_set_cover_if_present (gboolean bCheckSize);


void cd_musicplayer_update_icon (gboolean bFirstTime);


#endif

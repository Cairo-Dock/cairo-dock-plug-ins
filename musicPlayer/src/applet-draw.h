
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>

#include <applet-struct.h>

gboolean cd_musicplayer_draw_icon (void);

void cd_musicplayer_add_buttons_to_desklet(void);

void cd_musicplayer_animate_icon(int animationLength);
void cd_musicplayer_new_song_playing(void);

void cd_musicplayer_set_surface (MyPlayerStatus iStatus);

void cd_musicplayer_change_desklet_data (void);
void cd_musicplayer_player_none (void);

#endif

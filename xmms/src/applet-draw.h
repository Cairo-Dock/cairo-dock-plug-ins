
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>

#include <applet-struct.h>

gboolean cd_xmms_draw_icon (CairoDockModuleInstance *myApplet);

void cd_xmms_add_buttons_to_desklet(CairoDockModuleInstance *myApplet);

void cd_xmms_animate_icon(CairoDockModuleInstance *myApplet, int animationLength);
void cd_xmms_new_song_playing(CairoDockModuleInstance *myApplet);

void cd_xmms_set_surface (CairoDockModuleInstance *myApplet, MyPlayerStatus iStatus);

void cd_xmms_change_desklet_data (CairoDockModuleInstance *myApplet);
void cd_xmms_player_none (CairoDockModuleInstance *myApplet);

#endif

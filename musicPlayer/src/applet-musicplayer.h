
#ifndef __APPLET_MUSICPLAYER__
#define  __APPLET_MUSICPLAYER__

#include <cairo-dock.h>

#include <applet-struct.h>

MusicPlayerHandeler *cd_musicplayer_get_handeler_by_name (const gchar *cName);
void cd_musicplayer_get_data (void);
void cd_musicplayer_arm_handeler (void);
void cd_musicplayer_disarm_handeler (void);
void cd_musicplayer_register_my_handeler (MusicPlayerHandeler *pHandeler, const gchar *cName);
void cd_musicplayer_free_handeler (MusicPlayerHandeler *pHandeler);

#endif

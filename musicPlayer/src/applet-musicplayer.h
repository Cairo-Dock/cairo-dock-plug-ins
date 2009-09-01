
#ifndef __APPLET_MUSICPLAYER__
#define  __APPLET_MUSICPLAYER__

#include <cairo-dock.h>

#include <applet-struct.h>


MusicPlayerHandeler *cd_musicplayer_get_handler_by_name (const gchar *cName);

void cd_musicplayer_launch_handler (void);

void cd_musicplayer_relaunch_handler (void);

void cd_musicplayer_stop_handler (void);

void cd_musicplayer_register_my_handler (MusicPlayerHandeler *pHandeler, const gchar *cName);

void cd_musicplayer_free_handler (MusicPlayerHandeler *pHandeler);


#endif

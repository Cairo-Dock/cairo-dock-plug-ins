#ifndef __APPLET_BANSHEE__
#define  __APPLET_BANSHEE__


#include <cairo-dock.h>

void cd_banshee_getSongInfos (void);

void cd_banshee_free_data (void);

void cd_banshee_control (MyPlayerControl pControl, char*);

void cd_banshee_load_dbus_commands (void);

gboolean cd_banshee_ask_control (MyPlayerControl pControl);

void cd_banshee_acquisition (void);

void cd_banshee_read_data (void);

void cd_musicplayer_register_banshee_handeler (void);

#endif


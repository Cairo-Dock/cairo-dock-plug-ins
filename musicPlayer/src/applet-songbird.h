

#ifndef __APPLET_SONGBIRD__
#define  __APPLET_SONGBIRD__


#include <cairo-dock.h>

void cd_songbird_free_data (void);

void cd_songbird_control (MyPlayerControl pControl, char*);

void cd_songbird_load_dbus_commands (void);

gboolean cd_songbird_ask_control (MyPlayerControl pControl);

void cd_songbird_acquisition (void);

void cd_songbird_read_data (void);

void cd_musicplayer_register_songbird_handeler (void);

#endif


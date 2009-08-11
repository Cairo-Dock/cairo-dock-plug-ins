#ifndef __APPLET_QUODLIBET__
#define  __APPLET_QUODLIBET__

#include <cairo-dock.h>

void cd_quodlibet_getSongInfos(void);

void cd_quodlibet_free_data (void);

void cd_quodlibet_control (MyPlayerControl pControl, char*);

void cd_quodlibet_load_dbus_commands (void);

gboolean cd_quodlibet_ask_control (MyPlayerControl pControl);

void cd_quodlibet_acquisition (void);

void cd_quodlibet_read_data (void);

void cd_musicplayer_register_quodlibet_handler (void);

#endif


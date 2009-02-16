#ifndef __APPLET_LISTEN__
#define  __APPLET_LISTEN__


#include <cairo-dock.h>

void cd_listen_free_data (void);

void cd_listen_control (MyPlayerControl pControl, char*);

void cd_listen_load_dbus_commands (void);

gboolean cd_listen_ask_control (MyPlayerControl pControl);

void cd_listen_acquisition (void);

void cd_listen_read_data (void);

void cd_musicplayer_register_listen_handeler (void);

#endif


#ifndef __APPLET_QUOD_LIBET__
#define  __APPLET_QUOD_LIBET__


#include <cairo-dock.h>

void cd_quod_libet_free_data (void);

void cd_quod_libet_control (MyPlayerControl pControl, char*);

void cd_quod_libet_load_dbus_commands (void);

gboolean cd_quod_libet_ask_control (MyPlayerControl pControl);

void cd_quod_libet_acquisition (void);

void cd_quod_libet_read_data (void);

void cd_musicplayer_register_quod_libet_handeler (void);

#endif


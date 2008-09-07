#ifndef __APPLET_EXAILE__
#define  __APPLET_EXAILE__


#include <cairo-dock.h>

void cd_exaile_free_data (void);

void cd_exaile_control (MyPlayerControl pControl, char*);

void cd_exaile_load_dbus_commands (void);

gboolean cd_exaile_ask_control (MyPlayerControl pControl);

void cd_exaile_acquisition (void);

void cd_exaile_read_data (void);

void cd_musicplayer_register_exaile_handeler (void);

#endif

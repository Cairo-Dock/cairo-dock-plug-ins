
#ifndef __APPLET_AMAROK_1__
#define  __APPLET_AMAROK_1__


#include <cairo-dock.h>

void cd_amarok1_free_data (void);

void cd_amarok1_control (MyPlayerControl pControl, gchar *cFile);

gboolean cd_amarok1_ask_control (MyPlayerControl pControl);

void cd_amarok1_acquisition (void);

void cd_amarok1_read_data (void);

void cd_musicplayer_register_amarok1_handeler (void);

#endif


#ifndef __APPLET_INFOPIPE__
#define  __APPLET_INFOPIPE__


#include <cairo-dock.h>

void cd_xmms_free_data (void);

void cd_xmms_control (MyPlayerControl pControl, gchar *cFile);

gboolean cd_xmms_ask_control (MyPlayerControl pControl);

void cd_xmms_acquisition (void);

void cd_xmms_read_data (void);

void cd_musicplayer_register_xmms_handeler (void);

#endif

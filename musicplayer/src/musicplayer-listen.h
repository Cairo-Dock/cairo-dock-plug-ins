#define  __MUSICPLAYER_LISTEN__

#include <cairo-dock.h>
#include <dbus/dbus-glib.h>

void musicplayer_getSongInfos_for_listen (void);
void musicplayer_getStatus_for_listen (void);
gboolean listen_get_data (void);

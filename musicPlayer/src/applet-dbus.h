
#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>

#include <applet-struct.h>



gboolean cd_musicplayer_dbus_connect_to_bus(void);
gboolean musicplayer_dbus_connect_to_bus_Shell (void);
void musicplayer_dbus_disconnect_from_bus (void);
void musicplayer_dbus_disconnect_from_bus_Shell (void);
void cd_musicplayer_dbus_detect_player (void);


void cd_musicplayer_getStatus_string(const char*, const char*, const char*);
void cd_musicplayer_getStatus_integer (int, int);

void cd_musicplayer_getCoverPath (void);


MusicPlayerHandeler *cd_musicplayer_dbus_find_opened_player (void);


#endif

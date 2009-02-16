
#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>

#include <applet-struct.h>



gboolean cd_musicplayer_dbus_connect_to_bus(void);
gboolean musicplayer_dbus_connect_to_bus_Shell (void);
void musicplayer_dbus_disconnect_from_bus (void);
void musicplayer_dbus_disconnect_from_bus_Shell (void);
gboolean cd_musicplayer_dbus_detection(void);
void cd_musicplayer_dbus_command(const char*);
gchar* musicplayer_dbus_getValue (const char *);
gchar* cd_musicplayer_getString_player (const char*);
void cd_musicplayer_check_dbus_connection (void);
//void cd_musicplayer_full_dbus_connection_with_shell (void);

void cd_musicplayer_getStatus_boolean(void);
void cd_musicplayer_getStatus_string(void);
void cd_musicplayer_getStatus_integer (void);
void cd_musicplayer_getSongInfos(void);
GHashTable* cd_musicplayer_getSongInfos_hashtable (gchar*, gchar*, GHashTable*);
void cd_musicplayer_getCoverPath (void);

guchar* cd_musicplayer_getCurPos_string (void);
int cd_musicplayer_getCurPos_integer (void);
gchar* cd_musicplayer_getLength_string (void);
int cd_musicplayer_getLength_integer (void);






#endif


#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>

#include <applet-struct.h>

static DBusGProxy *dbus_proxy_player = NULL;
static DBusGProxy *dbus_proxy_shell = NULL;

gboolean cd_musicplayer_dbus_connect_to_bus(void);
gboolean musicplayer_dbus_connect_to_bus_Shell (void);
void musicplayer_dbus_disconnect_from_bus (void);
void musicplayer_dbus_disconnect_from_bus_Shell (void);
void cd_musicplayer_dbus_detection(void);

void cd_musicplayer_dbus_command(const char*);
gchar* musicplayer_dbus_getValue (const char *);

void cd_musicplayer_getStatus_boolean(void);
void cd_musicplayer_getStatus_string(void);
void cd_musicplayer_getStatus_integer (void);

void cd_musicplayer_getSongInfos(void);

void cd_musicplayer_getCoverPath (void);
gboolean cd_musicplayer_check_for_changes (void);
void cd_musicplayer_load_dbus_commands (void);

void cd_musicplayer_check_dbus_connection (void);
#endif

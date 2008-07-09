#ifndef __MUSICPLAYER_DBUS__
#define  __MUSICPLAYER_DBUS__

#include <cairo-dock.h>
#include <dbus/dbus-glib.h>

#include "musicplayer-banshee.h"
#include "musicplayer-exaile.h"
#include "musicplayer-listen.h"
#include "musicplayer-rhythmbox.h"

static DBusGProxy *dbus_proxy_player = NULL;
static DBusGProxy *dbus_proxy_shell = NULL;

gboolean musicplayer_dbus_connect_to_bus(void);
gboolean musicplayer_dbus_connect_to_bus_Shell (void);
void musicplayer_dbus_disconnect_from_bus (void);
void musicplayer_dbus_disconnect_from_bus_Shell (void);
void dbus_detect_musicplayer(void);

void musicplayer_dbus_command(const char*);
gchar* musicplayer_dbus_getValue (const char *);

void musicplayer_getStatus_boolean(void);
void musicplayer_getStatus_string(void);
void musicplayer_getStatus_integer (void);

void musicplayer_getSongInfos(void);

void musicplayer_getCoverPath (void);
void musicplayer_check_for_changes (void);
void load_dbus_command_for_player (void);

gboolean check_dbus_connection (void);

#endif

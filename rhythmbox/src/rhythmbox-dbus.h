#ifndef __RHYTHMBOX_DBUS__
#define  __RHYTHMBOX_DBUS__

#include <dbus/dbus-glib.h>

gboolean rhythmbox_dbus_init(void);

int rhythmbox_getPlaying(void);
gchar *rhythmbox_getPlayingUri(void);

int rhythmbox_getElapsed(void);

gchar *rhythmbox_getSongName(const gchar *uri);

void rhythmbox_onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);
void rhythmbox_onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);


#endif

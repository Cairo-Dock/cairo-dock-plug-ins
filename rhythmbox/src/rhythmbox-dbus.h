#ifndef __RHYTHMBOX_DBUS__
#define  __RHYTHMBOX_DBUS__

#include <dbus/dbus-glib.h>

gboolean rhythmbox_dbus_init(void);
void dbus_detect_rhythmbox(void);

void rhythmbox_getPlaying(void);
void rhythmbox_getPlayingUri(void);
void rhythmbox_getElapsed(void);
void getSongInfos(void);
void onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);
void onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data);

#endif

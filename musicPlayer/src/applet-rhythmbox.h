#ifndef __APPLET_RHYTHMBOX__
#define  __APPLET_RHYTHMBOX__


#include <cairo-dock.h>

void cd_rhythmbox_free_data (void);

void cd_rhythmbox_control (MyPlayerControl pControl, char*);

void cd_rhythmbox_load_dbus_commands (void);

gboolean cd_rhythmbox_ask_control (MyPlayerControl pControl);

void cd_rhythmbox_acquisition (void);

void cd_rhythmbox_read_data (void);

void cd_musicplayer_register_rhythmbox_handeler (void);

void cd_musicplayer_rhythmbox_proxy_connection (void);

void onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);
void onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data);
void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data);

#endif


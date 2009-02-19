#ifndef __APPLET_amarok2__
#define  __APPLET_amarok2__


#include <cairo-dock.h>

void cd_amarok2_getSongInfos (void);

void cd_amarok2_getStatus (void);

void cd_amarok2_proxy_connection (void);

void cd_amarok2_free_data (void);

void cd_amarok2_control (MyPlayerControl pControl, char*);

void cd_amarok2_load_dbus_commands (void);

gboolean cd_amarok2_ask_control (MyPlayerControl pControl);

void cd_amarok2_acquisition (void);

void cd_amarok2_read_data (void);

void cd_musicplayer_register_amarok2_handeler (void);

void onChangeTrack(DBusGProxy *player_proxy,GHashTable *data_list, gpointer data);

void onChangeStatus(DBusGProxy *player_proxy, GValueArray *status, gpointer data);

/*void onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);

void onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);

void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data);

void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data);*/

#endif



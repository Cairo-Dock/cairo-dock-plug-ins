#ifndef __APPLET_MPRIS__
#define  __APPLET_MPRIS__

#include <cairo-dock.h>
#include <applet-struct.h>


void cd_mpris_getPlaying (void);

gboolean cd_mpris_is_loop (void);

gboolean cd_mpris_is_shuffle (void);

void cd_mpris_get_time_elapsed (void);

void cd_mpris_get_track_index (void);

void cd_mpris_getSongInfos (void);


void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data);

void onChangePlaying_mpris (DBusGProxy *player_proxy, GValueArray *status, gpointer data);

void onChangeTrackList (DBusGProxy *player_proxy, gint iNewTrackListLength, gpointer data);


gboolean cd_mpris_dbus_connect_to_bus (void);

void cd_mpris_free_data (void);

void cd_mpris_control (MyPlayerControl pControl, const char* song);

void cd_mpris_read_data (void);

void cd_mpris_configure (void);


MusicPlayerHandeler *cd_mpris_new_handler (void);


#endif

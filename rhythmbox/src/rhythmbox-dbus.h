#ifndef __rhythmbox_DBUS__
#define  __rhythmbox_DBUS__

gboolean rhythmbox_dbus_init(void);
int rhythmbox_getPlaying(void);
gchar *rhythmbox_getPlayingUri(void);
int rhythmbox_getElapsed(void);
gchar *rhythmbox_getSongName(gchar *uri);

#endif

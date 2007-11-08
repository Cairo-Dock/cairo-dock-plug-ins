#ifndef __rhythmbox_INIT__
#define  __rhythmbox_INIT__

#include <cairo-dock.h>
#include <dbus/dbus-glib.h>

gchar *cd_rhythmbox_pre_init (void);
Icon *cd_rhythmbox_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);
void cd_rhythmbox_stop (void);
gboolean rhythmbox_action (gpointer *data);
void rhythmbox_onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);
void rhythmbox_onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);

#endif



#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>
#include "applet-struct.h"

void cd_dbus_launch_service (void);

void cd_dbus_stop_service (void);


gboolean cd_dbus_main_show_desklet(dbusMainObject *dbusMainObject, gboolean *widgetLayer, GError **error);

gboolean cd_dbus_main_reboot(dbusMainObject *dbusMainObject, GError **error);

gboolean cd_dbus_main_quit (dbusMainObject *dbusMainObject, GError **error);

gboolean cd_dbus_main_show_dock (dbusMainObject *dbusMainObject, gboolean bShow, GError **error);

gboolean cd_dbus_main_reload_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_activate_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, gboolean bActivate, GError **error);

gboolean cd_dbus_main_load_launcher_from_file (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_main_create_launcher_from_scratch (dbusMainObject *dbusMainObject, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error);

gboolean cd_dbus_main_reload_launcher (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_main_remove_launcher (dbusMainObject *dbusMainObject, const gchar *cDesktopFile, GError **error);


gboolean cd_dbus_main_set_quick_info (dbusMainObject *dbusMainObject, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_set_label (dbusMainObject *dbusMainObject, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_set_icon (dbusMainObject *dbusMainObject, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_animate (dbusMainObject *dbusMainObject, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_main_show_dialog (dbusMainObject *dbusMainObject, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);


gboolean cd_dbus_main_register_new_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, gint iCategory, const gchar *cDescription, const gchar *cShareDataDir, GError **error);

gboolean cd_dbus_main_unregister_module (dbusMainObject *dbusMainObject, const gchar *cModuleName, GError **error);


#endif

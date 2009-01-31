
#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>

typedef struct
{
	GObject parent;
	DBusGConnection *connection;
} dbusCallback;

typedef struct
{
	GObjectClass parent_class;
} dbusCallbackClass;

void cd_dbus_launch_service (void);
void cd_dbus_stop_service (void);


gboolean cd_dbus_callback_hello(dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_show_dialog(dbusCallback *pDbusCallback, gchar *message, GError **error);

gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error);

gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_reload_module (dbusCallback *pDbusCallback, gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_show_dock (dbusCallback *pDbusCallback, gboolean bShow, GError **error);

gboolean cd_dbus_callback_load_launcher_from_file (dbusCallback *pDbusCallback, gchar *cDesktopFile, GError **error);

gboolean cd_dbus_callback_create_launcher_from_scratch (dbusCallback *pDbusCallback, gchar *cIconFile, gchar *cLabel, gchar *cCommand, gchar *cParentDockName, GError **error);

gboolean cd_dbus_callback_set_quick_info (dbusCallback *pDbusCallback, gchar *cQuickInfo, gchar *cIconName, gchar *cIconCommand, gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_set_label (dbusCallback *pDbusCallback, gchar *cLabel, gchar *cIconName, gchar *cIconCommand, gchar *cModuleName, GError **error);


#endif

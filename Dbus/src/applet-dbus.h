
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

#endif

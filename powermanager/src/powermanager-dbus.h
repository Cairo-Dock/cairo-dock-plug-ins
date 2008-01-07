#ifndef __POWERMANAGER_DBUS__
#define  __POWERMANAGER_DBUS__

#include <dbus/dbus-glib.h>

gboolean dbus_get_dbus (void);
void dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);

void onBatteryChanged(DBusGProxy *proxy, gboolean onBattery, gpointer data);

#endif

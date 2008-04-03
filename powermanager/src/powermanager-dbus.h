#ifndef __POWERMANAGER_DBUS__
#define  __POWERMANAGER_DBUS__

#include <glib.h>
#include <dbus/dbus-glib.h>

gboolean dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);

gboolean get_on_battery(void);
void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data);
gboolean update_stats(void);
void detect_battery(void);
int get_stats(gchar *dataType);

void power_halt(void);
void power_hibernate(void);
void power_suspend(void);
void power_reboot(void);
#endif

#include <string.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-dbus.h"

DBusGConnection *dbus_connexion_session;
DBusGConnection *dbus_connexion_system;
DBusGProxy *dbus_proxy_power;
DBusGProxy *dbus_proxy_battery;

CD_APPLET_INCLUDE_MY_VARS

extern gboolean dbus_enable;
extern gboolean battery_present;
extern gboolean on_battery;
extern int battery_time;
extern int battery_charge;

gboolean dbus_get_dbus (void)
{
	g_print ("%s ()\n",__func__);

	g_print ("Connexion au bus ... ");
	dbus_connexion_session = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_connexion_system = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	
	if(!dbus_connexion_session || !dbus_connexion_system)
	{
		g_print ("echouee\n");
		return FALSE;
	}
	else
	{
		g_print ("reussie\n");

		dbus_proxy_power = dbus_g_proxy_new_for_name (
			dbus_connexion_session,
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement",
			"org.freedesktop.PowerManagement"
		);
		
		dbus_proxy_battery = dbus_g_proxy_new_for_name (
			dbus_connexion_system,
			"org.freedesktop.Hal",
			"/org/freedesktop/Hal/devices/acpi_BAT0",
			"org.freedesktop.Hal.Device"
		);
		
		dbus_g_proxy_add_signal(dbus_proxy_power, "OnBatteryChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		
		return TRUE;
	}
}

void dbus_connect_to_bus (void)
{
	g_print ("%s ()\n",__func__);
	dbus_g_proxy_connect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL, NULL);
}

void dbus_disconnect_from_bus (void)
{
	g_print ("%s ()\n",__func__);
	dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL);
	g_print ("OnBatteryChanged deconnecte\n");
}

gboolean get_on_battery(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "GetOnBattery", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &on_battery,
		G_TYPE_INVALID);
}

void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data)
{
	if(onBattery)
	{
		on_battery = 1;
	}
	else
	{
		on_battery = 0;
	}
	update_icon();
}

void update_stats(void)
{
	if(battery_present)
	{
		dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyInteger", NULL,
			G_TYPE_STRING,"battery.charge_level.percentage",
			G_TYPE_INVALID,
			G_TYPE_INT, &battery_charge,
			G_TYPE_INVALID);
	}
	update_icon();
}

void detect_battery(void)
{
	dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyBoolean", NULL,
		G_TYPE_STRING,"battery.present",
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &battery_present,
		G_TYPE_INVALID);
}

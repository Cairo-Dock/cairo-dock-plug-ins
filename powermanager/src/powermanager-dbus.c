#include <string.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-dbus.h"

static DBusGConnection *dbus_connexion_session;
static DBusGConnection *dbus_connexion_system;
static DBusGProxy *dbus_proxy_power;
static DBusGProxy *dbus_proxy_battery;

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


gboolean dbus_get_dbus (void)
{
	cd_message ("%s ()\n",__func__);

	cd_message ("Connexion au bus ... ");
	dbus_connexion_session = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_connexion_system = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	
	if(!dbus_connexion_session || !dbus_connexion_system)
	{
		cd_message ("echouee\n");
		return FALSE;
	}
	else
	{
		cd_message ("reussie\n");

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
	cd_message ("%s ()\n",__func__);
	dbus_g_proxy_connect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL, NULL);
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("%s ()\n",__func__);
	dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL);
	cd_message ("OnBatteryChanged deconnecte\n");
}

gboolean get_on_battery(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "GetOnBattery", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &myData.on_battery,
		G_TYPE_INVALID);
}

void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data)
{
	myData.on_battery = onBattery;
	
	update_icon();
}

void update_stats(void)
{
	if(myData.battery_present)
	{
		dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyInteger", NULL,
			G_TYPE_STRING,"battery.charge_level.percentage",
			G_TYPE_INVALID,
			G_TYPE_INT, &myData.battery_charge,
			G_TYPE_INVALID);
		dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyInteger", NULL,
			G_TYPE_STRING,"battery.remaining_time",
			G_TYPE_INVALID,
			G_TYPE_INT, &myData.battery_time,
			G_TYPE_INVALID);
	}
	update_icon();
}

void detect_battery(void)
{
	dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyBoolean", NULL,
		G_TYPE_STRING,"battery.present",
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &myData.battery_present,
		G_TYPE_INVALID);
}

void power_halt(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Shutdown", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_hibernate(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Hibernate", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_suspend(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Suspend", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_reboot(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Reboot", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}

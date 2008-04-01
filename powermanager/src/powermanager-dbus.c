#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-dbus.h"

static DBusGConnection *dbus_connexion_session;
static DBusGConnection *dbus_connexion_system;
static DBusGProxy *dbus_proxy_power;
static DBusGProxy *dbus_proxy_stats;
static DBusGProxy *dbus_proxy_battery;

CD_APPLET_INCLUDE_MY_VARS

gchar* power_battery_name(void) {
  DIR *dir_fd;
  struct dirent *dir_data;
  char *battery_base_dir="/proc/acpi/battery";
  char *battery = "BAT0";
    
  dir_fd=opendir(battery_base_dir);
  if(dir_fd!=NULL) {
    int dir_found=0;

    readdir(dir_fd);
    readdir(dir_fd);

    while( !dir_found && (dir_data=readdir(dir_fd))!=NULL ) {
        battery = dir_data->d_name;
        dir_found=1;
    }

    closedir(dir_fd);
  }
    
  cd_message ("Battery Name: %s \n",battery);
  return battery;
}

gboolean dbus_get_dbus (void)
{
	cd_message ("");
  gchar *batteryPath = g_strdup_printf ("/org/freedesktop/Hal/devices/acpi_%s", power_battery_name());
  
	cd_message ("Connexion au bus ... ");
	dbus_connexion_session = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_connexion_system = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	
	if(!dbus_connexion_session || !dbus_connexion_system)
	{
		cd_message ("echouee");
		return FALSE;
	}
	else
	{
		cd_message ("reussie");

		dbus_proxy_power = dbus_g_proxy_new_for_name (
			dbus_connexion_session,
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement",
			"org.freedesktop.PowerManagement"
		);

		dbus_proxy_stats = dbus_g_proxy_new_for_name (
			dbus_connexion_session,
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement/Statistics",
			"org.freedesktop.PowerManagement.Statistics"
		);
		
		dbus_proxy_battery = dbus_g_proxy_new_for_name (
			dbus_connexion_system,
			"org.freedesktop.Hal",
			batteryPath,
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
	cd_message ("");
	dbus_g_proxy_connect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL, NULL);
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(on_battery_changed), NULL);
	cd_message ("OnBatteryChanged deconnecte");
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

gboolean update_stats(void)
{
	if(myData.battery_present)
	{
		myData.battery_charge = get_stats("charge");
		myData.battery_time = get_stats("time");
		update_icon();
	}
	return TRUE;
}

int get_stats(gchar *dataType)
{
	GValueArray *gva;
	GValue *gv;
	GPtrArray *ptrarray = NULL;
	GType g_type_ptrarray;
	int i;
	int x, y, col;  /// mettre des nom comprehensibles...

	g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INVALID));
		
	dbus_g_proxy_call (dbus_proxy_stats, "GetData", NULL,
		 G_TYPE_STRING, dataType,
		 G_TYPE_INVALID,
		 g_type_ptrarray, &ptrarray,
		 G_TYPE_INVALID);
	
	for (i=0; i< ptrarray->len; i++)  /// il semble que seule la derniere valeur ait de l'interet ....
	{
		gva = (GValueArray *) g_ptr_array_index (ptrarray, i);
		gv = g_value_array_get_nth (gva, 0);
		x = g_value_get_int (gv);
		g_value_unset (gv);
		gv = g_value_array_get_nth (gva, 1);
		y = g_value_get_int (gv);
		g_value_unset (gv);
		gv = g_value_array_get_nth (gva, 2);
		col = g_value_get_int (gv);
		g_value_unset (gv);
		g_value_array_free (gva);
	}
	g_ptr_array_free (ptrarray, TRUE);
	
	cd_debug ("PowerManager [%s]: %d", dataType, y); 
	return y;  /// a quoi servent x et col alors ??
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

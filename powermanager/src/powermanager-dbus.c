#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-dbus.h"

#define MY_BATTERY_DIR "/proc/acpi/battery"
#define MY_BATTERY_NAME "BAT0"

static DBusGConnection *dbus_connexion_system = NULL;
static DBusGProxy *dbus_proxy_power = NULL;
static DBusGProxy *dbus_proxy_stats = NULL;
static DBusGProxy *dbus_proxy_battery = NULL;

CD_APPLET_INCLUDE_MY_VARS

static const gchar* power_battery_name(void) {
	GError *erreur = NULL;
	GDir *dir = g_dir_open (MY_BATTERY_DIR, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	const gchar *cBatteryName;
	cBatteryName = g_dir_read_name (dir);  // le 1er fichier trouve, ou NULL si aucun.
	g_dir_close (dir);
	if (cBatteryName == NULL)
		cBatteryName = MY_BATTERY_NAME;  // utile ? si y'a rien dans ce repertoire, c'est surement qu'il n'y a pas de batterie non ?
	cd_message ("Battery Name: %s", cBatteryName);
	return cBatteryName;
	/*DIR *dir_fd;
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
  cd_message ("Battery Name: %s",battery);*/
}

gboolean dbus_connect_to_bus (void)
{
	cd_message ("");
	
	if (dbus_connexion_system == NULL)
		dbus_connexion_system = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (cairo_dock_bdus_is_enabled () && dbus_connexion_system != NULL)
	{
		dbus_proxy_power = cairo_dock_create_new_dbus_proxy (
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement",
			"org.freedesktop.PowerManagement"
		);

		dbus_proxy_stats = cairo_dock_create_new_dbus_proxy (
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement/Statistics",
			"org.freedesktop.PowerManagement.Statistics"
		);
		
		dbus_g_proxy_add_signal(dbus_proxy_power, "OnBatteryChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		
		
		gchar *batteryPath = g_strdup_printf ("/org/freedesktop/Hal/devices/acpi_%s", power_battery_name());
		dbus_proxy_battery = dbus_g_proxy_new_for_name (
			dbus_connexion_system,
			"org.freedesktop.Hal",
			batteryPath,
			"org.freedesktop.Hal.Device"
		);
		g_free (batteryPath);
		
		return TRUE;
	}
	return FALSE;
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_power != NULL)
	{
		dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
			G_CALLBACK(on_battery_changed), NULL);
		cd_message ("OnBatteryChanged deconnecte");
		g_object_unref (dbus_proxy_power);
		dbus_proxy_power = NULL;
	}
	if (dbus_proxy_battery != NULL)
	{
		g_object_unref (dbus_proxy_battery);
		dbus_proxy_battery = NULL;
	}
	if (dbus_proxy_stats != NULL)
	{
		g_object_unref (dbus_proxy_stats);
		dbus_proxy_stats = NULL;
	}
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
	if (dbus_proxy_battery != NULL)
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

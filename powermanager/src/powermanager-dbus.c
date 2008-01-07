#include <string.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-dbus.h"

DBusGConnection *dbus_connexion;
DBusGProxy *dbus_proxy_power;

CD_APPLET_INCLUDE_MY_VARS

extern gboolean dbus_enable;
extern cairo_surface_t *powermanager_pSurfaceSector44;
extern cairo_surface_t *powermanager_pSurface44;


gboolean dbus_get_dbus (void)
{
	g_print ("%s ()\n",__func__);

	g_print ("Connexion au bus ... ");
	dbus_connexion = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	
	if(!dbus_connexion)
	{
		g_print ("echouee\n");
		return FALSE;
	}
	else
	{
		g_print ("reussie\n");

		/*
		dbus_proxy_battery = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.freedesktop.Hal",
			"/org/freedesktop/Hal/devices/acpi_BAT0",
			"org.freedesktop.Hal.Device"
		);
		*/
		
		dbus_proxy_power = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement",
			"org.freedesktop.PowerManagement"
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
		G_CALLBACK(onBatteryChanged), NULL, NULL);
}

void dbus_disconnect_from_bus (void)
{
	g_print ("%s ()\n",__func__);
	dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
		G_CALLBACK(onBatteryChanged), NULL);
	g_print ("OnBatteryChanged deconnecte\n");
}

void onBatteryChanged(DBusGProxy *proxy, gboolean onBattery, gpointer data)
{
	if(onBattery)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurface44);
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceSector44);
	}
}

#include <stdlib.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-init.h"

cairo_surface_t *powermanager_pSurfaceBattery04 = NULL;
cairo_surface_t *powermanager_pSurfaceBattery14 = NULL;
cairo_surface_t *powermanager_pSurfaceBattery24 = NULL;
cairo_surface_t *powermanager_pSurfaceBattery34 = NULL;
cairo_surface_t *powermanager_pSurfaceBattery44 = NULL;
cairo_surface_t *powermanager_pSurfaceCharge04 = NULL;
cairo_surface_t *powermanager_pSurfaceCharge14 = NULL;
cairo_surface_t *powermanager_pSurfaceCharge24 = NULL;
cairo_surface_t *powermanager_pSurfaceCharge34 = NULL;
cairo_surface_t *powermanager_pSurfaceCharge44 = NULL;
cairo_surface_t *powermanager_pSurfaceSector = NULL;
cairo_surface_t *powermanager_pSurfaceBroken = NULL;

gchar *conf_defaultTitle = NULL;
gboolean dbus_enable = FALSE;
int checkLoop = -1;

gboolean on_battery = FALSE;
gboolean battery_present = FALSE;
int battery_time = 0;
int battery_charge = 0;

CD_APPLET_DEFINITION ("PowerManager", 1, 4, 7)

CD_APPLET_INIT_BEGIN (erreur)
	conf_defaultTitle = g_strdup (myIcon->acName);

	GString *sImagePath = g_string_new ("");  // ce serait bien de pouvoir choisir ses icones, comme dans l'applet logout...
	//Chargement de l'image "default"
	g_string_printf (sImagePath, "%s/battery_44.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBattery44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_34.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBattery34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_24.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBattery24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_14.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBattery14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_04.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBattery04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_44.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceCharge44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_34.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceCharge34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_24.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceCharge24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_14.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceCharge14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_04.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceCharge04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/sector.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceBroken = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	g_string_free (sImagePath, TRUE);

	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (!dbus_enable) dbus_enable = dbus_get_dbus();

	//Si le bus a ete acquis, on y connecte nos signaux.
	if (dbus_enable)
	{
		dbus_connect_to_bus ();
		detect_battery();
		if(battery_present)
		{
			get_on_battery();
			update_stats();
			checkLoop = g_timeout_add ((int) 10000, (GSourceFunc) update_stats, (gpointer) NULL);
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceSector)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBroken)
	}

	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_CONFIGURE_BEGIN
CD_APPLET_CONFIGURE_END

CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT

	if (dbus_enable)
	{
		dbus_disconnect_from_bus ();

		if(battery_present)
		{
			g_source_remove (checkLoop);
			checkLoop = 0;
		}
	}

	g_free (conf_defaultTitle);
	conf_defaultTitle = NULL;

	cairo_surface_destroy (powermanager_pSurfaceBattery44);
	powermanager_pSurfaceBattery44 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceBattery34);
	powermanager_pSurfaceBattery34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceBattery24);
	powermanager_pSurfaceBattery24 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceBattery14);
	powermanager_pSurfaceBattery14 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceBattery04);
	powermanager_pSurfaceBattery04 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceCharge44);
	powermanager_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceCharge34);
	powermanager_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceCharge24);
	powermanager_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceCharge14);
	powermanager_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceCharge04);
	powermanager_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector);
	powermanager_pSurfaceSector = NULL;
	cairo_surface_destroy (powermanager_pSurfaceBroken);
	powermanager_pSurfaceBroken = NULL;
CD_APPLET_STOP_END

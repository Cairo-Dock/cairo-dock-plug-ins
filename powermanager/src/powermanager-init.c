#include <stdlib.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-init.h"

cairo_surface_t *my_pSurfaceBattery04 = NULL;
cairo_surface_t *my_pSurfaceBattery14 = NULL;
cairo_surface_t *my_pSurfaceBattery24 = NULL;
cairo_surface_t *my_pSurfaceBattery34 = NULL;
cairo_surface_t *my_pSurfaceBattery44 = NULL;
cairo_surface_t *my_pSurfaceCharge04 = NULL;
cairo_surface_t *my_pSurfaceCharge14 = NULL;
cairo_surface_t *my_pSurfaceCharge24 = NULL;
cairo_surface_t *my_pSurfaceCharge34 = NULL;
cairo_surface_t *my_pSurfaceCharge44 = NULL;
cairo_surface_t *my_pSurfaceSector = NULL;
cairo_surface_t *my_pSurfaceBroken = NULL;

gchar *conf_defaultTitle = NULL;
gboolean dbus_enable = FALSE;
int checkLoop = -1;

gboolean on_battery = FALSE;
gboolean battery_present = FALSE;
int battery_time = 0;
int battery_charge = 0;


CD_APPLET_DEFINITION ("PowerManager", 1, 4, 7)


static void _load_surfaces (void)
{
	GString *sImagePath = g_string_new ("");
	
	g_string_printf (sImagePath, "%s/battery_44.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBattery44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_34.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBattery34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_24.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBattery24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_14.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBattery14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_04.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBattery04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_44.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceCharge44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_34.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceCharge34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_24.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceCharge24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_14.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceCharge14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_04.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceCharge04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/sector.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceSector = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	my_pSurfaceBroken = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
	conf_defaultTitle = g_strdup (myIcon->acName);
	
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
			CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pSurfaceSector)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pSurfaceBroken)
	}
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


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
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	_load_surfaces ();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		g_free (conf_defaultTitle);
		conf_defaultTitle = g_strdup (myIcon->acName);
		
		//\_______________ On stoppe le timer.
		g_source_remove (checkLoop);
		checkLoop = 0;
		
		//\_______________ On relance le timer.
		update_stats();
		checkLoop = g_timeout_add ((int) 10000, (GSourceFunc) update_stats, (gpointer) NULL);
	}
	else
	{
		//\_______________ On redessine notre icone.
		update_stats();
	}
CD_APPLET_RELOAD_END

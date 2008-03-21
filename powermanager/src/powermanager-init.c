#include <stdlib.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-struct.h"
#include "powermanager-init.h"

extern AppletConfig myConfig;
extern AppletData myData;

static gboolean dbus_enable = FALSE;


CD_APPLET_DEFINITION ("PowerManager", 1, 5, 3, CAIRO_DOCK_CATEGORY_ACCESSORY)


/*static void _load_surfaces (void)
{
	reset_surfaces ();
	
	GString *sImagePath = g_string_new ("");
	
	g_string_printf (sImagePath, "%s/battery_44.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBattery44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_34.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBattery34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_24.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBattery24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_14.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBattery14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_04.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBattery04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_44.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceCharge44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_34.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceCharge34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_24.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceCharge24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_14.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceCharge14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_04.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceCharge04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/sector.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceSector = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurfaceBroken = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	g_string_free (sImagePath, TRUE);
}*/

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	
	//_load_surfaces ();  // on ne charge pas toutes les surfaces, car cela prend trop de memoire, et trop de temps au chargement, alors que ce n'est pas necessaire. En effet, on ne redessine que si il y'a changement. Or la batterie se vide lentement, et la recharge n'est pas non plus fulgurante, donc au total on redesine reellement l'icone 1 fois toutes les 10 minutes peut-etre, ce qui ne justifie pas de pre-charger les surfaces.
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (! dbus_enable)
		dbus_enable = dbus_get_dbus();
	myData.dbus_enable = dbus_enable;
	
	//Si le bus a ete acquis, on y connecte nos signaux.
	if (dbus_enable)
	{
		dbus_connect_to_bus ();
		detect_battery();
		if(myData.battery_present)
		{
			get_on_battery();
			update_stats();
			myData.checkLoop = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) update_stats, (gpointer) NULL);
		}
		else
		{
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg")
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("broken.svg")
	}
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	if (dbus_enable)
	{
		dbus_disconnect_from_bus ();
		
		if(myData.battery_present)
		{
			g_source_remove (myData.checkLoop);
			myData.checkLoop = 0;
		}
	}
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	
	//\_______________ On recharge les donnees qui ont pu changer.
	//_load_surfaces ();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)  // si la frequence du timer passe en conf, il faudra l'arreter et le relancer.
	{
		if(myData.checkLoop != 0)  // la frequence peut avoir change.
		{
			g_source_remove (myData.checkLoop);
			myData.checkLoop = 0;
		}
		myData.checkLoop = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) update_stats, (gpointer) NULL);
	}
	
	//\_______________ On redessine notre icone.
	if (dbus_enable)
	{
		if(myData.battery_present)
		{
			myData.previously_on_battery = -1;  // pour forcer le redessin.
			myData.previous_battery_charge = -1;
			myData.previous_battery_time = -1;
			update_stats();
		}
		else
		{
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg")
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("broken.svg")
	}
	
CD_APPLET_RELOAD_END

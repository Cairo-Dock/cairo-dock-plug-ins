#include <stdlib.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-struct.h"
#include "powermanager-init.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_DEFINITION ("PowerManager", 1, 4, 7, CAIRO_DOCK_CATEGORY_ACCESSORY)


static void _load_surfaces (void)
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
}

CD_APPLET_INIT_BEGIN (erreur)
	myConfig.defaultTitle = g_strdup (myIcon->acName);
	
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
	
	_load_surfaces ();
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (! myData.dbus_enable)
		myData.dbus_enable = dbus_get_dbus();
	
	//Si le bus a ete acquis, on y connecte nos signaux.
	if (myData.dbus_enable)
	{
		dbus_connect_to_bus ();
		detect_battery();
		if(myData.battery_present)
		{
			get_on_battery();
			update_stats();
			myData.checkLoop = g_timeout_add ((int) 10000, (GSourceFunc) update_stats, (gpointer) NULL);
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceSector)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	if (myData.dbus_enable)
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
	_load_surfaces ();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)  // si la frequence du timer passe en conf, il faudra l'arreter et le relancer.
	{
		myConfig.defaultTitle = g_strdup (myIcon->acName);  // libere dans le reset_config() precedemment appele.
	}
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		if(myData.battery_present)
		{
			update_stats();
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceSector)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
	
CD_APPLET_RELOAD_END

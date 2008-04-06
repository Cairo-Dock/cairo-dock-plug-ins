#include <stdlib.h>
#include <cairo-dock.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-struct.h"
#include "powermanager-init.h"


CD_APPLET_DEFINITION ("PowerManager", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN (erreur)
	
	if (myDesklet != NULL)
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	// on ne charge pas toutes les surfaces, car cela prend trop de memoire, et trop de temps au chargement, alors que ce n'est pas necessaire. En effet, on ne redessine que si il y'a changement. Or la batterie se vide lentement, et la recharge n'est pas non plus fulgurante, donc au total on redesine reellement l'icone 1 fois toutes les 10 minutes peut-etre, ce qui ne justifie pas de pre-charger les surfaces.
	
	myData.dbus_enable = dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		detect_battery();
		if(myData.battery_present)
		{
			get_on_battery();
			
			double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
			myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
			
			myData.alerted = TRUE;
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
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	dbus_disconnect_from_bus ();
	
	if(myData.checkLoop != 0)
	{
		g_source_remove (myData.checkLoop);
		myData.checkLoop = 0;
	}
	
	free_cd_Gauge(myData.pGauge);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet != NULL)
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if(myData.checkLoop != 0)  // la frequence peut avoir change.
		{
			g_source_remove (myData.checkLoop);
			myData.checkLoop = 0;
		}
		myData.checkLoop = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) update_stats, (gpointer) NULL);
	}
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		if(myData.battery_present)
		{
			double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
			free_cd_Gauge(myData.pGauge);
			myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
			
			myData.previously_on_battery = -1;  // pour forcer le redessin.
			myData.previous_battery_charge = -1;
			myData.previous_battery_time = -1;
			myData.alerted = TRUE;
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

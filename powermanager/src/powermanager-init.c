#include <stdlib.h>
#include <cairo-dock.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-struct.h"
#include "powermanager-init.h"


CD_APPLET_DEFINITION ("PowerManager", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN
	
	if (myDesklet)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	
	// on ne charge pas toutes les surfaces, car cela prend trop de memoire, et trop de temps au chargement, alors que ce n'est pas necessaire. En effet, on ne redessine que si il y'a changement. Or la batterie se vide lentement, et la recharge n'est pas non plus fulgurante, donc au total on redesine reellement l'icone 1 fois toutes les 10 minutes peut-etre, ce qui ne justifie pas de pre-charger les surfaces.
	
	myData.dbus_enable = dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		if(myData.battery_present)
		{
			///get_on_battery();
			
			if (myConfig.bUseGauge)
			{
				double fMaxScale = cairo_dock_get_max_scale (myContainer);
				myData.pGauge = cairo_dock_load_gauge (myDrawContext, myConfig.cGThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
			}
			
			myData.previous_battery_charge = -1;
			myData.previous_battery_time = -1;
			myData.alerted = TRUE;
			myData.bCritical = TRUE;
			update_stats();
			myData.checkLoop = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) update_stats, (gpointer) NULL);
		}
		else
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg");

	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("broken.svg");
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	dbus_disconnect_from_bus ();
	
	if(myData.checkLoop != 0)
	{
		g_source_remove (myData.checkLoop);
		myData.checkLoop = 0;
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cairo_dock_free_gauge(myData.pGauge);
		myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
		
		if(myData.checkLoop != 0)  // la frequence peut avoir change.
		{
			g_source_remove (myData.checkLoop);
			myData.checkLoop = 0;
		}
		myData.checkLoop = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) update_stats, (gpointer) NULL);
		
	}
	else
		cairo_dock_reload_gauge (myDrawContext, myData.pGauge, myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		if(myData.battery_present)
		{
			if (myConfig.bUseGauge)  // On recharge la jauge.
			{
				///double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
				///myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
				
				cairo_dock_render_gauge (myDrawContext, myContainer, myIcon, myData.pGauge, (double) myData.battery_charge / 100);
				
				//Embleme sur notre icône
				CD_APPLET_DRAW_EMBLEM ((myData.on_battery ? CAIRO_DOCK_EMBLEM_BLANK : CAIRO_DOCK_EMBLEM_CHARGE), CAIRO_DOCK_EMBLEM_MIDDLE);
			}
			else  // on redessine juste l'icone actuelle.
				cd_powermanager_draw_icon_with_effect (myData.on_battery);
			
			if (!myData.on_battery && myData.battery_charge < 100)
				myData.alerted = FALSE; //We will alert when battery charge reach 100%
			if (myData.on_battery)
			{
				if (myData.battery_charge > myConfig.lowBatteryValue)
					myData.alerted = FALSE; //We will alert when battery charge is under myConfig.lowBatteryValue
				
				if (myData.battery_charge > 4)
					myData.bCritical = FALSE; //We will alert when battery charge is critical (under 4%)
			}
			
			myData.previous_battery_charge = -1;
			myData.previous_battery_time = -1;
			update_icon();
		}
		else
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg");

	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("broken.svg");
	
CD_APPLET_RELOAD_END

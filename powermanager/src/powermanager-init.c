#include <stdlib.h>

#include "powermanager-draw.h"
#include "powermanager-config.h"
#include "powermanager-dbus.h"
#include "powermanager-menu-functions.h"
#include "powermanager-init.h"

cairo_surface_t *powermanager_pSurface04 = NULL;
cairo_surface_t *powermanager_pSurface14 = NULL;
cairo_surface_t *powermanager_pSurface24 = NULL;
cairo_surface_t *powermanager_pSurface34 = NULL;
cairo_surface_t *powermanager_pSurface44 = NULL;
cairo_surface_t *powermanager_pSurfaceSector04 = NULL;
cairo_surface_t *powermanager_pSurfaceSector14 = NULL;
cairo_surface_t *powermanager_pSurfaceSector24 = NULL;
cairo_surface_t *powermanager_pSurfaceSector34 = NULL;
cairo_surface_t *powermanager_pSurfaceSector44 = NULL;
cairo_surface_t *powermanager_pBrokenSurface = NULL;

gchar *conf_defaultTitle = NULL;
gboolean dbus_enable = FALSE;

CD_APPLET_DEFINITION ("PowerManager", 1, 4, 6)

CD_APPLET_INIT_BEGIN (erreur)
	conf_defaultTitle = g_strdup (myIcon->acName);
	
	GString *sImagePath = g_string_new ("");  // ce serait bien de pouvoir choisir ses icones, comme dans l'applet logout...
	//Chargement de l'image "default"
	g_string_printf (sImagePath, "%s/battery_44.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurface44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_34.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurface34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_24.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurface24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_14.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurface14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/battery_04.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurface04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_44.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector44 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_34.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector34 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_24.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector24 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_14.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector14 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/charge_04.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pSurfaceSector04 = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	powermanager_pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	g_string_free (sImagePath, TRUE);
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (!dbus_enable) dbus_enable = dbus_get_dbus();
	
	//Si le bus a ete acquis, on y connecte nos signaux.
	if (dbus_enable)
	{
		dbus_connect_to_bus ();
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pBrokenSurface)
	}
CD_APPLET_INIT_END

CD_APPLET_STOP_BEGIN
	dbus_disconnect_from_bus ();
	
	g_free (conf_defaultTitle);
	conf_defaultTitle = NULL;
	
	cairo_surface_destroy (powermanager_pSurface44);
	powermanager_pSurface44 = NULL;
	cairo_surface_destroy (powermanager_pSurface34);
	powermanager_pSurface34 = NULL;
	cairo_surface_destroy (powermanager_pSurface24);
	powermanager_pSurface24 = NULL;
	cairo_surface_destroy (powermanager_pSurface14);
	powermanager_pSurface14 = NULL;
	cairo_surface_destroy (powermanager_pSurface04);
	powermanager_pSurface04 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector44);
	powermanager_pSurfaceSector34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector34);
	powermanager_pSurfaceSector34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector24);
	powermanager_pSurfaceSector34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector14);
	powermanager_pSurfaceSector34 = NULL;
	cairo_surface_destroy (powermanager_pSurfaceSector04);
	powermanager_pSurfaceSector34 = NULL;
	cairo_surface_destroy (powermanager_pBrokenSurface);
	powermanager_pBrokenSurface = NULL;
CD_APPLET_STOP_END

/*
gboolean powermanager_checkCharge(void)
{	
	int charge,time;
	gchar *viewableTime;
	
	charge = powermanager_getData(g_strdup_printf ("charge"));
	time = powermanager_getData(g_strdup_printf ("time"));
	
	if(batteryState)
	{
		if(charge >= 95) powermanager_setIconSurface(powermanager_pSurface44);
		else if(charge >= 65) powermanager_setIconSurface(powermanager_pSurface34);
		else if(charge >= 35) powermanager_setIconSurface(powermanager_pSurface24);
		else if(charge >= 5) powermanager_setIconSurface(powermanager_pSurface14);
		else powermanager_setIconSurface(powermanager_pSurface04);
	}
	else
	{
		if(lastCharge < 100 && charge == 100 && conf_highBatteryWitness)
		{
			powermanager_iconWitness(1);
			cairo_dock_show_temporary_dialog (g_strdup_printf("Batterie rechargé")
				,powermanager_pIcon
				,powermanager_pDock
				,5000);
		}
		if(lastTime > conf_lowBatteryLimit && (time/60) <= conf_lowBatteryLimit && conf_lowBatteryWitness)
		{
			powermanager_iconWitness(1);
			cairo_dock_show_temporary_dialog (g_strdup_printf("Batterie faible")
				,powermanager_pIcon
				,powermanager_pDock
				,5000);
		}
		
		if(charge >= 95) powermanager_setIconSurface(powermanager_pSurfaceSector44);
		else if(charge >= 65) powermanager_setIconSurface(powermanager_pSurfaceSector34);
		else if(charge >= 35) powermanager_setIconSurface(powermanager_pSurfaceSector24);
		else if(charge >= 5) powermanager_setIconSurface(powermanager_pSurfaceSector14);
		else powermanager_setIconSurface(powermanager_pSurfaceSector04);
	}
	
	if(time <= 60) viewableTime = g_strdup_printf("1 minute restante (%i %)",charge);
	else if(time < 3600) viewableTime = g_strdup_printf("%i minutes restantes (%i %)",(int) (time/60),charge);
	else if(time < 7200) viewableTime = g_strdup_printf("%i heure %i restante (%i %)",(int) (time/3600),(int) ((time%3600)/60),charge);
	else viewableTime = g_strdup_printf("%i heures %i restantes (%i %)",(int) (time/3600),(int) ((time%3600)/60),charge);
	gchar *chargeName;
	gchar *timeName;
	chargeName = "charge";
	timeName = "time";
	cairo_dock_set_icon_name (powermanager_pCairoContext, viewableTime, powermanager_pIcon, powermanager_pDock);
	
	if(*conf_quickInfoType == *chargeName)
	{
		cairo_dock_set_quick_info (powermanager_pCairoContext, g_strdup_printf("%i",charge), powermanager_pIcon);
	}
	else if(*conf_quickInfoType == *timeName)
	{
		if(time < 60)
		{
			cairo_dock_set_quick_info (powermanager_pCairoContext, g_strdup_printf("%i%s",time/60,"mn"), powermanager_pIcon);
		}
		else
		{
			cairo_dock_set_quick_info (powermanager_pCairoContext, g_strdup_printf("%i%s",time/3600,"h"), powermanager_pIcon);
		}
	}
	lastCharge = charge;
	lastTime = time/60;
	return TRUE;
}
*/

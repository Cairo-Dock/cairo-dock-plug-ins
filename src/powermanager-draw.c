#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern cairo_surface_t *powermanager_pSurfaceBattery04;
extern cairo_surface_t *powermanager_pSurfaceBattery14;
extern cairo_surface_t *powermanager_pSurfaceBattery24;
extern cairo_surface_t *powermanager_pSurfaceBattery34;
extern cairo_surface_t *powermanager_pSurfaceBattery44;
extern cairo_surface_t *powermanager_pSurfaceCharge04;
extern cairo_surface_t *powermanager_pSurfaceCharge14;
extern cairo_surface_t *powermanager_pSurfaceCharge24;
extern cairo_surface_t *powermanager_pSurfaceCharge34;
extern cairo_surface_t *powermanager_pSurfaceCharge44;
extern cairo_surface_t *powermanager_pSurfaceSector;

extern gboolean on_battery;
extern gboolean battery_present;
extern int battery_time;
extern int battery_charge;

void iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (1, animationLenght)
}

void update_icon(void)
{
	if(battery_present)
	{
		cairo_dock_set_quick_info (myDrawContext, g_strdup_printf ("%d%s", battery_charge,"%"), myIcon);
		if(on_battery)
		{
			if(battery_charge >= 95)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBattery44) }
			else if(battery_charge >= 65)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBattery34) }
			else if(battery_charge >= 35)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBattery24) }
			else if(battery_charge >= 5)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBattery14) }
			else
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceBattery04) }
		}
		else
		{
			if(battery_charge >= 95)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceCharge44) }
			else if(battery_charge >= 65)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceCharge34) }
			else if(battery_charge >= 35)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceCharge24) }
			else if(battery_charge >= 5)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceCharge14) }
			else
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceCharge04) }
		}
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (powermanager_pSurfaceSector)
	}
}

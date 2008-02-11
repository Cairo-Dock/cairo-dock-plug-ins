#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


void iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (1, animationLenght)
}

void update_icon(void)
{
	if(myData.battery_present)
	{g_print("Type d'affichage : %u\n",myConfig.quickInfoType);
		if(myConfig.quickInfoType == MY_APPLET_TIME)
		{
			cairo_dock_set_quick_info (myDrawContext, format_time(myData.battery_time), myIcon, (myDock != NULL ? 1 + g_fAmplitude : 1));
		}
		else if(myConfig.quickInfoType == MY_APPLET_CHARGE)
		{
			cairo_dock_set_quick_info (myDrawContext, g_strdup_printf ("%d%s", myData.battery_charge,"%"), myIcon, (myDock != NULL ? 1 + g_fAmplitude : 1));
		}
		if(myData.on_battery)
		{
			if(myData.battery_charge >= 95)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBattery44) }
			else if(myData.battery_charge >= 65)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBattery34) }
			else if(myData.battery_charge >= 35)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBattery24) }
			else if(myData.battery_charge >= 5)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBattery14) }
			else
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBattery04) }
		}
		else
		{
			if(myData.battery_charge >= 95)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceCharge44) }
			else if(myData.battery_charge >= 65)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceCharge34) }
			else if(myData.battery_charge >= 35)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceCharge24) }
			else if(myData.battery_charge >= 5)
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceCharge14) }
			else
				{ CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceCharge04) }
		}
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceSector)
	}
}

gchar *format_time(int seconde)
{
	if(myData.on_battery)
	{
		int hours = seconde / 3600;
		int minutes = (seconde % 3600) / 60;
		if(hours > 0)
		{
			if(minutes < 10) return g_strdup_printf("%ih0%i",hours,minutes);
			else return g_strdup_printf("%ih%i",hours,minutes);
		}
		else
		{
			if(minutes > 0) return g_strdup_printf("%i",minutes);
			else return NULL;
		}
	}
	else return NULL;
}

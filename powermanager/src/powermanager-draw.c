#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"

CD_APPLET_INCLUDE_MY_VARS


void iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (1, animationLenght)
}

void update_icon(void)
{
	if(myData.battery_present)
	{
		if (myData.previous_battery_time != myData.battery_time)
		{
			if(myConfig.quickInfoType == POWER_MANAGER_TIME)
			{
				CD_APPLET_SET_HOURS_MINUTES_AS_QUICK_INFO (myData.battery_time)
			}
			else if(myConfig.quickInfoType == POWER_MANAGER_CHARGE)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", myData.battery_charge)
			}
			else
			{
			  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
			}
		}
		
		make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) myData.battery_charge / 100);
	}
	else
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg")
	}
	CD_APPLET_REDRAW_MY_ICON
	//Il faut recharger l'icône car elle ne se met pas a jour a tout les coups
}

gchar *get_hours_minutes(int x) {
  gchar *time=_D("None");
  int h=0, m=0;
  m = x / 60;
  h = m / 60;
  m = m - (h * 60);
  if (m > 0)
  {
    time = g_strdup_printf("%dm", m);
  }
  if (h > 0)
  {
    time = g_strdup_printf("%dh%dm", h, m);
  }
  cd_message("%dh%dm", h, m);
  return time;
}

void cd_powermanager_bubble(void)
{
  gchar *hms = get_hours_minutes(myData.battery_time);
  if(myData.on_battery)
	{
	  cairo_dock_show_temporary_dialog ("%s %d%% \n %s %s", myIcon, myContainer, 6000, _D("Laptop on Battery \n Battery charged at:"), myData.battery_charge, _D("Estimated time with Charge:"), hms);
	}
	else
	{
	  cairo_dock_show_temporary_dialog ("%s %d%% \n %s %s", myIcon, myContainer, 6000, _D("Laptop on Charge \n Battery charged at:"), myData.battery_charge, _D("Estimated time with Charge:"), hms);
	}
}

void cd_powermanager_set_surface (gchar *svgFile) {	
	cairo_surface_t *pSurface;
	gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, svgFile);
	pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
	g_free (cImagePath);
	
	switch (myConfig.iEffect) {
    case POWER_MANAGER_EFFECT_NONE:
      CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
    break;
    case POWER_MANAGER_EFFECT_ZOOM:
     	cairo_save (myDrawContext);
	    double fScale = .3 + .7 * myData.battery_charge / 100.;
	    CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale)
      cairo_restore (myDrawContext);
	  break;
	  case POWER_MANAGER_EFFECT_TRANSPARENCY: 
	    cairo_save (myDrawContext);
	    double fAlpha = .3 + .7 * myData.battery_charge / 100.;
	    CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha)
	    cairo_restore (myDrawContext);
	  break;
	  case POWER_MANAGER_EFFECT_BAR:
	    cairo_save (myDrawContext);
	    CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR(pSurface, myData.battery_charge * .01)
	    cairo_restore (myDrawContext);
	  break;
	}
	cairo_surface_destroy (pSurface);
}


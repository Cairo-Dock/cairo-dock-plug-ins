#include <string.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_CONFIG_BEGIN ("PowerManager", NULL)
	reset_config ();
	
	
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	cairo_surface_destroy (myData.pSurfaceBattery44);
	myData.pSurfaceBattery44 = NULL;
	cairo_surface_destroy (myData.pSurfaceBattery34);
	myData.pSurfaceBattery34 = NULL;
	cairo_surface_destroy (myData.pSurfaceBattery24);
	myData.pSurfaceBattery24 = NULL;
	cairo_surface_destroy (myData.pSurfaceBattery14);
	myData.pSurfaceBattery14 = NULL;
	cairo_surface_destroy (myData.pSurfaceBattery04);
	myData.pSurfaceBattery04 = NULL;
	cairo_surface_destroy (myData.pSurfaceCharge44);
	myData.pSurfaceCharge34 = NULL;
	cairo_surface_destroy (myData.pSurfaceCharge34);
	myData.pSurfaceCharge34 = NULL;
	cairo_surface_destroy (myData.pSurfaceCharge24);
	myData.pSurfaceCharge34 = NULL;
	cairo_surface_destroy (myData.pSurfaceCharge14);
	myData.pSurfaceCharge34 = NULL;
	cairo_surface_destroy (myData.pSurfaceCharge04);
	myData.pSurfaceCharge34 = NULL;
	cairo_surface_destroy (myData.pSurfaceSector);
	myData.pSurfaceSector = NULL;
	cairo_surface_destroy (myData.pSurfaceBroken);
	myData.pSurfaceBroken = NULL;
	
	gboolean dbus_enable = myData.dbus_enable;
	memset (&myData, 0, sizeof (AppletData));
	myData.dbus_enable = dbus_enable;
}

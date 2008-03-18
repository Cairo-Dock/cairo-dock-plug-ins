#include <string.h>

#include "tomboy-struct.h"
#include "tomboy-config.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	
	myConfig.cIconDefault 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cIconClose			= CD_CONFIG_GET_STRING ("Configuration", "close icon");
	myConfig.cIconBroken 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.cIconDefault);
	myConfig.cIconDefault = NULL;
	g_free (myConfig.cIconClose);
	myConfig.cIconClose = NULL;
	g_free (myConfig.cIconBroken);
	myConfig.cIconBroken = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	cairo_surface_destroy (myData.pSurfaceDefault);
	myData.pSurfaceDefault = NULL;
	cairo_surface_destroy (myData.pSurfaceClose);
	myData.pSurfaceClose = NULL;
	cairo_surface_destroy (myData.pSurfaceBroken);
	myData.pSurfaceBroken = NULL;
	
	gboolean dbus_enable = myData.dbus_enable;
	memset (&myData, 0, sizeof (AppletData));
	myData.dbus_enable = dbus_enable;
}

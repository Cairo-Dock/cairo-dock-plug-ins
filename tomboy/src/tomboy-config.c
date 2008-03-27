#include <string.h>

#include "tomboy-struct.h"
#include "tomboy-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cIconDefault 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cIconClose			= CD_CONFIG_GET_STRING ("Configuration", "close icon");
	myConfig.cIconBroken 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.cIconDefault);
	myConfig.cIconDefault = NULL;
	g_free (myConfig.cIconClose);
	myConfig.cIconClose = NULL;
	g_free (myConfig.cIconBroken);
	myConfig.cIconBroken = NULL;
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (myData.pSurfaceDefault);
	myData.pSurfaceDefault = NULL;
	cairo_surface_destroy (myData.pSurfaceClose);
	myData.pSurfaceClose = NULL;
	cairo_surface_destroy (myData.pSurfaceBroken);
	myData.pSurfaceBroken = NULL;
CD_APPLET_RESET_DATA_END
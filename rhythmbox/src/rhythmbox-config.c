#include <string.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_CONFIG_BEGIN ("Rhythmbox", NULL)
	reset_config ();
	
	myConfig.enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.enableCover 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	myConfig.changeAnimation 	= CD_CONFIG_GET_ANIMATION_WITH_DEFAULT ("Configuration", "change_animation", CAIRO_DOCK_ROTATE);
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	cairo_surface_destroy (myData.pSurface);
	myData.pSurface = NULL;
	cairo_surface_destroy (myData.pPlaySurface);
	myData.pPlaySurface = NULL;
	cairo_surface_destroy (myData.pPauseSurface);
	myData.pPauseSurface = NULL;
	cairo_surface_destroy (myData.pCover);
	myData.pCover = NULL;
	cairo_surface_destroy (myData.pBrokenSurface);
	myData.pBrokenSurface = NULL;
	
	g_free (myData.playing_uri);
	myData.playing_uri = NULL;
	
	memset (&myData, 0, sizeof (AppletData));
}

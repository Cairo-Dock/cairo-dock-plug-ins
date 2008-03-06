
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cPlayer = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "current-player", MY_XMMS);
	
	myConfig.cDefaultIcon 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cPlayIcon 			= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cPauseIcon 		= CD_CONFIG_GET_STRING ("Configuration", "pauseicon");
	myConfig.cStopIcon 		= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cBrokenIcon 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_CONFIG_END


void reset_config (void) {
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.cDefaultIcon);
	myConfig.cDefaultIcon = NULL;
	g_free (myConfig.cPlayIcon);
	myConfig.cPlayIcon = NULL;
	g_free (myConfig.cPauseIcon);
	myConfig.cPauseIcon = NULL;
	g_free (myConfig.cStopIcon);
	myConfig.cStopIcon = NULL;
	g_free (myConfig.cBrokenIcon);
	myConfig.cBrokenIcon = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void) {
	
	cairo_surface_destroy (myData.pSurface);
	myData.pSurface = NULL;
	cairo_surface_destroy (myData.pStopSurface);
	myData.pStopSurface = NULL;
	cairo_surface_destroy (myData.pPlaySurface);
	myData.pPlaySurface = NULL;
	cairo_surface_destroy (myData.pPauseSurface);
	myData.pPauseSurface = NULL;
	cairo_surface_destroy (myData.pBrokenSurface);
	myData.pBrokenSurface = NULL;
	
	myData.playingTitle = NULL;
	
	memset (&myData, 0, sizeof (AppletData));
}

#include <string.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.enableCover 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	myConfig.changeAnimation 	= CD_CONFIG_GET_ANIMATION_WITH_DEFAULT ("Configuration", "change_animation", CAIRO_DOCK_ROTATE);
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.cDefaultIcon 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cPlayIcon 			= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cPauseIcon 		= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cStopIcon 		= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cBrokenIcon 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	
	g_free (myConfig.cDefaultIcon);
	g_free (myConfig.cPlayIcon);
	g_free (myConfig.cPauseIcon);
	g_free (myConfig.cStopIcon);
	g_free (myConfig.cBrokenIcon);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (myData.pSurface);
	cairo_surface_destroy (myData.pStopSurface);
	cairo_surface_destroy (myData.pPlaySurface);
	cairo_surface_destroy (myData.pPauseSurface);
	cairo_surface_destroy (myData.pBrokenSurface);
	
	cairo_surface_destroy (myData.pCover);
	
	g_free (myData.playing_uri);
CD_APPLET_RESET_DATA_END

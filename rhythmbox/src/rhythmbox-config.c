#include <string.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

#include "3dcover-draw.h"


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle 		= CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.enableCover 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	myConfig.changeAnimation 	= CD_CONFIG_GET_STRING ("Configuration", "change animation");
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	myConfig.bStealTaskBarIcon = CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	
	myConfig.cUserImage[PLAYER_NONE] 			= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[PLAYER_PLAYING] 		= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cUserImage[PLAYER_PAUSED] 		= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cUserImage[PLAYER_STOPPED] 		= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cUserImage[PLAYER_BROKEN] 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	
	//myConfig.extendedDesklet = CD_CONFIG_GET_BOOLEAN ("Configuration", "3D desklet");
	
	myConfig.bOpenglThemes = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_opengl_themes");
	if (myConfig.bOpenglThemes)
	{
		//\_______________ On on recupere le theme choisi.	
		myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "cd_box_3d");
	}
	
	//myData.iLastFileSize = 9999;
	//myData.bLoopForMagnatuneDone = FALSE;
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.changeAnimation);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++)
		g_free (myConfig.cUserImage[i]);
	
	g_free (myConfig.cThemePath);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
			
	g_free (myData.playing_uri);
	g_free (myData.playing_artist);
	g_free (myData.playing_album);
	g_free (myData.playing_title);
	g_free (myData.playing_cover);
	g_free (myData.previous_cover);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++)
	{
		if (myData.pSurfaces[i] != NULL)
			cairo_surface_destroy (myData.pSurfaces[i]);
	}
	
	cairo_surface_destroy (myData.pCover);
	
	cd_opengl_reset_opengl_datas (myApplet);
	
CD_APPLET_RESET_DATA_END

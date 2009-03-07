/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-musicplayer.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.pQuickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.cDefaultTitle			= CD_CONFIG_GET_STRING ("Icon", "name");
	if (strcmp (myConfig.cDefaultTitle, "__Player__") == 0) {
		g_free (myConfig.cDefaultTitle);
		cd_debug ("MP: default title as name of controled player");
		myConfig.cDefaultTitle		=	CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "current-player", "XMMS");
	}
	
	myConfig.cMusicPlayer 			= CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "current-player", "XMMS");
	myConfig.iExtendedMode			= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "extended_mode", MY_DESKLET_SIMPLE);
	
	myConfig.bEnableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.fTimeDialogs 			= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	
	myConfig.bEnableAnim 				= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_anim");
	myConfig.cChangeAnimation 	= CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "change_animation", "wobbly");
	myConfig.bEnableCover				= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.bStealTaskBarIcon 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	myConfig.bIconBubble 				= CD_CONFIG_GET_BOOLEAN ("Configuration", "bubble icon");
	
	myConfig.cUserImage[PLAYER_NONE] 			= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[PLAYER_PLAYING] 	= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cUserImage[PLAYER_PAUSED] 		= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cUserImage[PLAYER_STOPPED] 	= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cUserImage[PLAYER_BROKEN] 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	
	myConfig.iDecoration 		  = CD_CONFIG_GET_INTEGER ("Configuration", "desklet decoration");
	myConfig.extendedDesklet  = (myConfig.iDecoration == MY_APPLET_EXTENDED);
	if (myConfig.iDecoration == MY_APPLET_PERSONNAL) {
		myConfig.fFrameAlpha		= CD_CONFIG_GET_DOUBLE ("Configuration", "frame alpha");
		myConfig.cFrameImage 	  = CD_CONFIG_GET_FILE_PATH ("Configuration", "frame", NULL);
		myConfig.fReflectAlpha	= CD_CONFIG_GET_DOUBLE ("Configuration", "reflect alpha");
		myConfig.cReflectImage 	= CD_CONFIG_GET_FILE_PATH ("Configuration", "reflect", NULL);
		myConfig.iLeftOffset		= CD_CONFIG_GET_INTEGER ("Configuration", "left offset");
		myConfig.iTopOffset		  = CD_CONFIG_GET_INTEGER ("Configuration", "top offset");
		myConfig.iRightOffset		= CD_CONFIG_GET_INTEGER ("Configuration", "right offset");
		myConfig.iBottomOffset	= CD_CONFIG_GET_INTEGER ("Configuration", "bottom offset");
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cDefaultTitle);
	myConfig.cDefaultTitle = NULL;
	//g_free (myConfig.cMusicPlayer);
	myConfig.cMusicPlayer = NULL;
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		g_free (myConfig.cUserImage[i]);
		myConfig.cUserImage[i] = NULL;
	}
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	if (myData.pCover != NULL) {
		cairo_surface_destroy (myData.pCover);
		myData.pCover = NULL;
	}
	
	g_free (myData.cRawTitle);
	myData.cRawTitle = NULL;
	g_free (myData.cTitle);
	myData.cTitle = NULL;
	g_free (myData.cArtist);
	myData.cArtist = NULL;
	g_free (myData.cAlbum);
	myData.cAlbum = NULL;
	g_free (myData.cCoverPath);
	myData.cCoverPath = NULL;
	
	myData.cPreviousCoverPath = NULL;
	if( myData.cPreviousRawTitle )
		g_free (myData.cPreviousRawTitle);
	myData.cPreviousRawTitle = NULL;
	myData.cQuickInfo = NULL;
	
	//On s'occupe des handelers.
	g_list_foreach (myData.pHandelers, (GFunc) cd_musicplayer_free_handeler, NULL);
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
CD_APPLET_RESET_DATA_END

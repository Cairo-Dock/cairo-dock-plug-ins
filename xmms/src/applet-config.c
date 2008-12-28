/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.defaultTitle			= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iPlayer 					= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "current-player", MY_XMMS);
	myConfig.iExtendedMode		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "extended_mode", MY_DESKLET_INFO);
	
	myConfig.enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.timeDialogs 			= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	myConfig.extendedDesklet	= CD_CONFIG_GET_BOOLEAN ("Configuration", "extended_desklet");
	
	myConfig.enableAnim 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_anim");
	myConfig.changeAnimation 	= CD_CONFIG_GET_STRING ("Configuration", "change animation");
	
	myConfig.bStealTaskBarIcon 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	myConfig.bIconBubble 				= CD_CONFIG_GET_BOOLEAN ("Configuration", "bubble icon");
	
	myConfig.cUserImage[PLAYER_NONE] 			= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[PLAYER_PLAYING] 	= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cUserImage[PLAYER_PAUSED] 		= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cUserImage[PLAYER_STOPPED] 	= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cUserImage[PLAYER_BROKEN] 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.changeAnimation);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++)
		g_free (myConfig.cUserImage[i]);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		if (myData.pSurfaces[i] != NULL)
			cairo_surface_destroy (myData.pSurfaces[i]);
	}
	
	g_free (myData.playingTitle);
	myData.playingTitle = NULL;
	g_free (myData.cQuickInfo);
	myData.cQuickInfo = NULL;
CD_APPLET_RESET_DATA_END

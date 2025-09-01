/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-musicplayer.h"
#include "3dcover-draw.h"

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iQuickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.cMusicPlayer 			= CD_CONFIG_GET_STRING ("Configuration", "current-player");  // NULL by default
	myConfig.cLastKnownDesktopFile 	= CD_CONFIG_GET_STRING ("Configuration", "desktop-entry");  // see applet-struct.h
	myConfig.cDefaultTitle			= CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.bEnableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.iDialogDuration 		= 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "time_dialog", 4);
	
	myConfig.cChangeAnimation 		= CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "change_animation", "wobbly");
	myConfig.bEnableCover			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.bOpenglThemes 			= g_bUseOpenGL && CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_opengl_themes");
	myConfig.bStealTaskBarIcon 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	
	myConfig.cUserImage[PLAYER_NONE] 	= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[PLAYER_PLAYING] = CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cUserImage[PLAYER_PAUSED] 	= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cUserImage[PLAYER_STOPPED] = CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cUserImage[PLAYER_BROKEN] 	= CD_CONFIG_GET_STRING ("Configuration", "broken icon");

	myConfig.bDownload   = CD_CONFIG_GET_BOOLEAN ("Configuration", "DOWNLOAD");
	myConfig.bPauseOnClick = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "pause on click", 1) == 0);  // c'est une liste numerotee de 2 elements.
	if (!myConfig.bPauseOnClick)  // pour pouvoir agir sur la fenetre, il faut voler l'appli (plus tellement vrai avec MPRIS2...).
		myConfig.bStealTaskBarIcon = TRUE;
	myConfig.bNextPrevOnScroll = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "scrolling", 0) == 0);  // c'est une liste numerotee de 2 elements.
	
	//\_______________ On on recupere le theme choisi.
	if (myConfig.bOpenglThemes)
		myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "cd_box_3d");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN

	g_free (myConfig.cDefaultTitle);
	g_free (myConfig.cMusicPlayer);
	g_free (myConfig.cLastKnownDesktopFile);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		g_free (myConfig.cUserImage[i]);
	}
	
	g_free (myConfig.cThemePath);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		if (myData.pSurfaces[i] != NULL)
			cairo_surface_destroy (myData.pSurfaces[i]);
	}
	
	if (myData.pCover != NULL)
		cairo_surface_destroy (myData.pCover);
	
	g_free (myData.cRawTitle);
	g_free (myData.cTitle);
	g_free (myData.cArtist);
	g_free (myData.cAlbum);
	g_free (myData.cCoverPath);
	g_free (myData.cPreviousCoverPath);
	g_free (myData.cPreviousRawTitle);
	
	// On s'occupe des handlers.
	g_list_free_full (myData.pHandlers, cd_musicplayer_free_handler);
	
	// Bye bye pauvres textures opengl
	cd_opengl_reset_opengl_datas (myApplet);
	
CD_APPLET_RESET_DATA_END

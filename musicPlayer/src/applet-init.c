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

#include <stdlib.h>
#include <glib/gstdio.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"
#include "3dcover-draw.h"

#include "applet-mpris2.h"

CD_APPLET_DEFINE2_BEGIN (N_("musicPlayer"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY,
	N_("This applet lets you control any music player.\n"
	"First choose the player you want to control.\n"
	"<b>click</b> to show/hide the player or pause,\n"
	"<b>middle-click</b> to pause or go to next song,\n"
	"<b>Scroll up/down</b> to change the song or control the volume.\n"
	"You can drag and drop songs on the icon to put them in the queue (depends on Player),\n"
	" and jpeg image to use as cover.\n"
	"Note: you may have to install or activate the MPRIS plug-in of the player."),
	"ChanGFu (Rémy Robertson), Mav (Yann SLADEK), Tofe, Jackass, Nochka85, Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
	CD_APPLET_ACT_AS_LAUNCHER
CD_APPLET_DEFINE2_END


static void _set_handler_from_config (void)
{
	const gchar *cName = myConfig.cMusicPlayer;
	const gchar *cID = myConfig.cLastKnownDesktopFile;
	const gchar *cMpris = myConfig.cMpris2Name;
	
	if (cName && !(cMpris && cID))
	{
		const CDKnownMusicPlayer *player = cd_musicplayer_find_known_player (cName);
		if (player)
		{
			if (!cMpris) cMpris = player->mpris2;
			if (!cID) cID = player->id;
		}
	}
	
	if (!cMpris) return; // it is no use setting a player that will not be detected
	
	cd_musicplayer_set_current_handler (cMpris, cName, cID, FALSE, TRUE);
}


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	// Register the players -- for now, we only support MPRIS2, so
	// we don't need to register a custom handler for each player.
	cd_musicplayer_register_mpris2_handler();
	
	gchar *cCoverPath = g_strdup_printf ("%s/musicplayer", g_cCairoDockDataDir);
	if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
	{
		if (g_mkdir (cCoverPath, 7*8*8+7*8+5) != 0)
			cd_warning ("couldn't create directory %s to download covers", cCoverPath);
	}
	g_free (cCoverPath);
	
	
	//\_______________ on definit un mode de rendu pour notre desklet.
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	else if (myIcon->cName == NULL || *myIcon->cName == '\0')
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cMusicPlayer ? myConfig.cMusicPlayer :
			myApplet->pModule->pVisitCard->cTitle);
	}
	
	cairo_dock_set_icon_ignore_quicklist (myIcon);  // ignore additional actions in the menu, as the applet already adds the actions supported by any player.
	
	//\_______________ on charge le theme 3D si necessaire.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
		myConfig.bOpenglThemes = cd_opengl_load_3D_theme (myApplet, myConfig.cThemePath);
	
	
	//\_______________ on demarre le backend.
	// Pour forcer le dessin initial.
	myData.iPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	_set_handler_from_config ();
	cd_musicplayer_apply_status_surface (PLAYER_NONE);
	
	//\_______________ On s'abonne aux notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
	{
		CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;  // pour les animation de transitions.
		if (myDesklet)  // On ne teste le survol des boutons que si l'applet est detachee
		{
			gldi_object_register_notification (myContainer,
				NOTIFICATION_MOUSE_MOVED,
				(GldiNotificationFunc) cd_opengl_test_mouse_over_buttons,
				GLDI_RUN_AFTER,
				myApplet);
		}
	}
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) cd_opengl_test_mouse_over_buttons,
		myApplet);
	
	// stop and unset the current handler.
	cd_musicplayer_set_current_handler (NULL, NULL, NULL, FALSE, FALSE);
	
	// On stoppe les boucles de recup de la pochette.
	if (myData.iSidCheckCover != 0)
		g_source_remove (myData.iSidCheckCover);
	gldi_task_free (myData.pCoverTask);
	
	// cancel any pending DBus call
	if (myData.pCancelMain)
	{
		g_cancellable_cancel (myData.pCancelMain);
		g_object_unref (G_OBJECT (myData.pCancelMain));
	}
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On reset surfaces et textures.
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	cd_opengl_reset_opengl_datas (myApplet);
	
	//\_______________ On recharge entierement le theme 3D.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) cd_opengl_test_mouse_over_buttons,
			myApplet);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
		{
			CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
			if (myDesklet)  // On ne teste le survol des boutons que si l'applet est detachee
				gldi_object_register_notification (myContainer,
					NOTIFICATION_MOUSE_MOVED,
					(GldiNotificationFunc) cd_opengl_test_mouse_over_buttons,
					GLDI_RUN_AFTER,
					myApplet);
		}
	}
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
	{
		myConfig.bOpenglThemes = cd_opengl_load_3D_theme (myApplet, myConfig.cThemePath);
	}
	
	
	//\_______________ On force le redessin.
	//myData.iPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	if( myData.cPreviousRawTitle )
	{
		g_free (myData.cPreviousRawTitle);
		myData.cPreviousRawTitle = NULL;
	}
	if( myData.cPreviousCoverPath )
	{
		g_free (myData.cPreviousCoverPath);
		myData.cPreviousCoverPath = NULL;
	}
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	myData.cover_exist = FALSE;
	myData.iCurrentFileSize = 0;
	myData.iGetTimeFailed = 0;
	
	//\_______________ On gere le changement de player ou on redessine juste l'icone.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// on stoppe l'ancien backend et on relance le nouveau.
		cd_musicplayer_stop_current_handler (TRUE);  // libere tout ce qu'occupe notre ancien handler.

		CD_APPLET_MANAGE_APPLICATION (NULL);
		
		_set_handler_from_config ();
	}
	else  // on redessine juste l'icone.
	{
		if (myConfig.bEnableCover && myData.cover_exist && myData.cCoverPath != NULL)  // cover is available
		{
			cd_musiplayer_apply_cover ();
		}
		else  // no cover -> set the status surface.
		{
			cd_musicplayer_apply_status_surface (myData.iPlayingStatus);
		}
	}
CD_APPLET_RELOAD_END

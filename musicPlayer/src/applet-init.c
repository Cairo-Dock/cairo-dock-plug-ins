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
#include "applet-dbus.h" 
#include "3dcover-draw.h"

#include "applet-xmms.h" //Support XMMS
#include "applet-xmms2.h" //Support XMMS2
#include "applet-exaile.h" //Support Exaile
#include "applet-exaile3.h" //Support Exaile 0.3
#include "applet-songbird.h" //Support Songbird
#include "applet-banshee.h" //Support Banshee
#include "applet-rhythmbox.h" //Support Rhythmbox
#include "applet-qmmp.h" //Support Qmmp
#include "applet-guayadeque.h" //Support Guayadeque
#include "applet-quodlibet.h" //Support QuodLibet
#include "applet-listen.h" //Support Listen
#include "applet-amarok2.h" //Support Amarok 2
#include "applet-audacious.h" //Support Audacious
#include "applet-clementine.h" // Support Clementine

CD_APPLET_DEFINITION (N_("musicPlayer"),
	2,0,0,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("This applet lets you control any music player.\n"
	"Left click to Play/Pause, middle-click to play Next song.\n"
	"Scroll up/down to play previous/next song.\n"
	"You can drag and drop songs on the icon to put them in the queue (depends on Player),\n"
	" and jpeg image to use as cover.\n"
	"Note : For XMMS, you have to install the 'xmms-infopipe' plug-in.\n"
	"       For SongBird, you have to install its dbus add-on.\n"
	"       For Exaile 0.3, you have to activate its MPRIS plug-in"),
	"ChanGFu (Rémy Robertson), Mav (Yann SLADEK), Tofe, Jackass, Nochka85, Fabounet")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	// Add here all player's registering functions
	// Don't forget to add the registered Name in ../data/musicPlayer.conf.in
	cd_musicplayer_register_xmms_handler ();
	cd_musicplayer_register_exaile_handler();
	cd_musicplayer_register_exaile3_handler();
	cd_musicplayer_register_songbird_handler();
	cd_musicplayer_register_banshee_handler();
	cd_musicplayer_register_rhythmbox_handler();
	cd_musicplayer_register_qmmp_handler();
	cd_musicplayer_register_quodlibet_handler();
	cd_musicplayer_register_listen_handler();
	cd_musicplayer_register_amarok2_handler();
	cd_musicplayer_register_xmms2_handler();
	cd_musicplayer_register_audacious_handler();
	cd_musicplayer_register_guayadeque_handler();
	cd_musicplayer_register_clementine_handler();
	
	gchar *cCoverPath = g_strdup_printf ("%s/musicplayer", g_cCairoDockDataDir);
	if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
	{
		if (g_mkdir (cCoverPath, 7*8*8+7*8+5) != 0)
			cd_warning ("couldn't create directory %s", cCoverPath);
	}
	g_free (cCoverPath);
	
	
	//\_______________ on definit un mode de rendu pour notre desklet.
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	else if (myIcon->cName == NULL || *myIcon->cName == '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cMusicPlayer);
	
	
	//\_______________ on charge le theme 3D si necessaire.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
		myConfig.bOpenglThemes = cd_opengl_load_3D_theme (myApplet, myConfig.cThemePath);
	
	
	//\_______________ on demarre le backend.
	// Pour forcer le dessin initial.
	myData.iPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	myData.pCurrentHandeler = cd_musicplayer_get_handler_by_name (myConfig.cMusicPlayer);
	if (myData.pCurrentHandeler == NULL) {
 		cd_warning ("MP : this player (%s) is not supported.", myConfig.cMusicPlayer); 
 		return; 
	}
	
	cd_musicplayer_launch_handler ();  // connexion au bus, detection de l'appli, recuperation de l'etat du lecteur si il est en marche, sinon dessin de l'icone "eteint".
	
	//\_______________ On prend en charge l'icone de l'appli player.
	if (myConfig.bStealTaskBarIcon)
		CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandeler->appclass);
	
	//\_______________ On s'abonne aux notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
	{
		CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;  // pour les animation de transitions.
		if (myDesklet)  // On ne teste le survol des boutons que si l'applet est détachée
		{
			cairo_dock_register_notification_on_container (myContainer,
				CAIRO_DOCK_MOUSE_MOVED,
				(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
				CAIRO_DOCK_RUN_AFTER,
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
	cairo_dock_remove_notification_func_on_container (myContainer,
		CAIRO_DOCK_MOUSE_MOVED,
		(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
		myApplet);
	
	// On stoppe les boucles de recup de la pochette.
	if (myData.iSidCheckXmlFile != 0)
		g_source_remove (myData.iSidCheckXmlFile);
	if (myData.iSidCheckCover != 0)
		g_source_remove (myData.iSidCheckCover);
	if (myData.iSidGetCoverInfoTwice != 0)
		g_source_remove (myData.iSidGetCoverInfoTwice);
	
	// on libere la classe.
	CD_APPLET_MANAGE_APPLICATION (NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	else if (myIcon->cName == NULL || *myIcon->cName == '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cMusicPlayer);
	
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
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		cairo_dock_remove_notification_func_on_container (CD_APPLET_MY_OLD_CONTAINER,
			CAIRO_DOCK_MOUSE_MOVED,
			(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
			myApplet);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
		{
			CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
			if (myDesklet)  // On ne teste le survol des boutons que si l'applet est détachée
				cairo_dock_register_notification_on_container (myContainer,
					CAIRO_DOCK_MOUSE_MOVED,
					(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
					CAIRO_DOCK_RUN_AFTER,
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
	myData.bCoverNeedsTest = FALSE;
	myData.iNbCheckFile = 0;
	myData.iCurrentFileSize = 0;
	myData.iGetTimeFailed = 0;
	
	//\_______________ On gere le changement de player ou on redessine juste l'icone.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		// on stoppe l'ancien backend et on relance le nouveau.
		if (myData.pCurrentHandeler)
		{
			cd_musicplayer_stop_handler ();  // libère tout ce qu'occupe notre ancien handler.
			CD_APPLET_MANAGE_APPLICATION (NULL);
		}
		myData.pCurrentHandeler = cd_musicplayer_get_handler_by_name (myConfig.cMusicPlayer);
		if (myData.pCurrentHandeler == NULL)
		{ 
 			cd_warning ("MP : this player (%s) is not supported.", myConfig.cMusicPlayer); 
 			return FALSE; 
		}
		cd_musicplayer_launch_handler ();
		
		if (myConfig.bStealTaskBarIcon)
			CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandeler->appclass);
	}
	else { // on redessine juste l'icone.
		cd_musicplayer_update_icon (FALSE);  // FALSE pour ne pas avoir 2 fois le dialogue.
		if (! myData.cover_exist)
		{
			if(myData.iPlayingStatus == PLAYER_PLAYING)
			{
				cd_musicplayer_set_surface (PLAYER_PLAYING);
			}
			else
			{
				cd_musicplayer_set_surface (PLAYER_PAUSED);
			}
		}
	}
CD_APPLET_RELOAD_END

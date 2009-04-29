/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"
#include "applet-dbus.h" 
#include "3dcover-draw.h"

#include "applet-xmms.h" //Support XMMS
#include "applet-exaile.h" //Support Exaile
#include "applet-songbird.h" //Support Songbird
#include "applet-banshee.h" //Support Banshee
#include "applet-rhythmbox.h" //Support Rhythmbox
#include "applet-quodlibet.h" //Support QuodLibet
#include "applet-listen.h" //Support Listen
#include "applet-amarok2.h" //Support Amarok 2
#include "applet-amarok1.h" //Support Amarok 1.4

CD_APPLET_DEFINITION (N_("musicPlayer"),
	2,0,0,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("This applet lets you control any music player.\n"
	"Left click to Play/Pause, middle-click to play Next song.\n"
	"Scroll up/down to play previous/next song.\n"
	"You can drag and drop songs to put them in the queue (depends on Player).\n"
	"Note : For XMMS, you have to install the 'xmms-infopipe' plug-in.\n"
	"       For SongBird, you have to install its dbus add-on.\n"),
	"ChanGFu (Rémy Robertson), Mav (Yann SLADEK), Tofe, Jackass, Nochka85")


static void _musicplayer_set_simple_renderer (void) {
	cd_debug ("");
	const gchar *cConfigName = NULL;
	switch (myConfig.iDecoration) {
		case MY_APPLET_PERSONNAL :
		break ;
		case MY_APPLET_CD_BOX:
			cConfigName = "CD box";
		break ;
		case MY_APPLET_FRAME_REFLECTS :
			cConfigName = "frame&reflects";
		break ;
		case MY_APPLET_SCOTCH :
				cConfigName = "scotch";
		break ;
		case MY_APPLET_FRAME_SCOTCH :
				cConfigName = "frame with scotch";
		break ;
		default :
			return ;
	}
	if (cConfigName != NULL) {
		CairoDeskletRendererConfigPtr pConfig = cairo_dock_get_desklet_renderer_predefined_config ("Simple", cConfigName);
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
	}
	else if (myConfig.cFrameImage != NULL || myConfig.cReflectImage != NULL) {
		gpointer pManualConfig[9] = {myConfig.cFrameImage, myConfig.cReflectImage, GINT_TO_POINTER (CAIRO_DOCK_FILL_SPACE), &myConfig.fFrameAlpha, &myConfig.fReflectAlpha, GINT_TO_POINTER (myConfig.iLeftOffset), GINT_TO_POINTER (myConfig.iTopOffset), GINT_TO_POINTER (myConfig.iRightOffset), GINT_TO_POINTER (myConfig.iBottomOffset)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pManualConfig);
	}
	else {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	/*Add here all player's registering functions
	Dont forget to add the registered Name in ../data/musicPlayer.conf.in*/
	
	cd_musicplayer_register_xmms_handeler ();
	cd_musicplayer_register_exaile_handeler();
	cd_musicplayer_register_songbird_handeler();
	cd_musicplayer_register_banshee_handeler();
	cd_musicplayer_register_rhythmbox_handeler();
	cd_musicplayer_register_quodlibet_handeler();
	cd_musicplayer_register_listen_handeler();
	cd_musicplayer_register_amarok2_handeler();
	cd_musicplayer_register_amarok1_handeler();
	
	
	if (myDesklet) {
		cd_musicplayer_add_buttons_to_desklet ();
		if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
			gpointer data[3] = {" ", " ", GINT_TO_POINTER (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
		}
		else if (myConfig.iExtendedMode == MY_DESKLET_CAROUSSEL || myConfig.iExtendedMode == MY_DESKLET_CONTROLER) {
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			_musicplayer_set_simple_renderer ();
		}
	}
		
	//On initialise les variables pour la connexion aux proxys
	myData.dbus_enable = 0;
	myData.opening= 0;
	
	myData.pPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	if( myData.cPreviousRawTitle )
		g_free (myData.cPreviousRawTitle);
	myData.cPreviousRawTitle = NULL;
	if( myData.cPreviousCoverPath )
		g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name (myConfig.cMusicPlayer);

	if (myData.pCurrentHandeler == NULL) { 
 		cd_warning ("MP : Handeler pointer is null, halt."); 
 		return; 
	}
	
	cd_musicplayer_arm_handeler (); //On prépare notre handeler
	
	if (myConfig.bStealTaskBarIcon) {
	  cd_debug ("MP: inhibate %s (1)", myData.pCurrentHandeler->appclass);
		cairo_dock_inhibate_class (myData.pCurrentHandeler->appclass, myIcon);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.iExtendedMode == MY_DESKLET_OPENGL) { //On evite de surcharger en travail inutile
		cd_opengl_init_opengl_datas ();
		cairo_dock_launch_animation (myContainer);
		cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) cd_opengl_test_update_icon_slow, CAIRO_DOCK_RUN_FIRST, myApplet);
	}
	
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class (myData.pCurrentHandeler->appclass, myIcon);
		
	/* On libere la memoire prise par notre handeler */	
	cd_musicplayer_free_handeler(myData.pCurrentHandeler);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && myDesklet) { //Les icônes ont pu changer, reload.
		if (myConfig.iExtendedMode == MY_DESKLET_SIMPLE && myDesklet->icons != NULL) {
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		else
			cd_musicplayer_add_buttons_to_desklet ();
		
		//Chargement du bon moteur de rendu pour le desklet, il a pu changer
		if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
			gpointer data[3] = {" ", " ", GINT_TO_POINTER (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
		}
		else if (myConfig.iExtendedMode == MY_DESKLET_CAROUSSEL || myConfig.iExtendedMode == MY_DESKLET_CONTROLER) {
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			_musicplayer_set_simple_renderer ();
		}
	}
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	myData.pPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	if( myData.cPreviousRawTitle )
		g_free (myData.cPreviousRawTitle);
	myData.cPreviousRawTitle = NULL;
	if( myData.cPreviousCoverPath )
		g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	//On relance la connexion aux proxys
	myData.dbus_enable = 0;
	myData.opening = 0;
	
	//On vire nos notification opengl si necessaire
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) cd_opengl_test_update_icon_slow, myApplet); //CF: A voir si ca ne crash pas.
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cd_musicplayer_disarm_handeler (); //On libère tout ce qu'occupe notre ancien handeler.
		myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name (myConfig.cMusicPlayer);
		if( myData.pCurrentHandeler ) {
			if (myIcon->cClass != NULL && myData.pCurrentHandeler->appclass != NULL) { //Sécurité pour ne pas planter a cause du strcmp
				cd_debug ("MP: deinhibate %s (1)", myIcon->cClass);
				if ((!myConfig.bStealTaskBarIcon) || (strcmp (myIcon->cClass, myData.pCurrentHandeler->appclass))) 
				  cairo_dock_deinhibate_class (myIcon->cClass, myIcon);
			}
			else if (myIcon->cClass != NULL && !myConfig.bStealTaskBarIcon) { // on ne veut plus l'inhiber ou on veut inhiber une autre.
			  cd_debug ("MP: deinhibate %s (2)", myIcon->cClass);
			  cairo_dock_deinhibate_class (myIcon->cClass, myIcon);
			}
			  
			if (myConfig.bStealTaskBarIcon && myIcon->cClass == NULL) { // on commence a inhiber l'appli si on ne le faisait pas, ou qu'on s'est arrete.
				cd_debug ("MP: inhibate %s (2)", myData.pCurrentHandeler->appclass);
				cairo_dock_inhibate_class (myData.pCurrentHandeler->appclass, myIcon);
			}
			
			cd_musicplayer_arm_handeler (); //et on arme le bon.
			
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.iExtendedMode == MY_DESKLET_OPENGL) { //CF: Attention a l'intégration, il faut cloisonner chaque options au risque de faire travailler le dock inutilement
				cd_opengl_init_opengl_datas ();		
				cairo_dock_launch_animation (myContainer);
				cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) cd_opengl_test_update_icon_slow, CAIRO_DOCK_RUN_AFTER, myApplet);
			}
		}
	}
	else { // on redessine juste l'icone.
		cd_musicplayer_draw_icon ();
	}
CD_APPLET_RELOAD_END

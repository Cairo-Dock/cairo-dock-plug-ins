/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"

#include "applet-xmms.h" //Support XMMS

CD_APPLET_DEFINITION ("musicPlayer", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

/*Servira peut-être
CD_APPLET_PRE_INIT_BEGIN ("musicPlayer", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)
	
	
CD_APPLET_PRE_INIT_END*/


static void _musciplayer_set_simple_renderer (void) {
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
CD_APPLET_INIT_BEGIN (erreur)
	/*Add here all player's registering functions
	Dont forget to add the registered Name in ../data/musicPlayer.conf.in*/
	cd_musicplayer_register_xmms_handeler ();
	
	if (myDesklet) {
		cd_musicplayer_add_buttons_to_desklet ();
		if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
			gpointer data[3] = {" ", " ", (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
		}
		else if (myConfig.iExtendedMode == MY_DESKLET_CAROUSSEL || myConfig.iExtendedMode == MY_DESKLET_CONTROLER) {
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			_musciplayer_set_simple_renderer ();
		}
	}
	
	myData.pPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	myData.cPreviousRawTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name (myConfig.cMusicPlayer);
	/*myConfig.cMusicPlayer = "XMMS";
	myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name ("XMMS");*/
	cd_musicplayer_arm_handeler (); //On prépare notre handeler
	
	if (myConfig.bStealTaskBarIcon) {
		cairo_dock_inhibate_class (myData.pCurrentHandeler->appclass, myIcon);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class (myData.pCurrentHandeler->appclass, myIcon);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && myDesklet) {
		if (myConfig.iExtendedMode == MY_DESKLET_SIMPLE && myDesklet->icons != NULL) {
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		else
			cd_musicplayer_add_buttons_to_desklet ();
	}
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	if (myDesklet) {
		if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
			gpointer data[3] = {" ", " ", (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
		}
		else if (myConfig.iExtendedMode == MY_DESKLET_CAROUSSEL || myConfig.iExtendedMode == MY_DESKLET_CONTROLER) {
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			_musciplayer_set_simple_renderer ();
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	myData.pPlayingStatus = PLAYER_NONE;
	myData.pPreviousPlayingStatus = -1;
	myData.cPreviousRawTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cd_musicplayer_disarm_handeler (); //On libère tout ce qu'occupe notre ancien handeler.
		myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name (myConfig.cMusicPlayer);
		/*myData.pCurrentHandeler = cd_musicplayer_get_handeler_by_name ("XMMS");
		myConfig.cMusicPlayer = "XMMS";*/
		
		if (myIcon->cClass != NULL && (! myConfig.bStealTaskBarIcon || strcmp (myIcon->cClass, myData.pCurrentHandeler->appclass) != 0)) { // on ne veut plus l'inhiber ou on veut inhiber une autre.
			cairo_dock_deinhibate_class (myData.pCurrentHandeler->appclass, myIcon);
		}
		if (myConfig.bStealTaskBarIcon && myIcon->cClass == NULL) { // on comence a inhiber l'appli si on ne le faisait pas, ou qu'on s'est arrete.
			cairo_dock_inhibate_class (myData.pCurrentHandeler->appclass, myIcon);
		}
		
		cd_musicplayer_arm_handeler (); //et on arme le bon.
	}
	else  // on redessine juste l'icone.
		cd_musicplayer_draw_icon ();
CD_APPLET_RELOAD_END

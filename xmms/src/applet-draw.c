/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS

static gchar *s_cIconName[PLAYER_NB_STATUS] = {"xmms.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};

static GList * _list_icons (void) {
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < 4; i ++) {
		pIcon = g_new0 (Icon, 1);
		pIcon->acName = NULL;
		pIcon->acFileName = g_strdup_printf ("%s/%d.svg", MY_APPLET_SHARE_DATA_DIR, i);
		pIcon->fOrder = i;
		pIcon->iType = i;
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->acCommand = g_strdup ("none");
		pIcon->cParentDockName = NULL;
		pIconList = g_list_append (pIconList, pIcon);
	}
	
	return pIconList;
}
void cd_xmms_add_buttons_to_desklet(CairoDockModuleInstance *myApplet) {
	if (myDesklet && myConfig.extendedDesklet){
		GList *pIconList = _list_icons ();
		myDesklet->icons = pIconList;
	}
}

gboolean cd_xmms_draw_icon (CairoDockModuleInstance *myApplet) {
	gboolean bNeedRedraw = FALSE;
	if (myData.playingStatus == PLAYER_NONE) {
		myData.cQuickInfo = NULL;
		if (myData.cQuickInfo != myData.cPreviousQuickInfo) {
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			myData.cPreviousQuickInfo = myData.cQuickInfo;
		}
	}
	else {
		switch (myConfig.quickInfoType) {
			case MY_APPLET_NOTHING :
				myData.cQuickInfo = NULL;
				if (myData.cQuickInfo != myData.cPreviousQuickInfo) {
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
					myData.cPreviousQuickInfo = myData.cQuickInfo;
				}
			break ;
			
			case MY_APPLET_TIME_ELAPSED :
				myData.cQuickInfo = "elapsed";
				myData.cPreviousQuickInfo = myData.cQuickInfo;
				if (myData.iCurrentTime != myData.iPreviousCurrentTime) {
					myData.iPreviousCurrentTime = myData.iCurrentTime;
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime);
					bNeedRedraw = TRUE;
				}
			break ;
			
			case MY_APPLET_TIME_LEFT :
				myData.cQuickInfo = "left";
				myData.cPreviousQuickInfo = myData.cQuickInfo;
				if (myData.iCurrentTime != myData.iPreviousCurrentTime) {
					myData.iPreviousCurrentTime = myData.iCurrentTime;
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime - myData.iSongLength);
					bNeedRedraw = TRUE;
				}
			break ;
			
			case MY_APPLET_TRACK :
				myData.cQuickInfo = "track";
				myData.cPreviousQuickInfo = myData.cQuickInfo;
				if (myData.iTrackNumber != myData.iPreviousTrackNumber) {
					myData.iPreviousTrackNumber = myData.iTrackNumber;
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF("%d", myData.iTrackNumber);
					bNeedRedraw = TRUE;
				}
			break ;
			
			default :
			break;
		}
	}
	
	//cd_message("Previous: %s\nNow: %s", myData.previousPlayingTitle, myData.playingTitle);
	if (myData.previousPlayingTitle != myData.playingTitle) {
	  myData.previousPlayingTitle = myData.playingTitle;
		if (myData.playingTitle == NULL || strcmp (myData.playingTitle, "(null)") == 0) {
			CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		}
		else {
		  cd_message("Changing title to: %s", myData.playingTitle);
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.playingTitle);
			if (myConfig.enableAnim) {
		    cd_message("Animating for: %s", myData.playingTitle);
			  cd_xmms_animate_icon(myApplet, 1);
		  }
		  if (myConfig.enableDialogs) {
			  cd_xmms_new_song_playing(myApplet);
		  }
		}
	}
	
	if (myData.playingStatus != myData.previousPlayingStatus) {  // changement de statut.
		cd_message ("PlayingStatus : %d -> %d\n", myData.previousPlayingStatus, myData.playingStatus);
		myData.previousPlayingStatus = myData.playingStatus;
		cd_xmms_set_surface (myApplet, myData.playingStatus);
		if (myData.playingStatus == 0) {
		  myData.playingTitle = NULL; //Rien ne joue
		  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		}
	}
	else if (bNeedRedraw) {
		CD_APPLET_REDRAW_MY_ICON;
	}
	
	return TRUE;
}

//Fonction qui affiche la bulle au changement de musique
//Old function without icon
static void cd_xmms_new_song_playing_old(CairoDockModuleInstance *myApplet) {
	cairo_dock_show_temporary_dialog(myData.playingTitle, myIcon, myContainer, myConfig.timeDialogs);
}

//With Icon.
void cd_xmms_new_song_playing(CairoDockModuleInstance *myApplet) {
	cairo_dock_remove_dialog_if_any (myIcon); //On evite la superposition ?
	if (!myConfig.bIconBubble) {
		cd_xmms_new_song_playing_old(myApplet);
		return;
	}
	
	gchar *cImagePath = NULL;
	if (myConfig.cUserImage[PLAYER_NONE] != NULL)
		cImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[PLAYER_NONE]);
	else
		cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[PLAYER_NONE]);
	
	cairo_dock_show_temporary_dialog_with_icon (myData.playingTitle, myIcon, myContainer, myConfig.timeDialogs, cImagePath);
	g_free(cImagePath);
}

//Fonction qui anime l'icone au changement de musique
void cd_xmms_animate_icon(CairoDockModuleInstance *myApplet, int animationLength) {
	if (myDock) {
		CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength);
	}
}

void cd_xmms_set_surface (CairoDockModuleInstance *myApplet, MyPlayerStatus iStatus) {
	g_return_if_fail (iStatus < PLAYER_NB_STATUS);
	//g_print ("%s (%d)\n", __func__, iStatus);
	cairo_surface_t *pSurface = myData.pSurfaces[iStatus];
	if (pSurface == NULL) {
		if (myConfig.cUserImage[iStatus] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[iStatus]);
	}
	else {
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}

void cd_xmms_change_desklet_data (CairoDockModuleInstance *myApplet) {
	cd_debug ("");
	
	if (myData.playingTitle == NULL)
		return;
	if (myDesklet == NULL || !myConfig.extendedDesklet || (myConfig.iExtendedMode != MY_DESKLET_INFO && myConfig.iExtendedMode != MY_DESKLET_INFO_AND_CONTROLER))
		return;
	
	//On détermine l'artist (par default le 1er avant le tiret)
	gchar **rawTitle=NULL, *artist=NULL, *title=NULL;
	rawTitle = g_strsplit (myData.playingTitle, "-", -1);
	if (rawTitle[0] != NULL)
		artist = rawTitle[0];
	if (rawTitle[1] != NULL) {
		title = strchr (myData.playingTitle, '-');
		title ++;
		while (*title == ' ')
			title ++;
	}
	
	gpointer data[2] = {artist, title};
	cairo_dock_render_desklet_with_new_data (myDesklet, (CairoDeskletRendererDataPtr) data);
	
	/*gpointer data[3] = {artist, title, (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
	CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
	cd_xmms_set_surface (myData.playingStatus);
	gtk_widget_queue_draw (myDesklet->pWidget);*/
	
	g_strfreev (rawTitle);
}

void cd_xmms_player_none (CairoDockModuleInstance *myApplet) {
	cd_debug ("");
	if (myDesklet && myConfig.extendedDesklet && (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER)) {
		gpointer data[2] = {NULL, NULL};
		cairo_dock_render_desklet_with_new_data (myDesklet, (CairoDeskletRendererDataPtr) data);
		
		/*gpointer data[3] = {NULL, NULL, (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
		cd_xmms_set_surface (myData.playingStatus);
		gtk_widget_queue_draw (myDesklet->pWidget);*/
		}
}

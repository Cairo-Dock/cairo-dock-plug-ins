/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"
#include "applet-cover.h"

static const gchar *s_cIconName[PLAYER_NB_STATUS] = {"default.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};

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

void cd_musicplayer_add_buttons_to_desklet(void) {
	if (myDesklet && myConfig.iExtendedMode != MY_DESKLET_SIMPLE){
		GList *pIconList = _list_icons ();
		myDesklet->icons = pIconList;
	}
}

void _set_new_title (void) {
	if( myData.cPreviousRawTitle )
	{
		g_free( myData.cPreviousRawTitle ); myData.cPreviousRawTitle = NULL;
	}
	if( myData.cRawTitle )
	{
		myData.cPreviousRawTitle = g_strdup(myData.cRawTitle);
	}
	if (myData.cRawTitle == NULL || strcmp (myData.cRawTitle, "(null)") == 0) {
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
	}
	else {
		cd_message("MP : Changing title to: %s", myData.cRawTitle);
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cRawTitle);
		if (myConfig.bEnableAnim)
			cd_musicplayer_animate_icon (1);
		if (myConfig.bEnableDialogs)
			cd_musicplayer_new_song_playing ();
	}
}

gboolean cd_musicplayer_draw_icon (void) {
	gboolean bNeedRedraw = FALSE;

	/* Affichage de la Quick Info */
	if (myData.pPlayingStatus == PLAYER_NONE) {
		myData.cQuickInfo = NULL;
		if (myData.cQuickInfo != myData.cPreviousQuickInfo) {
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			myData.cPreviousQuickInfo = myData.cQuickInfo;
		}
	}
	else {
		switch (myConfig.pQuickInfoType) {
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
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iTrackNumber);
					bNeedRedraw = TRUE;
				}
			break ;
			
			default :
			break;
		}
	}
	
	/* Vérifie si le titre a changé */
	if (myData.cPreviousRawTitle != NULL && myData.cRawTitle != NULL) { // Si les titres sont définis...
		if (strcmp (myData.cPreviousRawTitle, myData.cRawTitle)) { // ... et qu'ils sont différents
			_set_new_title ();
			cd_check_musicPlayer_cover_exists(myData.cCoverPath,myData.pCurrentHandeler->iPlayer);
		}
	}
	else if (myData.cRawTitle != NULL) { // Si seulement le titre courant est défini
		_set_new_title ();
	}
	else {
		//Kedal a faire
	}

	/* Affichage de l'icone ou de la pochette et de son emblème */
	if (myData.pPlayingStatus != myData.pPreviousPlayingStatus) {  // changement de statut.
		cd_debug ("MP : PlayingStatus : %d -> %d\n", myData.pPreviousPlayingStatus, myData.pPlayingStatus);
		myData.pPreviousPlayingStatus = myData.pPlayingStatus;
		
		if (!myConfig.bEnableCover)
			cd_musicplayer_set_surface (myData.pPlayingStatus);
		
		if (myData.pPlayingStatus == PLAYER_NONE || myData.pPlayingStatus == PLAYER_BROKEN) {
		  myData.cRawTitle = NULL; //Rien ne joue
		  CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
		}
	}

	/* Affichage de la pochette */	
	if (myConfig.bEnableCover) {
		if (myData.cCoverPath != NULL && g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS)) {
				myData.cPreviousCoverPath = g_strdup(myData.cCoverPath);
			if (myData.cPreviousCoverPath==NULL || (myData.cCoverPath && g_strcasecmp(myData.cCoverPath,myData.cPreviousCoverPath)!=0)) { //On évite de dessiner pour rien
				gchar *cTmpPath = cd_check_musicPlayer_cover_exists(myData.cCoverPath,myData.pCurrentHandeler->iPlayer);
				if (cTmpPath)
					CD_APPLET_SET_IMAGE_ON_MY_ICON (cTmpPath);
				else
					cd_musicplayer_set_surface (0);
				//CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCoverPath);
				myData.cPreviousCoverPath = g_strdup(myData.cCoverPath);
				g_free (cTmpPath);
			}
		}
		else
			cd_musicplayer_set_surface (0); //On affiche l'image par défaut
		
		switch (myData.pPlayingStatus) {
			case PLAYER_PLAYING :
				//cd_debug("MP : Le lecteur est en mode Play");
				CD_APPLET_DRAW_EMBLEM (CAIRO_DOCK_EMBLEM_PLAY, CAIRO_DOCK_EMBLEM_MIDDLE);
				break;
				
			case PLAYER_PAUSED :
				//cd_debug("MP : Le lecteur est en mode Pause");
				CD_APPLET_DRAW_EMBLEM (CAIRO_DOCK_EMBLEM_PAUSE, CAIRO_DOCK_EMBLEM_MIDDLE);
				break;
				
			case PLAYER_STOPPED :
				CD_APPLET_DRAW_EMBLEM (CAIRO_DOCK_EMBLEM_STOP, CAIRO_DOCK_EMBLEM_UPPER_RIGHT);
				break;
			
			default :
				break;	
		}
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	return TRUE;
}

//Fonction qui affiche la bulle au changement de musique
//Old function without icon
void cd_musicplayer_new_song_playing_old (void) {
	cairo_dock_show_temporary_dialog (myData.cRawTitle, myIcon, myContainer, myConfig.fTimeDialogs);
}

//With Icon.
void cd_musicplayer_new_song_playing (void) {
	cairo_dock_remove_dialog_if_any (myIcon); //On evite la superposition
	if (!myConfig.bIconBubble) {
		cd_musicplayer_new_song_playing_old ();
		return;
	}
	
	gchar *cImagePath = NULL;
	if (myConfig.cUserImage[PLAYER_NONE] != NULL)
		cImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[PLAYER_NONE]);
	else
		cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[PLAYER_NONE]);
	
	cairo_dock_show_temporary_dialog_with_icon (myData.cRawTitle, myIcon, myContainer, myConfig.fTimeDialogs, cImagePath);
	g_free (cImagePath);
}

//Fonction qui anime l'icone au changement de musique
void cd_musicplayer_animate_icon (int animationLength) {
	if (myDock && myConfig.cChangeAnimation != NULL) {
	  cd_debug ("Animation: %s", myConfig.cChangeAnimation);
		CD_APPLET_ANIMATE_MY_ICON (myConfig.cChangeAnimation, animationLength);
	}
}

void cd_musicplayer_set_surface (MyPlayerStatus iStatus) {
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
			gchar *cLocalImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cLocalImagePath);
			g_free (cLocalImagePath);
		}
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaces[iStatus]);
	}
	else {
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}

void cd_musicplayer_change_desklet_data (void) {
	cd_debug ("");
	
	if (myData.cRawTitle == NULL)
		return;
	if (myDesklet == NULL || (myConfig.iExtendedMode != MY_DESKLET_INFO && myConfig.iExtendedMode != MY_DESKLET_INFO_AND_CONTROLER))
		return;
	
	gpointer data[2] = {NULL, NULL};
	gchar **rawTitle=NULL, *artist=NULL, *title=NULL;
	if (myData.cArtist == NULL && myData.cTitle == NULL) { //On détermine l'artist (par default le 1er avant le tiret)
		rawTitle = g_strsplit (myData.cRawTitle, "-", -1);
		if (rawTitle[0] != NULL)
			artist = rawTitle[0];
			
		if (rawTitle[1] != NULL) {
			title = strchr (myData.cRawTitle, '-');
			title ++;
			while (*title == ' ')
				title ++;
		}
		data[0] = artist;
		data[1] = title;
	}
	else {
		data[0] = myData.cArtist;
		data[1] = myData.cTitle;
	}
	
	cairo_dock_render_desklet_with_new_data (myDesklet, data);
	g_strfreev (rawTitle);
}

void cd_musicplayer_player_none (void) {
	cd_debug ("");
	if (myDesklet && (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER)) {
		gpointer data[2] = {NULL, NULL};
		cairo_dock_render_desklet_with_new_data (myDesklet, data);
		gtk_widget_queue_draw (myDesklet->pWidget);
	}
}

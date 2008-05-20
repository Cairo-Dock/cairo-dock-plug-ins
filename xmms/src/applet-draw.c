/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

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
void cd_xmms_add_buttons_to_desklet(void) {
	if (myDesklet && myConfig.extendedDesklet){
		GList *pIconList = _list_icons ();
		myDesklet->icons = pIconList;
	}
}

void cd_xmms_draw_icon (void) {
	gboolean bNeedRedraw = FALSE;
	if (myData.playingStatus == PLAYER_NONE) {
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(NULL);
	}
	else {
		switch (myConfig.quickInfoType) {
			case MY_APPLET_NOTHING :
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(NULL);
			break ;
			
			case MY_APPLET_TIME_ELAPSED :
				if (myData.iCurrentTime != myData.iPreviousCurrentTime) {
					myData.iPreviousCurrentTime = myData.iCurrentTime;
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime)
					bNeedRedraw = TRUE;
				}
			break ;
			
			case MY_APPLET_TIME_LEFT :
				if (myData.iCurrentTime != myData.iPreviousCurrentTime) {
					myData.iPreviousCurrentTime = myData.iCurrentTime;
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime - myData.iSongLength)
					bNeedRedraw = TRUE;
				}
			break ;
			
			case MY_APPLET_TRACK :
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
			CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle)
		}
		else {
		  cd_message("Changing title to: %s", myData.playingTitle);
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.playingTitle)
			if (myConfig.enableAnim) {
		    cd_message("Animating for: %s", myData.playingTitle);
			  cd_xmms_animate_icon(1);
		  }
		  if (myConfig.enableDialogs) {
		    //cd_message("Bubble for: %s", myData.playingTitle);
			  cd_xmms_new_song_playing();
		  }
		}
	}
	
	if (myData.playingStatus != myData.previousPlayingStatus) {  // changement de statut.
		cd_message ("PlayingStatus : %d -> %d\n", myData.previousPlayingStatus, myData.playingStatus);
		myData.previousPlayingStatus = myData.playingStatus;
		cd_xmms_set_surface (myData.playingStatus);
		if (myData.playingStatus == 0) {
		  myData.playingTitle = NULL; //Rien ne joue
		  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle)
		}
	}
	else if (bNeedRedraw) {
		CD_APPLET_REDRAW_MY_ICON
	}
}

//Fonction de dessin en mode desklet, penser a faire un mode de rendu dans rendering
/*void cd_xmms_draw_in_desklet (cairo_t *pCairoContext, gchar *cQuickInfo) {
  double h = myDesklet->iHeight, w = myDesklet->iWidth;
  Icon *pIcon;
  GList *pIconList = NULL;
  
  //On efface ce qu'il y a dans le desklet: bidouille...
  cairo_surface_t *pSurface;
	gchar *cImagePath = g_strdup_printf ("%s/blank.svg", MY_APPLET_SHARE_DATA_DIR);
  pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
	g_free (cImagePath);
	CD_APPLET_SET_SURFACE_ON_MY_ICON(pSurface);
  
	pIcon = g_new0 (Icon, 1);
	pIcon->acName = g_strdup_printf ("%s", myData.playingTitle);
	
  switch(myData.playingStatus) { //On Affiche le bon statut du lecteur
    case PLAYER_PLAYING:
      if (myConfig.cPlayIcon != NULL) {
        pIcon->acFileName = g_strdup_printf ("%s",myConfig.cPlayIcon);
      }
      else {
        pIcon->acFileName = g_strdup_printf ("%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
      }
    break;
    case PLAYER_PAUSED:
      if (myConfig.cPauseIcon != NULL) {
        pIcon->acFileName = g_strdup_printf ("%s",myConfig.cPauseIcon);
      }
      else {
        pIcon->acFileName = g_strdup_printf ("%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
      }
    break;
    case PLAYER_STOPPED:
      if (myConfig.cStopIcon != NULL) {
        pIcon->acFileName = g_strdup_printf ("%s",myConfig.cStopIcon);
      }
      else {
        pIcon->acFileName = g_strdup_printf ("%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
      }
    break;
    case PLAYER_BROKEN:
      if (myConfig.cBrokenIcon != NULL) {
        pIcon->acFileName = g_strdup_printf ("%s",myConfig.cBrokenIcon);
      }
      else {
        pIcon->acFileName = g_strdup_printf ("%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
      }
    break;
  }
	pIcon->cQuickInfo = cQuickInfo;
	pIcon->fOrder = 1;
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup ("none");
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	pIcon->fDrawX = 0;
	pIcon->fDrawY = 0;
	pIcon->fWidth = h-3;
	pIcon->fHeight = h-3;
	pIconList = g_list_append (pIconList, pIcon);
	
	//On détermine l'artist (par default le 1er avant le tiret)
	gchar **rawTitle, *artist, *title;
	rawTitle = g_strsplit(myData.playingTitle,"-", -1);
	artist = g_strdup_printf (" %s",rawTitle[0]);
	title = g_strdup_printf (" %s",rawTitle[1]);
	
	//On affiche l'icon du status
	cairo_dock_fill_one_icon_buffer (pIcon, pCairoContext, 1., CAIRO_DOCK_HORIZONTAL, FALSE);
	cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, 10);
	
	//On affiche l'artiste en 1er
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, h+2, h/2-g_iLabelSize-20); //On deplace la zone de travail
	
	pIcon->acName = artist;
	cairo_dock_fill_one_text_buffer (pIcon, pCairoContext, g_iLabelSize, g_cLabelPolice, CAIRO_DOCK_HORIZONTAL); //On remplis le buffer de text
	
	cairo_set_source_surface (pCairoContext, pIcon->pTextBuffer, 0, 0);
	cairo_paint (pCairoContext); //On dessine
	cairo_restore (pCairoContext);
	
	//On affiche le titre en 2em
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, h+2, h/2-g_iLabelSize+10); //On deplace la zone de travail
	
	pIcon->acName = title;
	cairo_dock_fill_one_text_buffer (pIcon, pCairoContext, g_iLabelSize, g_cLabelPolice, CAIRO_DOCK_HORIZONTAL); //On remplis le buffer de text
	
	cairo_set_source_surface (pCairoContext, pIcon->pTextBuffer, 0, 0); 
	cairo_paint (pCairoContext); //On dessine
	cairo_restore (pCairoContext);
	
  if (strcmp(myData.lastQuickInfo,cQuickInfo) != 0) {
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(cQuickInfo);
		myData.lastQuickInfo = cQuickInfo;
  }
	CD_APPLET_REDRAW_MY_ICON
}*/



//Fonction qui affiche la bulle au changement de musique
//Old function without icon
void cd_xmms_new_song_playing_old(void) {
	cairo_dock_show_temporary_dialog(myData.playingTitle, myIcon, myContainer, myConfig.timeDialogs);
}

//With Icon.
void cd_xmms_new_song_playing(void) {
	cairo_dock_remove_dialog_if_any (myIcon); //On evite la superposition ?
	if (!myConfig.bIconBubble) {
		cd_xmms_new_song_playing_old();
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
void cd_xmms_animate_icon(int animationLength) {
	if (myDock) {
		CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength)
	}
}

void cd_xmms_set_surface (MyPlayerStatus iStatus) {
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

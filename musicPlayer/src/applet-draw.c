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
#include "3dcover-draw.h"
#include "applet-musicplayer.h"
#include "applet-cover.h"

static gchar *s_cDefaultIconName[PLAYER_NB_STATUS] = {"default.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};
static gchar *s_cDefaultIconName3D[PLAYER_NB_STATUS] = {"default.jpg", "play.jpg", "pause.jpg", "stop.jpg", "broken.jpg"};

/* redessine l'icone chaque seconde.
 */
gboolean cd_musicplayer_draw_icon (gpointer data)
{
	g_return_val_if_fail (myData.pCurrentHandeler->iLevel != PLAYER_EXCELLENT, FALSE);
	g_print ("%s (%d)\n", __func__, myData.iPlayingStatus);
	
	gboolean bNeedRedraw = FALSE;
	if (myData.iCurrentTime != myData.iPreviousCurrentTime)
	{
		myData.iPreviousCurrentTime = myData.iCurrentTime;
		if (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime);
			bNeedRedraw = TRUE;
		}
		else if (myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime - myData.iSongLength);
			bNeedRedraw = TRUE;
		}
	}
	
	if (myData.pCurrentHandeler->iLevel == PLAYER_BAD)
	{
		if (myData.iPlayingStatus != myData.pPreviousPlayingStatus)  // changement de l'etat du lecteur.
		{
			cd_debug ("MP : PlayingStatus : %d -> %d\n", myData.pPreviousPlayingStatus, myData.iPlayingStatus);
			myData.pPreviousPlayingStatus = myData.iPlayingStatus;
			
			cd_musicplayer_update_icon (FALSE);
			bNeedRedraw = FALSE;
		}
		else if (cairo_dock_strings_differ (myData.cPreviousRawTitle, myData.cRawTitle) || myData.iTrackNumber != myData.iPreviousTrackNumber)  // changement de chanson.
		{
			g_free (myData.cPreviousRawTitle);
			myData.cPreviousRawTitle = g_strdup (myData.cRawTitle);
			myData.iPreviousTrackNumber = myData.iTrackNumber;
			cd_musicplayer_update_icon (FALSE);
			bNeedRedraw = FALSE;
		}
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	return TRUE;
}


gboolean cd_musicplayer_check_size_is_constant (const gchar *cFilePath)
{
	int iSize = cairo_dock_get_file_size (cFilePath);
	gboolean bConstantSize = (iSize != 0 && iSize == myData.iCurrentFileSize);
	myData.iCurrentFileSize = iSize;
	if (iSize == 0)
		myData.iNbCheckFile ++;
	return bConstantSize;
}

/* Teste la disponibilite de la pochette, et l'affiche sur l'icone si possible.
 */
gboolean cd_musiplayer_set_cover_if_present (gboolean bCheckSize)
{
	g_print ("%s (%s)\n", __func__, myData.cCoverPath);
	if (g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
	{
		cd_message ("MP : la couverture '%s' est presente sur le disque", myData.cCoverPath);
		
		if (!bCheckSize || cd_musicplayer_check_size_is_constant (myData.cCoverPath))
		{
			cd_message ("MP : sa taille est constante (%d)", myData.iCurrentFileSize);
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
			{	
				if (myData.iPrevTextureCover != 0)
					_cairo_dock_delete_texture (myData.iPrevTextureCover);
				myData.iPrevTextureCover = myData.TextureCover;
				myData.TextureCover = cairo_dock_create_texture_from_image (myData.cCoverPath);
				if (myData.iPrevTextureCover != 0)
				{
					myData.iCoverTransition = NB_TRANSITION_STEP;
					cairo_dock_launch_animation (myContainer);
				}
				else
				{
					cd_opengl_render_to_texture (myApplet);
					CD_APPLET_REDRAW_MY_ICON;
				}
			}
			else
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCoverPath);
				CD_APPLET_REDRAW_MY_ICON;
			}
			myData.cover_exist = TRUE;
			myData.iSidCheckCover = 0;
			return FALSE;
		}
	}
	myData.iNbCheckFile ++;
	if (myData.iNbCheckFile > 5)  // on abandonne au bout de 5s.
	{
		g_print ("on abandonne la pochette\n");
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	return TRUE;
}


static gboolean _cd_musicplayer_check_distant_cover_twice (gpointer data)
{
	myData.pCurrentHandeler->get_cover ();  // on ne recupere que la couverture.
	cd_musicplayer_update_icon (FALSE);
	myData.iSidGetCoverInfoTwice = 0;
	return FALSE;
}
/* Met entierement a jour l'icone au changement d'etat ou de chanson.
 */
void cd_musicplayer_update_icon (gboolean bFirstTime)
{
	cd_message ("%s (%d, %s)", __func__, bFirstTime, myData.cTitle);
	if (myData.cPlayingUri != NULL || myData.cTitle != NULL)
	{
		if (bFirstTime)
		{
			//Affichage de la chanson courante.
			gchar *songName = g_strdup_printf("%s - %s", myData.cArtist, myData.cTitle);
			cd_message ("  songName : %s", songName);
			CD_APPLET_SET_NAME_FOR_MY_ICON (songName);
			g_free (songName);
			
			//Affichage de l'info-rapide.
			if (myConfig.iQuickInfoType == MY_APPLET_TRACK && myData.iTrackNumber > 0)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%d", (myDesklet && myDesklet->iWidth >= 64 ? D_("Track") : ""), myData.iTrackNumber);  // inutile de redessiner notre icone, ce sera fait plus loin.
			}
			else
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			}
			
			//Animation de l'icone et dialogue.
			cd_musicplayer_animate_icon (1);
			if(myConfig.bEnableDialogs)
			{
				cd_musicplayer_popup_info ();
			}
		}
		
		//Affichage de la couverture de l'album.
		if (myData.iSidCheckCover != 0)  // on stoppe la precedente boucle de verification de la couverture.
		{
			g_source_remove (myData.iSidCheckCover);
			myData.iSidCheckCover = 0;
		}
		if (myData.iSidGetCoverInfoTwice != 0)  // on stoppe la precedente boucle de temporisation.
		{
			g_source_remove (myData.iSidGetCoverInfoTwice);
			myData.iSidGetCoverInfoTwice = 0;
		}
		if (myData.cCoverPath == NULL && bFirstTime && myData.pCurrentHandeler->get_cover != NULL)  // info manquante, cela arrive avec les chansons distantes (bug du lecteur ?) on teste 2 fois de suite a 2 secondes d'intervalle.
		{
			g_print ("on reviendra dans 2s\n");
			myData.iSidGetCoverInfoTwice = g_timeout_add_seconds (2, (GSourceFunc) _cd_musicplayer_check_distant_cover_twice, NULL);
		}
		else if (myData.cCoverPath != NULL && ! myData.cover_exist && myConfig.bEnableCover)  // couverture connue mais pas encore chargee.
		{
			if (myData.bCoverNeedsTest)  // il faut lancer le test en boucle.
			{
				if (myData.iSidCheckXmlFile == 0 && myData.iSidCheckCover == 0)  // pas de fichier XML intermediaire a telecharger ou alors c'est deja fait.
				{
					myData.iCurrentFileSize = 0;
					myData.iNbCheckFile = 0;
					myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc) cd_musiplayer_set_cover_if_present, GINT_TO_POINTER (TRUE));  // TRUE <=> tester la taille contante.
				}
			}
			else  // la couverture est deja disponible, on peut tester tout de suite.
			{
				cd_musiplayer_set_cover_if_present (FALSE);  // FALSE <=> tester seulement l'existence du fichier.
			}
		}
		
		g_print ("cover_exist : %d\n", myData.cover_exist);
		if (! myData.cover_exist && bFirstTime)  // en attendant d'avoir une couverture, ou s'il n'y en a tout simplement pas, on met les images par defaut. La 2eme fois ce n'est pas la peine de le refaire, puisque si on passe une 2eme fois dans cette fonction, c'est bien parce que la couverture n'existait pas la 1ere fois.
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
	else  // aucune donnees, c'est soit un probleme soit le lecteur qui s'est ferme.
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myData.bIsRunning)
			cd_musicplayer_set_surface (PLAYER_STOPPED);  // je ne sais pas si en mode Stopped la chanson est NULL ou pas...
		else
			cd_musicplayer_set_surface (PLAYER_NONE);
	}
}


/* Affiche les infos de la chanson courante dans une bulle de dialogue.
 */
void cd_musicplayer_popup_info (void)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	if ((!myData.cArtist || !myData.cAlbum) && myData.cPlayingUri)
	{
		gchar *str = strrchr (myData.cPlayingUri, '/');
		if (str)
			str ++;
		else
			str = myData.cPlayingUri;
		cairo_dock_show_temporary_dialog_with_icon ("%s : %s",
			myIcon,
			myContainer,
			myConfig.iDialogDuration,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			D_("New song"),
			str);
	}
	else
		cairo_dock_show_temporary_dialog_with_icon ("%s : %s\n%s : %s\n%s : %s",
			myIcon,
			myContainer,
			myConfig.iDialogDuration,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			D_("Artist"),
			myData.cArtist != NULL ? myData.cArtist : D_("Unknown"),
			D_("Album"),
			myData.cAlbum != NULL ? myData.cAlbum : D_("Unknown"),
			D_("Title"),
			myData.cTitle != NULL ? myData.cTitle : D_("Unknown"));
}

/* Anime l'icone au changement de musique
 */
void cd_musicplayer_animate_icon (int animationLength)
{
	if (myDock && myConfig.cChangeAnimation != NULL)
	{
		CD_APPLET_ANIMATE_MY_ICON (myConfig.cChangeAnimation, animationLength);
	}
}

/* Applique la surface correspondant a un etat sur l'icone.
 */
void cd_musicplayer_set_surface (MyPlayerStatus iStatus)
{
	g_return_if_fail (iStatus < PLAYER_NB_STATUS);
	gboolean bUse3DTheme = (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes);
	gchar **cIconName = (bUse3DTheme ? s_cDefaultIconName3D : s_cDefaultIconName);
	cairo_surface_t *pSurface = myData.pSurfaces[iStatus];
	
	if (pSurface == NULL)  // surface pas encore chargee.
	{
		if (myConfig.cUserImage[iStatus] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, cIconName[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		if (bUse3DTheme)
		{
			if (myData.iPrevTextureCover != 0)
				_cairo_dock_delete_texture (myData.iPrevTextureCover);
			myData.iPrevTextureCover = myData.TextureCover;
			myData.TextureCover = cairo_dock_create_texture_from_surface (myData.pSurfaces[iStatus]);
			if (myData.iPrevTextureCover != 0)
			{
				myData.iCoverTransition = NB_TRANSITION_STEP;
				cairo_dock_launch_animation (myContainer);
			}
			else
			{
				cd_opengl_render_to_texture (myApplet);
				CD_APPLET_REDRAW_MY_ICON;
			}
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[iStatus]);
		}
	}
	else  // surface en memoire.
	{
		if (bUse3DTheme)
		{
			if (myData.iPrevTextureCover != 0)
				_cairo_dock_delete_texture (myData.iPrevTextureCover);
			myData.iPrevTextureCover = myData.TextureCover;
			myData.TextureCover = cairo_dock_create_texture_from_surface (pSurface);
			if (myData.iPrevTextureCover != 0)
			{
				myData.iCoverTransition = NB_TRANSITION_STEP;
				cairo_dock_launch_animation (myContainer);
			}
			else
			{
				cd_opengl_render_to_texture (myApplet);
				CD_APPLET_REDRAW_MY_ICON;
			}
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		}
	}
}

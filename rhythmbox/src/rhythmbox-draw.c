#include "string.h"
#include <glib/gi18n.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"
#include "3dcover-draw.h"

static gchar *s_cIconName[PLAYER_NB_STATUS] = {"default.jpg", "play.jpg", "pause.jpg", "stop.jpg", "broken.jpg"};

static GList * _list_icons (void)
{
	GList *pIconList = NULL;
	Icon *pIcon;
	int i;
	for (i = 0; i < 4; i ++)
	{
		pIcon = g_new0 (Icon, 1);
		pIcon->acName = NULL;
		pIcon->acFileName = g_strdup_printf ("%s/%d.png", MY_APPLET_SHARE_DATA_DIR, i);
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
void rhythmbox_add_buttons_to_desklet (void)
{
	if (myDesklet && myConfig.extendedDesklet)
	{
		GList *pIconList = _list_icons ();
		myDesklet->icons = pIconList;
	}
}

void rhythmbox_iconWitness(int animationLength)
{
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength);
}

gboolean _rhythmbox_check_cover_is_present (gpointer data)
{
	cd_debug ("%s (%s)", __func__, myData.playing_cover);
	if (g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
	{
		cd_message ("RB : la couverture '%s' est desormais disponible", myData.playing_cover);
		
		if (myData.CoverWasDistant)
		{
			// On n'affiche pas tout de suite la pochette sinon on risque un bug avec lastfm
			cd_debug ("RB : BOUCLE 2 : C'est une pochette qui était distante -> On affiche rien avant la prochaine boucle");
			myData.CoverWasDistant = FALSE ;
			return TRUE;
		}
		else
		{
			cd_debug ("RB : BOUCLE 2 : La pochette est locale -> On affiche");
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
			{	
				myData.TextureCover = cd_opengl_load_texture (myApplet, myData.playing_cover);
			}
			else
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
				CD_APPLET_REDRAW_MY_ICON;
			}
			myData.cover_exist = TRUE;
			myData.iSidCheckCover = 0;
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}
void update_icon(gboolean make_witness)
{
	cd_message ("Update icon");
	if(myData.playing_uri != NULL)
	{
		//Affichage de la chanson courante.
		gchar *songName = g_strdup_printf("%s - %s", myData.playing_artist, myData.playing_title);
		cd_message ("  songName : %s", songName);
		CD_APPLET_SET_NAME_FOR_MY_ICON (songName);
		g_free (songName);
		
		//Affichage de l'info-rapide.
		if(myConfig.quickInfoType == MY_APPLET_TRACK && myData.playing_track > 0)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%d", (myDesklet && myDesklet->iWidth >= 64 ? D_("Track") : ""), myData.playing_track);  // inutile de redessiner notre icone, ce sera fait plus loin.
		}
		
		//Affichage de la couverture de l'album.
		if (!myData.cover_exist && myConfig.enableCover && myData.playing_cover != NULL && g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
		{
			cd_message ("RB : la couverture '%s' est deja dispo", myData.playing_cover);
			
			if (myData.CoverWasDistant)
			{
				// On n'affiche pas tout de suite la pochette sinon on risque un bug avec lastfm
				cd_debug ("RB : BOUCLE 1 : C'est une pochette qui était distante -> On affiche rien avant la prochaine boucle");
				myData.CoverWasDistant = FALSE ;
			}
			else
			{
				cd_debug ("RB : BOUCLE 1 : La pochette est locale -> On affiche");
			
				if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
				{	
					myData.TextureCover = cd_opengl_load_texture (myApplet, myData.playing_cover);
				}
				else
				{
					CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
					CD_APPLET_REDRAW_MY_ICON;
				}
			}
			
			myData.cover_exist = TRUE;
			
			if (myData.iSidCheckCover != 0)
			{
				g_source_remove (myData.iSidCheckCover);
				myData.iSidCheckCover = 0;
			}
		}
			
		
		else
		{
			if(myData.playing)
			{
				rhythmbox_set_surface (PLAYER_PLAYING);
			}
			else
			{
				rhythmbox_set_surface (PLAYER_PAUSED);
			}
			myData.cover_exist = FALSE;
			if (myConfig.enableCover && myData.playing_cover != NULL && myData.iSidCheckCover == 0)
			{
				cd_message ("RB : myData.playing_cover : %s, mais n'existe pas encore => on boucle.", myData.playing_cover);
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc) _rhythmbox_check_cover_is_present, (gpointer) NULL);
			}
		}
		
		//Animation de l'icone et dialogue.
		if(make_witness)
		{
			rhythmbox_iconWitness(1);
			if(myConfig.enableDialogs)
			{
				music_dialog();
			}
		}
	}
	else
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myData.opening)
			rhythmbox_set_surface (PLAYER_STOPPED);  // je ne sais pas si en mode Stopped la chanson est NULL ou pas...
		else
			rhythmbox_set_surface (PLAYER_NONE);
			
	}
}

void music_dialog(void)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	cairo_dock_show_temporary_dialog ("%s : %s\n%s : %s\n%s : %s",
		myIcon,
		myContainer,
		myConfig.timeDialogs,
		D_("Artist"),
		myData.playing_artist != NULL ? myData.playing_artist : D_("Unknown"),
		D_("Album"),
		myData.playing_album != NULL ? myData.playing_album : D_("Unknown"),
		D_("Title"),
		myData.playing_title != NULL ? myData.playing_title : D_("Unknown"));
}


void rhythmbox_set_surface (MyAppletPlayerStatus iStatus)
{
	g_return_if_fail (iStatus < PLAYER_NB_STATUS);
	cairo_surface_t *pSurface = myData.pSurfaces[iStatus];
	if (pSurface == NULL) {
		if (myConfig.cUserImage[iStatus] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iStatus]);
			
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
			{	
				myData.TextureCover = cd_opengl_load_texture (myApplet, cUserImagePath);
			}
			else
			{		
				myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			}
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cIconName[iStatus]);
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
			{	
				myData.TextureCover = cd_opengl_load_texture (myApplet, cImagePath);
			}
			else
			{
				myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			}
			g_free (cImagePath);
		}
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[iStatus]);
	}
	else {
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}

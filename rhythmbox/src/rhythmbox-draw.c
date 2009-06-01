#include "string.h"
#include <glib/gi18n.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"
#include "3dcover-draw.h"

static gchar *s_cDefaultIconName[PLAYER_NB_STATUS] = {"default.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};
static gchar *s_cDefaultIconName3D[PLAYER_NB_STATUS] = {"default.jpg", "play.jpg", "pause.jpg", "stop.jpg", "broken.jpg"};

/*static GList * _list_icons (void)
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
}*/

void rhythmbox_iconWitness(int animationLength)
{
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength);
}

gboolean _rhythmbox_check_cover_is_present (gpointer data)
{
	//g_print ("%s (%s)\n", __func__, myData.playing_cover);
	if (g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
	{
		cd_message ("RB : la couverture '%s' est desormais disponible", myData.playing_cover);
		
		if (myData.CoverWasDistant)
		{
			/// tester la taille du fichier sur 2 iterations successives ... 
			cd_debug ("RB : BOUCLE 2 : C'est une pochette qui était distante -> On n'affiche rien avant la prochaine boucle");
			myData.CoverWasDistant = FALSE ;
			myData.cover_exist = FALSE;
			return TRUE;
		}
		else
		{
			cd_debug ("RB : BOUCLE 2 : La pochette est locale -> On affiche");
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
			{	
				if (myData.TextureCover != 0)
					_cairo_dock_delete_texture (myData.TextureCover);
				myData.TextureCover = cairo_dock_create_texture_from_image (myData.playing_cover);
				cd_opengl_render_to_texture (myApplet);
			}
			else
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
			}
			CD_APPLET_REDRAW_MY_ICON;
			myData.cover_exist = TRUE;
			myData.iSidCheckCover = 0;
			return FALSE;
		}
	}
	else
	{
		myData.cover_exist = FALSE;
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
		if (!myData.cover_exist && myConfig.enableCover && myData.playing_cover != NULL)  // couverture potentielle mais pas encore chargee.
		{
			if (myData.iSidCheckCover != 0)
			{
				g_source_remove (myData.iSidCheckCover);
				myData.iSidCheckCover = 0;
			}	
			_rhythmbox_check_cover_is_present (myApplet);
			if (! myData.cover_exist)
			{
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc) _rhythmbox_check_cover_is_present, (gpointer) NULL);
			}
		}
		
		if (! myData.cover_exist)
		{
			if(myData.playing)
			{
				rhythmbox_set_surface (PLAYER_PLAYING);
			}
			else
			{
				rhythmbox_set_surface (PLAYER_PAUSED);
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
		if (myData.bIsRunning)
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
			if (myData.TextureCover != 0)
				_cairo_dock_delete_texture (myData.TextureCover);
			myData.TextureCover = cairo_dock_create_texture_from_surface (myData.pSurfaces[iStatus]);
			cd_opengl_render_to_texture (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
			g_print ("DESSIN RB\n");
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
			if (myData.TextureCover != 0)
				_cairo_dock_delete_texture (myData.TextureCover);
			myData.TextureCover = cairo_dock_create_texture_from_surface (pSurface);
			cd_opengl_render_to_texture (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
			g_print ("DESSIN RB\n");
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		}
	}
}

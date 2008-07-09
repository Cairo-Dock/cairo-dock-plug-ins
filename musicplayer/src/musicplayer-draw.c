#include "string.h"
#include <glib/gi18n.h>

#include "musicplayer-struct.h"
#include "musicplayer-draw.h"
#include "musicplayer-dbus.h"

CD_APPLET_INCLUDE_MY_VARS

static gchar *s_cIconName[PLAYER_NB_STATUS] = {"default.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};


void musicplayer_iconWitness(int animationLength)
{
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength)
}

/*gboolean _exaile_check_cover_is_present (gpointer data)
{
	if (g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover)
		CD_APPLET_REDRAW_MY_ICON
		myData.cover_exist = TRUE;
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}*/

void update_icon(gboolean make_witness)
{
	cd_message ("musicplayer : Update icon avec %d", make_witness);
	cd_message("musicplayer : valeur de enableDialogs --> %d",myConfig.enableDialogs);
	
	gchar *cover_path = NULL;
	
	if(myData.status >= 0 && !myData.stopped)
	{
		//Affichage de la chanson courante.
		gchar *songName = g_strdup_printf("%s - %s", myData.playing_title, myData.playing_artist);
		cd_message ("  songName : %s", songName);
		CD_APPLET_SET_NAME_FOR_MY_ICON (songName);
		g_free (songName);
		
		//Affichage de l'info-rapide (en attente de comment traiter ce putain de uchar)
		/*if(myConfig.quickInfoType == MY_APPLET_TRACK && myData.playing_track > 0)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.playing_track);  // inutile de redessiner notre icone, ce sera fait plus loin.
		}*/

		
		//Affichage de la couverture de l'album.
		musicplayer_getCoverPath();

		if (myConfig.enableCover && myData.playing_cover != NULL && g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
		{
			myData.cover_exist = TRUE;
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
			CD_APPLET_REDRAW_MY_ICON
		}
		else
		{
			if(myData.playing) musicplayer_set_surface (PLAYER_PLAYING);
			else if (myData.paused)	musicplayer_set_surface (PLAYER_PAUSED);
			else if (myData.stopped) musicplayer_set_surface (PLAYER_STOPPED);
			myData.cover_exist = FALSE;
		}
		
		//Animation de l'icone et dialogue.
		if(make_witness)
		{
			musicplayer_iconWitness(1);
			if(myConfig.enableDialogs)
			{
				music_dialog();
			}
		}
	}
	else
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
		if (myData.opening)
			musicplayer_set_surface (PLAYER_STOPPED); 
		else
			musicplayer_set_surface (PLAYER_NONE);
	}
	CD_APPLET_REDRAW_MY_ICON
}

void music_dialog(void)
{
	cairo_dock_show_temporary_dialog (D_("Artist : %s\nAlbum : %s\nTitle : %s"),
		myIcon,
		myContainer,
		myConfig.timeDialogs,
		myData.playing_artist != NULL ? myData.playing_artist : D_("Unknown"),
		myData.playing_album != NULL ? myData.playing_album : D_("Unknown"),
		myData.playing_title != NULL ? myData.playing_title : D_("Unknown"));
}


void musicplayer_set_surface (MyAppletPlayerStatus iStatus)
{
	g_return_if_fail (iStatus < PLAYER_NB_STATUS);
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




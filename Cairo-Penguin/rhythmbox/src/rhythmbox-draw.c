#include "string.h"
#include <glib/gi18n.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


void rhythmbox_iconWitness(int animationLength)
{
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength)
}

gboolean _rhythmbox_check_cover_is_present (gpointer data)
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
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", myData.playing_track);  // inutile de redessiner notre icone, ce sera fait plus loin.
		}
		
		//Affichage de la couverture de l'album.
		//gchar *cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.playing_artist, myData.playing_album);
		//cd_message ("  cover : %s", cover);
		if (myConfig.enableCover && myData.playing_cover != NULL && g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
			CD_APPLET_REDRAW_MY_ICON
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
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pPlaySurface);
			}
			else
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pPauseSurface);
			}
			myData.cover_exist = FALSE;
			if (myConfig.enableCover && myData.playing_cover != NULL && myData.iSidCheckCover == 0)
			{
				g_print ("myData.playing_cover : %s mais n'existe pas encore !", myData.playing_cover);
				myData.iSidCheckCover = g_timeout_add (1000, (GSourceFunc) _rhythmbox_check_cover_is_present, (gpointer) NULL);
			}
		}
		//g_free (cover);
		
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
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurface);
	}
	CD_APPLET_REDRAW_MY_ICON
}

void music_dialog(void)
{
	cairo_dock_show_temporary_dialog (_D("Artist : %s\nAlbum : %s\nTitle : %s"),
		myIcon,
		myDock,
		myConfig.timeDialogs,
		myData.playing_artist != NULL ? myData.playing_artist : _D("Unknown"),
		myData.playing_album != NULL ? myData.playing_album : _D("Unknown"),
		myData.playing_title != NULL ? myData.playing_title : _D("Unknown"));
}

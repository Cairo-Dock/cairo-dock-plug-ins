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


void update_icon(gboolean make_witness)
{
	g_print ("%s ()\n",__func__);
	if(myData.playing_uri != NULL)
	{
		//Affichage de la chanson courante.
		gchar *songName = g_strdup_printf("%s - %s", myData.playing_artist, myData.playing_title);
		g_print ("  songName : %s\n", songName);
		CD_APPLET_SET_NAME_FOR_MY_ICON (songName);
		g_free (songName);
		
		//Affichage de l'info-rapide.
		if(myConfig.quickInfoType == MY_APPLET_TRACK && myData.playing_track > 0)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", myData.playing_track);  // inutile de redessiner notre icone, ce sera fait plus loin.
		}
		
		//Affichage de la couverture de l'album.
		gchar *cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.playing_artist, myData.playing_album);
		g_print ("  cover : %s\n", cover);
		if(g_file_test (cover, G_FILE_TEST_EXISTS) && myConfig.enableCover)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (cover);
			myData.cover_exist = TRUE;
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
		}
		g_free (cover);
		
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

#include "string.h"
#include <glib/gi18n.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"

//Inclusion des variables de dessins
CD_APPLET_INCLUDE_MY_VARS

extern cairo_surface_t *rhythmbox_pSurface;
extern cairo_surface_t *rhythmbox_pPlaySurface;
extern cairo_surface_t *rhythmbox_pPauseSurface;
extern cairo_surface_t *rhythmbox_pStopSurface;
extern cairo_surface_t *rhythmbox_pCover;

//Inclusion des variables de configuration
extern gchar *conf_defaultTitle;
extern CairoDockAnimationType conf_changeAnimation;
extern gboolean conf_enableDialogs;
extern gboolean conf_enableCover;
extern double conf_timeDialogs;
extern MyAppletQuickInfoType conf_quickInfoType;

//Inclusion des variables d'etats
extern gboolean rhythmbox_opening;
extern gboolean rhythmbox_playing;
extern gboolean cover_exist;
extern int playing_duration;
extern int playing_track;
extern gchar *playing_uri;
extern const gchar *playing_artist;
extern const gchar *playing_album;
extern const gchar *playing_title;

void rhythmbox_iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (conf_changeAnimation, animationLenght)
}


void update_icon(gboolean make_witness)
{
	g_print ("%s ()\n",__func__);
	if(playing_uri != NULL)
	{
		gchar *songName,*cover;
		
		songName = g_strdup_printf("%s - %s",playing_artist,playing_title);
		g_print ("  songName : %s\n", songName);
		CD_APPLET_SET_NAME_FOR_MY_ICON (songName);
		g_free (songName);
		
		//Affichage de l'info-rapide.
		if(conf_quickInfoType == MY_APPLET_TRACK && playing_track > 0)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", playing_track);  // inutile de redessiner notre icone, ce sera fait plus loin.
		}
		
		//Affichage de l'album
		cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), playing_artist,playing_album);
		g_print ("  cover : %s\n", cover);
		if(g_file_test (cover, G_FILE_TEST_EXISTS) && conf_enableCover)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (cover);
			cover_exist = TRUE;
		}
		else
		{
			if(rhythmbox_playing)
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pPlaySurface);
			}
			else
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pPauseSurface);
			}
			cover_exist = FALSE;
		}
		g_free (cover);
		
		
		if(make_witness)
		{
			rhythmbox_iconWitness(1);
			if(conf_enableDialogs)
			{
				music_dialog();
			}
		}
	}
	else
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (conf_defaultTitle);
		CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pSurface);
	}
}

void music_dialog(void)
{
	gchar *message = g_strdup_printf(_D("Artist : %s\nAlbum : %s\nTitle : %s"),playing_artist,playing_album,playing_title);
	cairo_dock_show_temporary_dialog (message,myIcon,myDock,conf_timeDialogs);
	g_free (message);
}

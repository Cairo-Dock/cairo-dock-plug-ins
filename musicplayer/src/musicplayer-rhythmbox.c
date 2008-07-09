/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"
#include "string.h"
#include <dbus/dbus-glib.h>
#include <glib/gi18n.h>

#include "musicplayer-dbus.h"
#include "musicplayer-struct.h"
#include "musicplayer-init.h"
#include "musicplayer-config.h"
#include "musicplayer-draw.h"
#include "musicplayer-rhythmbox.h"

CD_APPLET_INCLUDE_MY_VARS


gboolean musicplayer_rhythmbox_dbus_connection (void)
{
	cd_message ("");
	if (cairo_dock_bdus_is_enabled ())
	{
		dbus_proxy_player = cairo_dock_create_new_session_proxy (
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Player",
			"org.gnome.Rhythmbox.Player"
		);
		
		dbus_proxy_shell = cairo_dock_create_new_session_proxy (
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Shell",
			"org.gnome.Rhythmbox.Shell"
		);
		
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingUriChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "elapsedChanged",
			G_TYPE_UINT,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL, NULL);
			
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL, NULL);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCovertArtChanged), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

void rhythmbox_dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL);
		cd_message ("playingChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL);
		cd_message ("playingUriChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL);
		cd_message ("elapsedChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCovertArtChanged), NULL);
		cd_message ("onCovertArtChanged deconnecte\n");
		
		g_object_unref (dbus_proxy_player);
		dbus_proxy_player = NULL;
	}
	if (dbus_proxy_shell != NULL)
	{
		g_object_unref (dbus_proxy_shell);
		dbus_proxy_shell = NULL;
	}
}

void dbus_detect_rhythmbox(void)
{
	cd_message ("");
	myData.opening = cairo_dock_dbus_detect_application ("org.gnome.Rhythmbox");
}


//*********************************************************************************
// rhythmbox_getPlaying() : Test si Rhythmbox joue de la musique ou non
//*********************************************************************************
void rhythmbox_getPlaying (void)
{
	cd_message ("");
	myData.playing = cairo_dock_dbus_get_boolean (dbus_proxy_player, "getPlaying");
	cd_message("musicplayer : statut : %d", myData.playing);
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
void rhythmbox_getPlayingUri(void)
{
	cd_message ("");
	
	g_free (myData.playing_uri);
	myData.playing_uri = NULL;
	
	myData.playing_uri = cairo_dock_dbus_get_string (dbus_proxy_player, "getPlayingUri");
	cd_message("musicplayer : playing uri --> %s", myData.playing_uri);
}


void getSongInfos(void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
	GError *error = NULL;
	
	if(dbus_g_proxy_call (dbus_proxy_shell, "getSongProperties", &error,
		G_TYPE_STRING, myData.playing_uri,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
		g_free (myData.playing_artist);
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_artist = g_strdup (g_value_get_string(value));
		else myData.playing_artist = NULL;
		cd_message ("musicplayer : playing_artist <- %s", myData.playing_artist);
		
		g_free (myData.playing_album);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_album = g_strdup (g_value_get_string(value));
		else myData.playing_album = NULL;
		cd_message ("musicplayer : playing_album <- %s", myData.playing_album);
		
		g_free (myData.playing_title);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_title = g_strdup (g_value_get_string(value));
		else myData.playing_title = NULL;
		cd_message ("musicplayer : playing_title <- %s", myData.playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.playing_track = g_value_get_uint(value);
		else myData.playing_track = 0;
		cd_message ("musicplayer : playing_track <- %d", myData.playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.playing_duration = g_value_get_uint(value);
		else myData.playing_duration = 0;
		cd_message ("musicplayer : playing_duration <- %ds", myData.playing_duration);
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		g_free (myData.playing_cover);
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
		{
			GError *erreur = NULL;
			const gchar *cString = g_value_get_string(value);
			if (cString != NULL && strncmp (cString, "file://", 7) == 0)
			{
				myData.playing_cover = g_filename_from_uri (cString, NULL, &erreur);
				if (erreur != NULL)
				{
					cd_warning ("Attention : %s", erreur->message);
					g_error_free (erreur);
				}
			}
			else
			{
				myData.playing_cover = g_strdup (cString);
			}
		}
		else
		{
			gchar *cSongPath = g_filename_from_uri (myData.playing_uri, NULL, NULL);  // on teste d'abord dans le repertoire de la chanson.
		cd_message("musicplayer : coucou 2");
			if (cSongPath != NULL)
			{
						cd_message("musicplayer : coucou 2");

				gchar *cSongDir = g_path_get_dirname (cSongPath);
				g_free (cSongPath);
				myData.playing_cover = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, myData.playing_artist, myData.playing_album);
				g_free (cSongDir);
				cd_message ("musicplayer : test de %s\n", myData.playing_cover);
				if (! g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
				{
					g_free (myData.playing_cover);
					myData.playing_cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.playing_artist, myData.playing_album);
				}
			}
		}
		cd_message("musicplayer :  playing_cover <- %s", myData.playing_cover);
		
		//g_hash_table_destroy (data_list);
	}
	else
	{
		//trace("musicplayer : error RB : %s", error->message);
		cd_warning ("musicplayer : can't get song properties");
		g_free (myData.playing_uri);
		myData.playing_uri = NULL;
		g_free (myData.playing_cover);
		myData.playing_cover = NULL;
	}
}


//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	cd_message ("%s (%s)",__func__,uri);
	
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);  // on redessine a la fin.
	
	g_free (myData.playing_uri);
	if(uri != NULL && *uri != '\0')
	{
		myData.playing_uri = g_strdup (uri);
		myData.opening = TRUE;
		getSongInfos();
	}
	else
	{
		myData.playing_uri = NULL;
		myData.cover_exist = FALSE;
		
		g_free (myData.playing_artist);
		myData.playing_artist = NULL;
		g_free (myData.playing_album);
		myData.playing_album = NULL;
		g_free (myData.playing_title);
		myData.playing_title = NULL;
		g_free (myData.playing_cover);
		myData.playing_cover = NULL;
		myData.playing_duration = 0;
		myData.playing_track = 0;
		
		dbus_detect_rhythmbox();
	}
	update_icon(TRUE);
}

//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void onChangePlaying(DBusGProxy *player_proxy, gboolean playing, gpointer data)
{
	cd_message ("");
	myData.playing = playing;
	if(! myData.cover_exist && myData.playing_uri != NULL)
	{
		cd_message ("  playing_uri : %s", myData.playing_uri);
		if(myData.playing)
		{
			musicplayer_set_surface (PLAYER_PLAYING);
		}
		else
		{
			musicplayer_set_surface (PLAYER_PAUSED);
		}
	}
}

//*********************************************************************************
// rhythmbox_elapsedChanged() : Fonction executée à chaque changement de temps joué
//*********************************************************************************
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data)
{
	if(elapsed > 0)
	{
		//g_print ("%s () : %ds\n", __func__, elapsed);
		if(myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed)
			CD_APPLET_REDRAW_MY_ICON
		}
		else if(myConfig.quickInfoType == MY_APPLET_TIME_LEFT)  // avec un '-' devant.
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed - myData.playing_duration)
			CD_APPLET_REDRAW_MY_ICON
		}
		else if(myConfig.quickInfoType == MY_APPLET_PERCENTAGE)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int) (100.*elapsed/myData.playing_duration))
			CD_APPLET_REDRAW_MY_ICON
		}
	}
}


void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data)
{
	cd_message ("%s (%s)",__func__,cImageURI);
	g_free (myData.playing_cover);
	myData.playing_cover = g_strdup (cImageURI);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
	CD_APPLET_REDRAW_MY_ICON
	myData.cover_exist = TRUE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
}

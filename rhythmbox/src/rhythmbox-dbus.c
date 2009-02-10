#include <string.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-dbus.h"

static DBusGProxy *dbus_proxy_player = NULL;
static DBusGProxy *dbus_proxy_shell = NULL;


gboolean rhythmbox_dbus_connect_to_bus (void)
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
		cd_debug ("playingChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL);
		cd_debug ("playingUriChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL);
		cd_debug ("elapsedChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCovertArtChanged), NULL);
		cd_debug ("onCovertArtChanged deconnecte");
		
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
}


void getSongInfos(void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
	
	if(dbus_g_proxy_call (dbus_proxy_shell, "getSongProperties", NULL,
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
		cd_message ("  playing_artist <- %s", myData.playing_artist);
		
		g_free (myData.playing_album);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_album = g_strdup (g_value_get_string(value));
		else myData.playing_album = NULL;
		cd_message ("  playing_album <- %s", myData.playing_album);
		
		g_free (myData.playing_title);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_title = g_strdup (g_value_get_string(value));
		else myData.playing_title = NULL;
		cd_message ("  playing_title <- %s", myData.playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.playing_track = g_value_get_uint(value);
		else myData.playing_track = 0;
		cd_message ("  playing_track <- %d", myData.playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.playing_duration = g_value_get_uint(value);
		else myData.playing_duration = 0;
		cd_message ("  playing_duration <- %ds", myData.playing_duration);
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		g_free (myData.playing_cover);
		myData.playing_cover = NULL;
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
		{
			const gchar *cString = g_value_get_string(value);
			cd_debug ("RB nous a refile cette adresse : %s", cString);
			if (cString != NULL)
			{
				if (strncmp (cString, "file://", 7) == 0)
				{
					GError *erreur = NULL;
					myData.playing_cover = g_filename_from_uri (cString, NULL, &erreur);
					if (erreur != NULL)
					{
						cd_warning ("Attention : %s", erreur->message);
						g_error_free (erreur);
					}
				}
				else if (*cString == '/')
				{
					myData.playing_cover = g_strdup (cString);
				}
			}
		}
		if (myData.playing_cover == NULL)
		{
			gchar *cSongPath = g_filename_from_uri (myData.playing_uri, NULL, NULL);  // on teste d'abord dans le repertoire de la chanson.
			if (cSongPath != NULL)  // c'est un fichier local.
			{
				gchar *cSongDir = g_path_get_dirname (cSongPath);
				g_free (cSongPath);
				
				myData.playing_cover = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, myData.playing_artist, myData.playing_album);
				cd_debug ("test de %s", myData.playing_cover);
				if (! g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
				{
					g_free (myData.playing_cover);
					myData.playing_cover = g_strdup_printf ("%s/cover.jpg", cSongDir);
					cd_debug ("  test de %s", myData.playing_cover);
					if (! g_file_test (myData.playing_cover, G_FILE_TEST_EXISTS))
					{
						g_free (myData.playing_cover);
						myData.playing_cover = NULL;
					}
				}
				g_free (cSongDir);
			}
			
			if (myData.playing_cover == NULL)  // on regarde maintenant dans le cache de RB.
			{
				myData.playing_cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.playing_artist, myData.playing_album);  /// gerer le repertoire ~/.cache/rhythmbox/covers ...
			}
		}
		g_print ("  playing_cover <- %s", myData.playing_cover);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("  can't get song properties");
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
			rhythmbox_set_surface (PLAYER_PLAYING);
		}
		else
		{
			rhythmbox_set_surface (PLAYER_PAUSED);
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
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.quickInfoType == MY_APPLET_TIME_LEFT)  // avec un '-' devant.
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed - myData.playing_duration);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.quickInfoType == MY_APPLET_PERCENTAGE)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int) (100.*elapsed/myData.playing_duration));
			CD_APPLET_REDRAW_MY_ICON;
		}
	}
}


void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data)
{
	cd_message ("%s (%s)",__func__,cImageURI);
	g_free (myData.playing_cover);
	myData.playing_cover = g_strdup (cImageURI);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
	CD_APPLET_REDRAW_MY_ICON;
	myData.cover_exist = TRUE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
}

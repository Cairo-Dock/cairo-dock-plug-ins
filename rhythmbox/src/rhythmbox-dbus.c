#include <string.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-dbus.h"

static DBusGConnection *dbus_connexion;
static DBusGProxy *dbus_proxy_dbus;
static DBusGProxy *dbus_proxy_player;
static DBusGProxy *dbus_proxy_shell;


CD_APPLET_INCLUDE_MY_VARS


//*********************************************************************************
// rhythmbox_dbus_pre_init() : Initialise la connexion d-bus
//*********************************************************************************
gboolean rhythmbox_dbus_get_dbus (void)
{
	cd_message ("Connexion au bus ... ");
	dbus_connexion = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	
	if(!dbus_connexion)
	{
		cd_message ("echouee");
		return FALSE;
	}
	else
	{
		cd_message ("reussie");
		
		dbus_proxy_player = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Player",
			"org.gnome.Rhythmbox.Player"
		);
		
		dbus_proxy_shell = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Shell",
			"org.gnome.Rhythmbox.Shell"
		);
		
		dbus_proxy_dbus = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.freedesktop.DBus",
			"/",
			"org.freedesktop.DBus"
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
		
		return TRUE;
	}
}


void rhythmbox_dbus_connect_to_bus (void)
{
	cd_message ("");
	dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
		G_CALLBACK(onChangePlaying), NULL, NULL);
		
	dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
		G_CALLBACK(onChangeSong), NULL, NULL);
	
	dbus_g_proxy_connect_signal(dbus_proxy_player, "elapsedChanged",
		G_CALLBACK(onElapsedChanged), NULL, NULL);
}

void rhythmbox_dbus_disconnect_from_bus (void)
{
	cd_message ("");
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingChanged",
		G_CALLBACK(onChangePlaying), NULL);
	cd_message ("playingChanged deconnecte\n");
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingUriChanged",
		G_CALLBACK(onChangeSong), NULL);
	cd_message ("playingUriChanged deconnecte\n");
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "elapsedChanged",
		G_CALLBACK(onElapsedChanged), NULL);
	cd_message ("elapsedChanged deconnecte\n");
}

void dbus_detect_rhythmbox(void)
{
	cd_message ("");
	gchar **name_list = NULL;
	
	myData.opening = FALSE;
	if(dbus_g_proxy_call (dbus_proxy_dbus, "ListNames", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,
		&name_list,
		G_TYPE_INVALID))
	{
		cd_message ("  detection du service Rhythmbox...");
		int i;
		for (i = 0; name_list[i] != NULL; i ++)
		{
			if (strcmp (name_list[i], "org.gnome.Rhythmbox") == 0)
			{
				myData.opening = TRUE;
				break;
			}
		}
	}
	g_strfreev (name_list);
}


//*********************************************************************************
// rhythmbox_getPlaying() : Test si Rhythmbox joue de la musique ou non
//*********************************************************************************
void rhythmbox_getPlaying (void)
{
	cd_message ("");
	
	dbus_g_proxy_call (dbus_proxy_player, "getPlaying", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &myData.playing,
		G_TYPE_INVALID);
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
void rhythmbox_getPlayingUri(void)
{
	cd_message ("");
	
	g_free (myData.playing_uri);
	myData.playing_uri = NULL;
	
	dbus_g_proxy_call (dbus_proxy_player, "getPlayingUri", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRING, &myData.playing_uri ,
		G_TYPE_INVALID);
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
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_artist = g_value_get_string(value);
		else myData.playing_artist = NULL;
		cd_message ("  playing_artist <- %s", myData.playing_artist);
		
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_album = g_value_get_string(value);
		else myData.playing_album = NULL;
		cd_message ("  playing_album <- %s", myData.playing_album);
		
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_title = g_value_get_string(value);
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
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");  // y'aura-t-il le 'rb:' dans RB 11.3 ?
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.playing_cover = g_strdup (g_value_get_string(value));
		else myData.playing_cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.playing_artist, myData.playing_album);
		g_print ("  playing_cover <- %s", myData.playing_cover);
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
	
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);  // on redessine a la fin.
	
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
		myData.playing_uri = NULL;
		myData.playing_artist = NULL;
		myData.playing_album = NULL;
		myData.playing_title = NULL;
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
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW ("%d%%", (int) (100.*elapsed/myData.playing_duration))
		}
	}
}

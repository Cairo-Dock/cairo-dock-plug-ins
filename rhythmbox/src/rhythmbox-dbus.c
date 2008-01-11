#include <string.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-dbus.h"

DBusGConnection *dbus_connexion;
DBusGProxy *dbus_proxy_dbus;
DBusGProxy *dbus_proxy_player;
DBusGProxy *dbus_proxy_shell;

CD_APPLET_INCLUDE_MY_VARS

extern gchar *conf_defaultTitle;
extern gboolean conf_enableDialogs;
extern gboolean conf_enableCover;
extern double conf_timeDialogs;
extern MyAppletQuickInfoType conf_quickInfoType;

extern cairo_surface_t *rhythmbox_pPlaySurface;
extern cairo_surface_t *rhythmbox_pPauseSurface;
extern cairo_surface_t *rhythmbox_pStopSurface;

extern gboolean rhythmbox_opening;
extern gboolean rhythmbox_playing;
extern gboolean cover_exist;
extern int playing_duration;
extern int playing_track;
extern gchar *playing_uri;
extern const gchar *playing_artist;
extern const gchar *playing_album;
extern const gchar *playing_title;


//*********************************************************************************
// rhythmbox_dbus_pre_init() : Initialise la connexion d-bus
//*********************************************************************************
gboolean rhythmbox_dbus_get_dbus (void)
{
	g_print ("%s ()\n",__func__);

	g_print ("Connexion au bus ... ");
	dbus_connexion = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	
	if(!dbus_connexion)
	{
		g_print ("echouee\n");
		return FALSE;
	}
	else
	{
		g_print ("reussie\n");
		
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
	g_print ("%s ()\n",__func__);
	dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
		G_CALLBACK(onChangePlaying), NULL, NULL);
		
	dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
		G_CALLBACK(onChangeSong), NULL, NULL);
	
	dbus_g_proxy_connect_signal(dbus_proxy_player, "elapsedChanged",
		G_CALLBACK(onElapsedChanged), NULL, NULL);
}

void rhythmbox_dbus_disconnect_from_bus (void)
{
	g_print ("%s ()\n",__func__);
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingChanged",
		G_CALLBACK(onChangePlaying), NULL);
	g_print ("playingChanged deconnecte\n");
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingUriChanged",
		G_CALLBACK(onChangeSong), NULL);
	g_print ("playingUriChanged deconnecte\n");
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_player, "elapsedChanged",
		G_CALLBACK(onElapsedChanged), NULL);
	g_print ("elapsedChanged deconnecte\n");
}

void dbus_detect_rhythmbox(void)
{
	g_print ("%s ()\n",__func__);
	gchar **name_list = NULL;
	
	rhythmbox_opening = FALSE;
	if(dbus_g_proxy_call (dbus_proxy_dbus, "ListNames", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,
		&name_list,
		G_TYPE_INVALID))
	{
		g_print("  detection du service Rhythmbox...\n");
		int i;
		for (i = 0; name_list[i] != NULL; i ++)
		{
			if (strcmp (name_list[i], "org.gnome.Rhythmbox") == 0)
			{
				rhythmbox_opening = TRUE;
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
	g_print ("%s ()\n",__func__);
	gboolean playing;
	
	dbus_g_proxy_call (dbus_proxy_player, "getPlaying", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &playing,
		G_TYPE_INVALID);
	
	rhythmbox_playing = playing;
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
void rhythmbox_getPlayingUri(void)
{
	g_print ("%s ()\n",__func__);
	
	g_free (playing_uri);
	playing_uri = NULL;
	dbus_g_proxy_call (dbus_proxy_player, "getPlayingUri", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRING, &playing_uri ,
		G_TYPE_INVALID);
}


//*********************************************************************************
// rhythmbox_getElapsed() : Retourne le temps écoulé pour la musique joué
//*********************************************************************************
void rhythmbox_getElapsed(void)
{	
	g_print ("%s ()\n",__func__);
	int time_elapsed;
	
	dbus_g_proxy_call (dbus_proxy_player, "getElapsed", NULL,
		G_TYPE_INVALID,
		G_TYPE_UINT, &time_elapsed,
		G_TYPE_INVALID);
	g_print(" -> %ds\n",time_elapsed);
}

void getSongInfos(void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
	
	if(dbus_g_proxy_call (dbus_proxy_shell, "getSongProperties", NULL,
		G_TYPE_STRING, playing_uri,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) playing_artist = g_value_get_string(value);
		else playing_artist = "Inconnu";
		g_print ("  playing_artist <- %s\n", playing_artist);
		
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) playing_album = g_value_get_string(value);
		else playing_album = "Inconnu";
		g_print ("  playing_album <- %s\n", playing_album);
		
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) playing_title = g_value_get_string(value);
		else playing_title = "Inconnu";
		g_print ("  playing_title <- %s\n", playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) playing_track = g_value_get_uint(value);
		else playing_track = 0;
		g_print ("  playing_track <- %d\n", playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) playing_duration = g_value_get_uint(value);
		else playing_duration = 0;
		g_print ("  playing_duration <- %ds\n", playing_duration);
	}
	else
	{
		g_print ("  peut pas recevoir les proprietes\n");
		g_free (playing_uri);
		playing_uri = NULL;
	}
}


//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	g_print ("%s (%s)\n",__func__,uri);
	
	cairo_dock_remove_quick_info (myIcon);
	
	g_free (playing_uri);
	if(uri != NULL && *uri != '\0')
	{
		playing_uri = g_strdup (uri);
		rhythmbox_opening = TRUE;
		getSongInfos();
	}
	else
	{
		playing_uri = NULL;
		cover_exist = FALSE;
		playing_uri = NULL;
		playing_artist = NULL;
		playing_album = NULL;
		playing_title = NULL;
		playing_duration = 0;
		playing_track = 0;
		
		dbus_detect_rhythmbox();
	}
	update_icon(TRUE);
}

//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void onChangePlaying(DBusGProxy *player_proxy, gboolean playing, gpointer data)
{
	g_print ("%s ()\n",__func__);
	rhythmbox_playing = playing;
	if(!cover_exist && playing_uri != NULL)
	{
		g_print ("  playing_uri : %s\n", playing_uri);
		if(playing)
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pPlaySurface)
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pPauseSurface)
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
		if(conf_quickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			gchar *cQuickInfo;
			if(elapsed < 60)
			{
				cQuickInfo = g_strdup_printf ("%i", elapsed);
			}
			else
			{
				int min = elapsed / 60;
				int sec = elapsed % 60;
				if(sec < 10) cQuickInfo = g_strdup_printf ("%i:0%i", min,sec);
				else cQuickInfo = g_strdup_printf ("%i:%i", min,sec);
			}
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cQuickInfo);
			g_free (cQuickInfo);
		}
		else if(conf_quickInfoType == MY_APPLET_TIME_LEFT)
		{
			gchar *cQuickInfo;
			int time = playing_duration - elapsed;
			if(time < 60)
			{
				cQuickInfo = g_strdup_printf ("%i", time);
			}
			else
			{
				int min = time / 60;
				int sec = time % 60;
				if(sec < 10) cQuickInfo = g_strdup_printf ("%i:0%i", min,sec);
				else cQuickInfo = g_strdup_printf ("%i:%i", min,sec);
				
			}
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cQuickInfo);
			g_free (cQuickInfo);
		}
		else if(conf_quickInfoType == MY_APPLET_PERCENTAGE)
		{
			gchar *cQuickInfo = g_strdup_printf ("%d%s", ((elapsed*100/playing_duration)),"%");
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cQuickInfo);
			g_free (cQuickInfo);
		}
	}
}

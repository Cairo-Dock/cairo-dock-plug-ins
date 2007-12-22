#include <string.h>

#include <dbus/dbus-glib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-dbus.h"

#define RHYTHMBOX_DBUS_OBJECT 
#define RHYTHMBOX_DBUS_PATH_PLAYER 
#define RHYTHMBOX_DBUS_INTERFACE_PLAYER 
#define RHYTHMBOX_DBUS_PATH_SHELL 
#define RHYTHMBOX_DBUS_INTERFACE_SHELL 
#define RHYTHMBOX_DBUS_SERVICE_NAME "org.gnome.Rhythmbox"

DBusGConnection *dbus_connexion;
DBusGProxy *dbus_proxy_dbus;
DBusGProxy *dbus_proxy_player;
DBusGProxy *dbus_proxy_shell;

CD_APPLET_INCLUDE_MY_VARS

extern gchar *conf_defaultTitle;
extern gboolean conf_enableDialogs;
extern gboolean conf_enableCover;
extern double conf_timeDialogs;
extern gchar *conf_quickInfoType;

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
// rhythmbox_dbus_init() : Initialise la connexion d-bus
//*********************************************************************************
gboolean rhythmbox_dbus_init (void)
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
			RHYTHMBOX_DBUS_SERVICE_NAME,
			"/org/gnome/Rhythmbox/Player",
			"org.gnome.Rhythmbox.Player"
		);
		
		dbus_proxy_shell = dbus_g_proxy_new_for_name (
			dbus_connexion,
			RHYTHMBOX_DBUS_SERVICE_NAME,
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
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL, NULL);
			
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingUriChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		dbus_g_proxy_add_signal(dbus_proxy_player, "elapsedChanged",
			G_TYPE_UINT,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL, NULL);
		return TRUE;
	}
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
			if (strcmp (name_list[i], RHYTHMBOX_DBUS_SERVICE_NAME) == 0)
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

		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) playing_album = g_value_get_string(value);
		else playing_album = "Inconnu";

		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) playing_title = g_value_get_string(value);
		else playing_title = "Inconnu";

		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) playing_track = g_value_get_uint(value);
		else playing_track = 0;

		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) playing_duration = g_value_get_uint(value);
		else playing_duration = 0;
	}
	else
	{
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
	if(uri != NULL)
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
		gchar *cQuickInfo;
		if(strcmp(conf_quickInfoType,"elapsed") == 0)
		{
			cQuickInfo = g_strdup_printf ("%d", elapsed);
			cairo_dock_set_quick_info (myDrawContext, cQuickInfo, myIcon);
		}
		else if(strcmp(conf_quickInfoType,"rest") == 0)
		{
			cQuickInfo = g_strdup_printf ("%d", (playing_duration - elapsed));
			cairo_dock_set_quick_info (myDrawContext, cQuickInfo, myIcon);
		}
		g_free (cQuickInfo);
	}
}

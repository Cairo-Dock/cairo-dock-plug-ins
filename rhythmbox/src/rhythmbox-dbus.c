#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-init.h"
#include "rhythmbox-dbus.h"

#define RHYTHMBOX_DBUS_OBJECT "org.gnome.Rhythmbox"
#define RHYTHMBOX_DBUS_PATH_PLAYER "/org/gnome/Rhythmbox/Player"
#define RHYTHMBOX_DBUS_INTERFACE_PLAYER "org.gnome.Rhythmbox.Player"
#define RHYTHMBOX_DBUS_PATH_SHELL "/org/gnome/Rhythmbox/Shell"
#define RHYTHMBOX_DBUS_INTERFACE_SHELL "org.gnome.Rhythmbox.Shell"

DBusGConnection *dbus_connexion;
DBusGProxy *dbus_proxy_player;
DBusGProxy *dbus_proxy_shell;

extern gchar *conf_defaultTitle;

//*********************************************************************************
// rhythmbox_dbus_init() : Initialise la connexion d-bus
//*********************************************************************************
gboolean rhythmbox_dbus_init(void)
{
	g_print ("%s ()\n",__func__);

	g_print ("Connexion au bus ... ");
	dbus_connexion = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	
	if(!dbus_connexion)
	{
		g_print ("échouée\n");
		return FALSE;		
	}
	else
	{
		g_print ("réussie\n");

		dbus_proxy_player = dbus_g_proxy_new_for_name (
			dbus_connexion,
			RHYTHMBOX_DBUS_OBJECT,
			RHYTHMBOX_DBUS_PATH_PLAYER,
			RHYTHMBOX_DBUS_INTERFACE_PLAYER
		);
		
		dbus_proxy_shell = dbus_g_proxy_new_for_name (
			dbus_connexion,
			RHYTHMBOX_DBUS_OBJECT,
			RHYTHMBOX_DBUS_PATH_SHELL,
			RHYTHMBOX_DBUS_INTERFACE_SHELL
		);
		
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingUriChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(rhythmbox_onChangeSong), NULL, NULL);
		
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(rhythmbox_onChangePlaying), NULL, NULL);
	
		return TRUE;
	}
}


//*********************************************************************************
// rhythmbox_getPlaying() : Test si Rhythmbox joue de la musique ou non
//*********************************************************************************
int rhythmbox_getPlaying(void)
{
	g_print ("%s ()\n",__func__);
	gboolean playing;
			
	dbus_g_proxy_call (dbus_proxy_player, "getPlaying", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &playing,
		G_TYPE_INVALID);
		
	return playing ? 1 : 0;
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************

gchar *rhythmbox_getPlayingUri(void)
{
	g_print ("%s ()\n",__func__);
	gchar *playing_uri;
	
	if(dbus_g_proxy_call (dbus_proxy_player, "getPlayingUri", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRING, &playing_uri,
		G_TYPE_INVALID))
	{	
		return playing_uri;
	}
	else return NULL;
}


//*********************************************************************************
// rhythmbox_getElapsed() : Retourne le temps écoulé pour la musique joué
//*********************************************************************************
int rhythmbox_getElapsed(void)
{	
	g_print ("%s ()\n",__func__);
	int time_elapsed;
	
	dbus_g_proxy_call (dbus_proxy_player, "getElapsed", NULL,
		G_TYPE_INVALID,
		G_TYPE_INT, &time_elapsed,
		G_TYPE_INVALID);
	
	g_print ("Elapsed : %s\n",time_elapsed);
	
	return time_elapsed;
}


//*********************************************************************************
// rhythmbox_getSongName() : Retourne le titre musical d'un adresse
//*********************************************************************************
gchar *rhythmbox_getSongName(gchar *uri)
{
	g_print ("%s ()\n",__func__);
	GHashTable *data_list = NULL;
	GValue *value;
	gchar *artist, *title;
	
	if(!dbus_g_proxy_call (dbus_proxy_shell, "getSongProperties", NULL,
		G_TYPE_STRING, uri,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
		return conf_defaultTitle;
	}
	else
	{
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
		{
			title = g_value_get_string(value);
		}
		else title = "Titre inconnu";
		
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
		{
			artist = g_value_get_string(value);
		}
		else artist = "Artiste inconnu";
		return g_strdup_printf ("%s - %s",artist,title);
	}
}

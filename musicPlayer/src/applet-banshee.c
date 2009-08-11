/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Yann SLADEK (for any bug report, please mail me to mav@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)
Rémy Robertson (changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-dbus.h"
#include "applet-draw.h"
#include "applet-banshee.h"

/*
service : org.bansheeproject.Banshee

<interface name="org.bansheeproject.Banshee.PlayerEngine">
	<method name="Open">
		<arg name="uri" direction="in" type="s" />
	</method>
	<method name="Close" />
	<method name="Pause" />
	<method name="Play" />
	<method name="TogglePlaying" />
	<signal name="EventChanged">
		<arg name="evnt" direction="out" type="s" />
		<arg name="message" direction="out" type="s" />
		<arg name="bufferingPercent" direction="out" type="d" />
	</signal>
	<signal name="StateChanged">
		<arg name="state" direction="out" type="s" />
	</signal>
	<property name="CurrentTrack" type="a{sv}" access="read" />
	<property name="CurrentUri" type="s" access="read" />
	<property name="CurrentState" type="s" access="read" />
	<property name="LastState" type="s" access="read" />
	<property name="Volume" type="q" access="readwrite" />
	<property name="Position" type="u" access="readwrite" />
	<property name="CanSeek" type="b" access="read" />
	<property name="CanPause" type="b" access="read" />
	<property name="Length" type="u" access="read" />
</interface>

interface : org.bansheeproject.Banshee.PlaybackController
path : /org/bansheeproject/Banshee/PlaybackController
	Next
	Previous
	PlaybackShuffleMode ShuffleMode { get; set; }
    PlaybackRepeatMode RepeatMode { get; set; }
	
interface : org.bansheeproject.Banshee.PlayerEngine
path : /org/bansheeproject/Banshee/PlayerEngine
	event DBusPlayerEventHandler EventChanged;
    event DBusPlayerStateHandler StateChanged;

    void Open (string uri);
    
    void Pause ();
    void Play ();
    void TogglePlaying ();
    
    IDictionary CurrentTrack { get; } artist,name,album,track-number
    string CurrentUri { get; }
    
    string CurrentState { get; }

    uint Position { get; set; }
    uint Length { get; }
*/


static inline void _extract_playing_status (const gchar *cCurrentState)
{
	if (cCurrentState == NULL)
		myData.iPlayingStatus = PLAYER_STOPPED;
	else if (strcmp (cCurrentState, "playing") == 0)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else if (strcmp (cCurrentState, "pause") == 0)
		myData.iPlayingStatus = PLAYER_PAUSED;
	else
		myData.iPlayingStatus = PLAYER_STOPPED;
}

/* Teste si Audacious joue de la musique ou non
 */
static void _banshee_getPlaying (void)
{
	cd_message ("");
	gchar *cCurrentState = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "GetCurrentState");
	_extract_playing_status (cCurrentState);
	g_free (cCurrentState);
}

/* Renvoie le temps ecoule en secondes..
 */
static void _banshee_get_time_elapsed (void)
{
	cd_message ("");
	myData.iCurrentTime = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_player, "GetPosition") / 1000;
}

static inline void _extract_metadata (GHashTable *data_list)
{
	GValue *value;
	g_free (myData.cArtist);
	value = (GValue *) g_hash_table_lookup (data_list, "artist");
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		myData.cArtist = g_strdup (g_value_get_string(value));
	else
		myData.cArtist = NULL;

	g_free (myData.cAlbum);
	value = (GValue *) g_hash_table_lookup (data_list, "album");
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		myData.cAlbum = g_strdup (g_value_get_string(value));
	else
		myData.cAlbum = NULL;

	g_free (myData.cTitle);
	value = (GValue *) g_hash_table_lookup (data_list, "name");
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		myData.cTitle = g_strdup (g_value_get_string (value));
	else
		myData.cTitle = NULL;

	value = (GValue *) g_hash_table_lookup(data_list, "track-number");
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		myData.iTrackNumber = g_value_get_int(value);
	else
		myData.iTrackNumber = 0;
	
	value = (GValue *) g_hash_table_lookup(data_list, "length");
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		myData.iSongLength = g_value_get_int(value);
	else
		myData.iSongLength = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_player, "GetLength") / 1000;
	
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // on memorise la precedente couverture.
	myData.cCoverPath = NULL;
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_banshee_getSongInfos (void)
{
	GHashTable *data_list = NULL;
	GValue *value;
	
	if (dbus_g_proxy_call (myData.dbus_proxy_player, "GetCurrentTrack", NULL, G_TYPE_INVALID,
		MP_DBUS_TYPE_SONG_METADATA,
		  &data_list,
		  G_TYPE_INVALID))
	{
		_extract_metadata (data_list);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("  can't get song properties");
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}


/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	cd_message ("MP : %s ()",__func__);
	
	if (metadata != NULL)
	{
		_extract_metadata (metadata);
		myData.bIsRunning = TRUE;
	}
	else
	{
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
		g_free (myData.cArtist);
		myData.cArtist = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
		myData.iSongLength = 0;
		myData.iTrackNumber = 0;
		myData.cover_exist = FALSE;
		
		cd_musicplayer_dbus_detect_player ();
	}
	cd_musicplayer_update_icon (TRUE);
}

/* Fonction executée à chaque changement play/pause
 */
static void onChangePlaying(DBusGProxy *player_proxy, gchar *cCurrentStatus, gpointer data)  // int iStatus[4]
{
	myData.bIsRunning = TRUE;
	_extract_playing_status (cCurrentStatus);
	
	if(! myData.cover_exist && myData.cPlayingUri != NULL)
	{
		if(myData.iPlayingStatus == PLAYER_PLAYING)
		{
			cd_musicplayer_set_surface (PLAYER_PLAYING);
		}
		else
		{
			cd_musicplayer_set_surface (PLAYER_PAUSED);
		}
	}
	else
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
}



////////////////////////////
// Definition du backend. //
////////////////////////////

/* Fonction de connexion au bus de audacious.
 */
gboolean cd_banshee_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // cree le proxy.
		
		myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell ();  // cree le proxy pour la 2eme interface car AU en a 2.
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "StateChanged",
			G_TYPE_INT,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "EventChanged",
			MP_DBUS_TYPE_SONG_METADATA,
			G_TYPE_INVALID);
		
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "StateChanged",
			G_CALLBACK(onChangePlaying), NULL, NULL);
			
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "EventChanged",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par notre controleur
 */
void cd_banshee_free_data (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "StateChanged",
			G_CALLBACK(onChangePlaying), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "EventChanged",
			G_CALLBACK(onChangeSong), NULL);
	}
	
	musicplayer_dbus_disconnect_from_bus();
	musicplayer_dbus_disconnect_from_bus_Shell();
}

/* Controle du lecteur */
void cd_banshee_control (MyPlayerControl pControl, const char *file)
{
	gchar *cCommand = NULL;
	
	switch (pControl)
	{
		case PLAYER_PREVIOUS :
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "Previous", 
				G_TYPE_BOOLEAN, FALSE,  // FALSE <=> dont play it now
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_PLAY_PAUSE :
			cairo_dock_dbus_call (myData.dbus_proxy_player, "TogglePlaying");
		break;

		case PLAYER_NEXT :
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "Next", 
				G_TYPE_BOOLEAN, FALSE,  // FALSE <=> dont play it now
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_SHUFFLE :
		{
			gboolean bShuffle = cairo_dock_dbus_get_boolean (myData.dbus_proxy_shell, "GetShuffleMode");
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetShuffleMode", 
				G_TYPE_BOOLEAN, ! bShuffle,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		}
		break;
		
		case PLAYER_REPEAT :
		{
			int iReapeat = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetRepeatMode");
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetRepeatMode", 
				G_TYPE_BOOLEAN, (iReapeat + 1) % 3,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		}
		break;
		
		default :
			return;
	}
}


/* Fonction de lecture des infos */
void cd_banshee_read_data (void)
{
	if (myData.dbus_enable)
	{
		if (myData.bIsRunning && myData.iPlayingStatus == PLAYER_PLAYING)
		{
			_banshee_get_time_elapsed ();
		}
		else
		{
			myData.iCurrentTime = 0;
		}
	}
	
}

void cd_banshee_load_dbus_commands (void)
{
	//cd_debug("MP : On charge les commande pour Banshee");
	myData.DBus_commands.service = "org.bansheeproject.Banshee";
	myData.DBus_commands.path = "/org/bansheeproject/Banshee/PlayerEngine";
	myData.DBus_commands.interface = "org.bansheeproject.Banshee.PlayerEngine";
	myData.DBus_commands.path2 = "/org/bansheeproject/Banshee/PlaybackController";
	myData.DBus_commands.interface2 = "org.bansheeproject.Banshee.PlaybackController";
	myData.DBus_commands.play = "Play";
	myData.DBus_commands.pause = "Pause";
	myData.DBus_commands.play_pause = "TogglePlaying";
	myData.DBus_commands.next = "Next";
	myData.DBus_commands.previous = "Previous";
	myData.DBus_commands.get_title = "GetCurrentTrack";
	myData.DBus_commands.get_artist = "";
	myData.DBus_commands.get_album = "";
	myData.DBus_commands.get_cover_path = "";
	myData.DBus_commands.get_status = "GetCurrentState";
	myData.DBus_commands.duration = "GetLength";
	myData.DBus_commands.current_position = "GetPosition";
	return;
}

/* Initialise le backend de BA.
 */
void cd_banshee_configure (void)
{
	cd_debug ("");
	myData.DBus_commands.service = "org.bansheeproject.Banshee";
	myData.DBus_commands.path = "/org/bansheeproject/Banshee/PlaybackController";
	myData.DBus_commands.interface = "org.bansheeproject.Banshee.PlaybackController";
	myData.DBus_commands.path2 = "/org/bansheeproject/Banshee/PlayerEngine";
	myData.DBus_commands.interface2 = "org.bansheeproject.Banshee.PlayerEngine";
	
	myData.dbus_enable = cd_banshee_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de AU.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de BA sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			g_print ("MP : BA is running");
			_banshee_getPlaying();
			cd_banshee_getSongInfos ();
			cd_musicplayer_update_icon (TRUE);
		}
		else  // player eteint.
		{
			cd_musicplayer_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		cd_musicplayer_set_surface (PLAYER_BROKEN);
	}
}

void cd_musicplayer_register_banshee_handler (void) { //On enregistre notre lecteur
	MusicPlayerHandeler *pBanshee = g_new0 (MusicPlayerHandeler, 1);
	pBanshee->read_data = cd_banshee_read_data;  // recupere le temps ecoule car on n'a pas de signal pour ca.
	pBanshee->free_data = cd_banshee_free_data;
	pBanshee->configure = cd_banshee_configure;  // renseigne les proprietes DBus et se connecte au bus.
	pBanshee->control = cd_banshee_control;
	pBanshee->get_cover = NULL;
	pBanshee->cCoverDir = NULL;  // banshee ne gere pas les pochettes.
	
	pBanshee->appclass = "Banshee";
	pBanshee->launch = "banshee";
	pBanshee->name = "Banshee";
	pBanshee->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE;
	pBanshee->iPlayer = MP_BANSHEE;
	pBanshee->bSeparateAcquisition = FALSE;
	pBanshee->iLevel = PLAYER_GOOD;  // n'a besoin d'une boucle que pour afficher le temps ecoule.
	cd_musicplayer_register_my_handler (pBanshee,"Banshee");
}


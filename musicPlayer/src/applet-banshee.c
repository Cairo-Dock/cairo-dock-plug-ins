/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-dbus.h"
#include "applet-draw.h"
#include "applet-cover.h"
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

	cover cache : "$HOME/.cache/album-art/`GetCurrentTrack["artwork-id"])`.jpg"
*/


static inline gboolean _extract_playing_status (const gchar *cCurrentState)
{
	myData.pPreviousPlayingStatus = myData.iPlayingStatus;
	if (cCurrentState == NULL)
		return FALSE;
	if (strcmp (cCurrentState, "playing") == 0)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else if (strcmp (cCurrentState, "paused") == 0)
		myData.iPlayingStatus = PLAYER_PAUSED;
	else if (strcmp (cCurrentState, "idle") == 0)
	{
		myData.iPlayingStatus = PLAYER_STOPPED;
		return FALSE;  // on ne redessine pas ici, on va attendre la prochaine iteration de read_data. En effet, le lecteur passe par l'etat "idle" a chaque chgt de chanson, alors qu'il n'est manifestement pas a l'arret.
	}
	else  // loading, loaded, etc => on considere que le lecteur va jouer la chanson des qu'il aura fini de la charger, on se place donc direct en mode "playing".
	{
		myData.iPlayingStatus = PLAYER_PLAYING;
		return FALSE;  // inutile de redessiner pour ces etats.
	}
	return TRUE;
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
		
	g_free (myData.cPlayingUri);
	myData.cPlayingUri = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "GetCurrentUri");
	
	const gchar *cString = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "artwork-id");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
		cString = g_value_get_string(value);
	cd_debug ("MP : got cover path from Banshee : '%s'\n", cString);
	gchar *cCoverPath = (cString ? g_strdup_printf ("%s/.cache/album-art/%s.jpg", g_getenv ("HOME"), cString) : NULL);
	if (cString && ! g_file_test (cCoverPath, G_FILE_TEST_EXISTS)) // path has changed :-/
	{
		g_free (cCoverPath);
		cCoverPath = g_strdup_printf ("%s/.cache/media-art/%s.jpg", g_getenv ("HOME"), cString);
	}
	cd_musicplayer_set_cover_path (cCoverPath);
	g_free (cCoverPath);
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_banshee_getSongInfos (void)
{
	GHashTable *data_list = NULL;
	
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

static void cd_banshee_getCoverPath (void)
{
	cd_debug ("MP - %s ()\n", __func__);
	GHashTable *data_list = NULL;
	GValue *value;
	GError *erreur = NULL;
	
	dbus_g_proxy_call (myData.dbus_proxy_player, "GetCurrentTrack", &erreur, G_TYPE_INVALID,
		MP_DBUS_TYPE_SONG_METADATA,
		&data_list,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		myData.iPlayingStatus = PLAYER_NONE;
	}
	else
	{
		const gchar *cString = NULL;
		value = (GValue *) g_hash_table_lookup(data_list, "artwork-id");
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
			cString = g_value_get_string(value);
		cd_debug ("MP -  => got cover path from Banshee : '%s'\n", cString);
		gchar *cCoverPath = g_strdup_printf ("%s/.cache/album-art/300/%s.jpg", g_getenv ("HOME"), cString);
		if (! g_file_test  (cCoverPath, G_FILE_TEST_EXISTS))
		{
			g_free (cCoverPath);
			cCoverPath = g_strdup_printf ("%s/.cache/album-art/%s.jpg", g_getenv ("HOME"), cString);
			if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS)) // path has changed :-/
			{
				g_free (cCoverPath);
				cCoverPath = g_strdup_printf ("%s/.cache/media-art/300/%s.jpg", g_getenv ("HOME"), cString);
				if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
				{
					g_free (cCoverPath);
					cCoverPath = g_strdup_printf ("%s/.cache/media-art/%s.jpg", g_getenv ("HOME"), cString);
				}
			}
		}
		cd_musicplayer_set_cover_path (cString);
		g_free (cCoverPath);
		g_hash_table_destroy (data_list);
	}
}
/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, const gchar *cEvent, const gchar *cMessage, double fBufferingPercent, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s (%s, %s, %.2f)\n", __func__, cEvent, cMessage, fBufferingPercent);
	if (cMessage != NULL)
	{
		if (strcmp (cMessage, "startofstream") == 0)  // new song
		{
			cd_banshee_getSongInfos ();
		}
		else if (strcmp (cMessage, "trackinfoupdated") == 0)  // song update, in practice only the cover can be updated.
		{
			cd_debug ("MP -  trackinfoupdated\n");
			if (myData.cCoverPath == NULL)  // the player didn't provide a cover the first time, see if it does now.
			{
				cd_debug ("MP -   aucune pochette, on regarde si Banshee a mieux a nous proposer\n");
				cd_banshee_getCoverPath ();
				CD_APPLET_LEAVE ();
			}
		}
		else
			CD_APPLET_LEAVE ();
	}
	else
	{
		cd_debug ("MP - message vide !\n");
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
	}
	cd_musicplayer_update_icon ();
	CD_APPLET_LEAVE ();
}

static void g_cclosure_marshal_VOID__STRING_STRING_DOUBLE (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	cd_debug ("MP - %s ()\n", __func__);
	const GValue *value;
	const gchar *cEvent = NULL;
	const gchar *cMessage = NULL;
	double fBufferingPercent=0.;
	
	value = &param_values[0];
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		cEvent = g_value_get_string (value);
	
	value = &param_values[1];
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		cMessage = g_value_get_string (value);
	
	value = &param_values[2];
	if (value != NULL && G_VALUE_HOLDS_DOUBLE (value))
		fBufferingPercent = g_value_get_double (value);
	
	onChangeSong (NULL, cEvent, cMessage, fBufferingPercent, marshal_data);
}


/* Fonction executée à chaque changement play/pause
 */
static void onChangePlaying(DBusGProxy *player_proxy, const gchar *cCurrentStatus, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s (%s)\n", __func__, cCurrentStatus);
	gboolean bStateChanged = _extract_playing_status (cCurrentStatus);
	if (! bStateChanged)
		CD_APPLET_LEAVE ();
		//return ;
	
	if (myData.iPlayingStatus == PLAYER_PLAYING)
		cd_musicplayer_relaunch_handler ();
	if(! myData.cover_exist && myData.cPlayingUri != NULL)
	{
		cd_musicplayer_apply_status_surface (myData.iPlayingStatus);
	}
	else
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
	CD_APPLET_LEAVE ();
}



////////////////////////////
// Definition du backend. //
////////////////////////////

/* Controle du lecteur.
 */
static void cd_banshee_control (MyPlayerControl pControl, const char *file)
{
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
			gboolean bShuffle = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetShuffleMode");
			cd_debug ("MP - bShuffle : %d\n", bShuffle);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetShuffleMode", 
				G_TYPE_INT, ! bShuffle,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		}
		break;
		
		case PLAYER_REPEAT :
		{
			int iRepeat = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetRepeatMode");
			cd_debug ("MP - iRepeat : %d\n", iRepeat);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetRepeatMode", 
				G_TYPE_INT, (iRepeat+1)%3,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
		}
		break;
		
		default :
			return;
	}
}


/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_banshee_get_data (void)
{
	if (myData.iPlayingStatus == PLAYER_PLAYING)
	{
		_banshee_get_time_elapsed();
		if (myData.iCurrentTime < 0)
			myData.iPlayingStatus = PLAYER_STOPPED;
	}
	else if (myData.iPlayingStatus != PLAYER_PAUSED)  // en pause le temps reste constant.
	{
		myData.iCurrentTime = 0;
		if (myData.iPlayingStatus == PLAYER_STOPPED && myData.pPreviousPlayingStatus != PLAYER_STOPPED)  /// utile ?...
		{
			myData.pPreviousPlayingStatus = PLAYER_STOPPED;
			cd_musicplayer_apply_status_surface (PLAYER_NONE);
			g_free (myData.cCoverPath);
			myData.cCoverPath = NULL;
		}
	}
}

/* Initialise le backend de BA.
 */
static void cd_banshee_start (void)
{
	// register to the signals
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "StateChanged",
		G_TYPE_STRING,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "StateChanged",
		G_CALLBACK(onChangePlaying), NULL, NULL);

	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_STRING_DOUBLE,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE ,G_TYPE_INVALID);	
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "EventChanged",
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE,  // MP_DBUS_TYPE_SONG_METADATA
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "EventChanged",
		G_CALLBACK(onChangeSong), NULL, NULL);
	
	// get the current state.
	_banshee_getPlaying ();
	cd_banshee_getSongInfos ();
	cd_musicplayer_update_icon ();
}

void cd_musicplayer_register_banshee_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Banshee";
	pHandler->get_data = cd_banshee_get_data;  // recupere le temps ecoule car on n'a pas de signal pour ca.
	pHandler->stop = NULL;
	pHandler->start = cd_banshee_start;  // renseigne les proprietes DBus et se connecte au bus.
	pHandler->control = cd_banshee_control;
	pHandler->get_cover = NULL;
	pHandler->cCoverDir = g_strdup_printf ("%s/.cache/media-art", g_getenv ("HOME"));
	
	pHandler->cMprisService = "org.bansheeproject.Banshee";
	pHandler->path = "/org/bansheeproject/Banshee/PlaybackController";
	pHandler->interface = "org.bansheeproject.Banshee.PlaybackController";
	pHandler->path2 = "/org/bansheeproject/Banshee/PlayerEngine";
	pHandler->interface2 = "org.bansheeproject.Banshee.PlayerEngine";
	
	pHandler->appclass = "banshee";  // en fait la vraie classe est plus compliquee (Mono oblige), mais le dock sait extraire ca.
	pHandler->launch = "banshee";
	pHandler->cMpris2Service = "org.mpris.MediaPlayer2.banshee";
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE;
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_GOOD;  // n'a besoin d'une boucle que pour afficher le temps ecoule.
	cd_musicplayer_register_my_handler (pHandler);
}


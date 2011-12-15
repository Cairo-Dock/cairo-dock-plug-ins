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
#include "applet-cover.h"
#include "applet-draw.h"
#include "applet-quodlibet.h"

/* cat ~/.quodlibet/current
comment=#NIPPONSEI @ IRC.RIZON.NET
album=Danzai no Hana ~Guilty Sky~
~#playcount=3
~#bitrate=320000
artist=Kosaka Riyu
~#length=261
title=Danzai no Hana ~Guilty Sky~
~#rating=0.500000
genre=Anime
~#added=1250552346
~filename=/media/mecenie/Claymore_[P-L]/ED_Single/01 - Danzai no Hana ~Guilty Sky~.mp3
~#skipcount=0
~#mtime=1180436036.000000
date=2007
tracknumber=1
~mountpoint=/media/mecenie
~#laststarted=1250553619
~#lastplayed=1250553881
~format=MP3
 */

#define MP_DBUS_TYPE_PLAYER_STATUS G_TYPE_BOOLEAN
#define QL_DBUS_TYPE_SONG_METADATA dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_STRING)
/////////////////////////////////
// Les Fonctions propres a QL. //
/////////////////////////////////

static inline void _extract_playing_status (gboolean status)
{
	cd_debug ("%s (%d)\n", __func__, status);
	
	if (status)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
}

static void _quodlibet_getPlaying (void)
{
	cd_debug ("%s ()\n", __func__);
	GError *erreur = NULL;
	gboolean status;
	dbus_g_proxy_call (myData.dbus_proxy_player, "IsPlaying", &erreur,
		G_TYPE_INVALID,
		MP_DBUS_TYPE_PLAYER_STATUS, &status,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		myData.iPlayingStatus = PLAYER_NONE;
	}
	else
	{
		_extract_playing_status (status);
	}
}

/* Renvoie le temps ecoule en secondes.
 */
static gint64 cairo_dock_dbus_get_integer64 (DBusGProxy *pDbusProxy, const gchar *cAccessor)
{
	GError *erreur = NULL;
	gint64 iValue = 0;
	dbus_g_proxy_call (pDbusProxy, cAccessor, &erreur,
		G_TYPE_INVALID,
		G_TYPE_INT64, &iValue,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	return iValue;
}
static void _quodlibet_get_time_elapsed (void)
{
	myData.iCurrentTime = cairo_dock_dbus_get_integer64 (myData.dbus_proxy_player, "GetPosition")/1000;
	cd_debug ("MP : current_position <- %i\n", myData.iCurrentTime);
}

static inline void _extract_metadata (GHashTable *data_list)  // album, date, discnumber, discsubtitle, tracknumber, location, organization, labelid, producer, ~filename, ~#length, ~#bitrate, ~#track, ~#disc
{
	const gchar *value;
	g_free (myData.cArtist);
	value = (const char *) g_hash_table_lookup (data_list, "artist");
	if (value != NULL)
		myData.cArtist = g_strdup (value);
	else
		myData.cArtist = NULL;
	cd_debug ("  MP : playing_artist <- '%s'\n", myData.cArtist);
	
	g_free (myData.cAlbum);
	value = (const char *) g_hash_table_lookup (data_list, "album");
	if (value != NULL)
		myData.cAlbum = g_strdup (value);
	else
		myData.cAlbum = NULL;
	cd_debug ("  MP : playing_album <- '%s'\n", myData.cAlbum);
	
	g_free (myData.cTitle);
	value = (const char *) g_hash_table_lookup (data_list, "title");
	if (value != NULL)
		myData.cTitle = g_strdup (value);
	else
		myData.cTitle = NULL;
	cd_debug ("  MP : playing_title <- '%s'\n", myData.cTitle);
	
	value = (const char *) g_hash_table_lookup (data_list, "tracknumber");  // ~#track ?
	cd_debug ("MP : tracknumber : '%s'\n", value);
	if (value != NULL)
		myData.iTrackNumber = atoll (value);
	else
		myData.iTrackNumber = 0;
	cd_debug ("  MP : playing_track <- %d\n", myData.iTrackNumber);
	
	value = (const char *) g_hash_table_lookup (data_list, "~#length");
	cd_debug ("MP : ~#length : '%s'\n", value);
	if (value != NULL) 
		myData.iSongLength = atoll (value);
	else 
		myData.iSongLength = 0;
	cd_debug ("  MP : playing_duration <- %d\n", myData.iSongLength);
	
	g_free (myData.cPlayingUri);
	value = (const char *) g_hash_table_lookup (data_list, "~filename");   // location ?
	if (value != NULL)
		myData.cPlayingUri = g_strdup (value);
	else
		myData.cPlayingUri = NULL;
	cd_debug ("  cUri <- %s\n", myData.cPlayingUri);
	
	cd_musicplayer_get_cover_path (NULL, TRUE);
}

static void cd_quodlibet_getSongInfos (void)
{
	GHashTable *data_list = NULL;
	
	if (dbus_g_proxy_call (myData.dbus_proxy_player, "CurrentSong", NULL,
		G_TYPE_INVALID,
		QL_DBUS_TYPE_SONG_METADATA,
		&data_list,
		G_TYPE_INVALID))
	{
		_extract_metadata (data_list);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("MP : Can't get song properties");
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cArtist);
		myData.cArtist = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
		myData.iSongLength = 0;
		myData.iTrackNumber = 0;
		myData.cover_exist = FALSE;
	}
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executee à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s ()\n", __func__);
	
	if (metadata != NULL)
	{
		_extract_metadata (metadata);
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
	}
	cd_musicplayer_update_icon (TRUE);
	CD_APPLET_LEAVE ();
}

/* Fonction executee à chaque changement "paused".
 */
static void on_pause (DBusGProxy *player_proxy, gpointer data)  // paused
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s ()\n", __func__);
	
	myData.iPlayingStatus = PLAYER_PAUSED;
	
	if(! myData.cover_exist && (myData.cPlayingUri != NULL || myData.cTitle != NULL))
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
	CD_APPLET_LEAVE ();
}
/* Fonction executee à chaque changement "unpaused".
 */
static void on_unpaused (DBusGProxy *player_proxy, gpointer data)  // unpaused
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s ()\n", __func__);
	
	myData.iPlayingStatus = PLAYER_PLAYING;
	
	cd_musicplayer_relaunch_handler ();
	if(! myData.cover_exist && (myData.cPlayingUri != NULL || myData.cTitle != NULL))
	{
		cd_musicplayer_set_surface (myData.iPlayingStatus);
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

/* Controle du lecteur
 */
static void cd_quodlibet_control (MyPlayerControl pControl, const char* song)
{
	const gchar *cCommand = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "Previous";
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = "PlayPause";
		break;

		case PLAYER_NEXT :
			cCommand = "Next";
		break;

		default :
			return;
		break;
	}
	
	if (cCommand != NULL)
	{
		cd_debug ("MP : Handler QuodLibet : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}

/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_quodlibet_get_data (void)
{
	if (myData.iPlayingStatus == PLAYER_PLAYING)
	{
		_quodlibet_get_time_elapsed ();
		if (myData.iCurrentTime < 0)
			myData.iPlayingStatus = PLAYER_STOPPED;
	}
	else if (myData.iPlayingStatus != PLAYER_PAUSED)  // en pause le temps reste constant.
		myData.iCurrentTime = 0;
}

/* Initialise le backend de QL.
 */
static void cd_quodlibet_start (void)
{
	// register to the signals
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "paused",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "paused",
		G_CALLBACK(on_pause), NULL, NULL);

	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "unpaused",
		G_TYPE_NONE,
		G_TYPE_INVALID);  // idem.
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "unpaused",
		G_CALLBACK(on_unpaused), NULL, NULL);

	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "song-started",
		QL_DBUS_TYPE_SONG_METADATA,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "song-started",
		G_CALLBACK(onChangeSong), NULL, NULL);
	
	// get the current state.
	_quodlibet_getPlaying ();
	cd_quodlibet_getSongInfos ();
	cd_musicplayer_update_icon (TRUE);
}


/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_quodlibet_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "QuodLibet";
	pHandler->get_data = cd_quodlibet_get_data;
	pHandler->stop = NULL;
	pHandler->start = cd_quodlibet_start;
	pHandler->control = cd_quodlibet_control;
	pHandler->get_cover = NULL;
	pHandler->cCoverDir = NULL;  /// il me semble que QL gere les pochettes ...
	
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT;
	pHandler->appclass = "quodlibet";
	pHandler->launch = "quodlibet";
	pHandler->cMprisService = "net.sacredchao.QuodLibet";
	pHandler->cMpris2Service = "org.mpris.MediaPlayer2.quodlibet";
	pHandler->path = "/net/sacredchao/QuodLibet";
	pHandler->interface = "net.sacredchao.QuodLibet";
	pHandler->path2 = NULL;
	pHandler->interface2 = NULL;
	
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_GOOD;  // n'a besoin d'une boucle que pour afficher le temps ecoule.
	cd_musicplayer_register_my_handler (pHandler);
}

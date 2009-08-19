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
	g_print ("%s (%d)\n", __func__, status);
	
	if (status)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
}

static void _quodlibet_getPlaying (void)
{
	g_print ("%s ()\n", __func__);
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
		myData.iPlayingStatus = PLAYER_STOPPED;
	}
	else
	{
		_extract_playing_status (status);
	}
}

/* Renvoie le temps ecoule en secondes.
 */
gint64 cairo_dock_dbus_get_integer64 (DBusGProxy *pDbusProxy, const gchar *cAccessor)
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
	g_print ("MP : current_position <- %i\n", myData.iCurrentTime);
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
	g_print ("  MP : playing_artist <- '%s'\n", myData.cArtist);
	
	g_free (myData.cAlbum);
	value = (const char *) g_hash_table_lookup (data_list, "album");
	if (value != NULL)
		myData.cAlbum = g_strdup (value);
	else
		myData.cAlbum = NULL;
	g_print ("  MP : playing_album <- '%s'\n", myData.cAlbum);
	
	g_free (myData.cTitle);
	value = (const char *) g_hash_table_lookup (data_list, "title");
	if (value != NULL)
		myData.cTitle = g_strdup (value);
	else
		myData.cTitle = NULL;
	g_print ("  MP : playing_title <- '%s'\n", myData.cTitle);
	
	value = (const char *) g_hash_table_lookup (data_list, "tracknumber");  // ~#track ?
	g_print ("MP : tracknumber : '%s'\n", value);
	if (value != NULL)
		myData.iTrackNumber = atoll (value);
	else
		myData.iTrackNumber = 0;
	g_print ("  MP : playing_track <- %d\n", myData.iTrackNumber);
	
	value = (const char *) g_hash_table_lookup (data_list, "~#length");
	g_print ("MP : ~#length : '%s'\n", value);
	if (value != NULL) 
		myData.iSongLength = atoll (value);
	else 
		myData.iSongLength = 0;
	g_print ("  MP : playing_duration <- %d\n", myData.iSongLength);
	
	g_free (myData.cPlayingUri);
	value = (const char *) g_hash_table_lookup (data_list, "~filename");   // location ?
	if (value != NULL)
		myData.cPlayingUri = g_strdup (value);
	else
		myData.cPlayingUri = NULL;
	g_print ("  cUri <- %s\n", myData.cPlayingUri);
	
	cd_musicplayer_get_cover_path (NULL, TRUE);
}

void cd_quodlibet_getSongInfos (void)
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

/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	g_print ("MP : %s ()\n", __func__);
	
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

/* Fonction executée à chaque changement "paused".
 */
static void onChangePlaying (DBusGProxy *player_proxy, gpointer data)  // paused
{
	g_print ("MP : %s ()\n", __func__);
	myData.bIsRunning = TRUE;
	
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
}
/* Fonction executée à chaque changement "unpaused".
 */
static void onChangePlaying2 (DBusGProxy *player_proxy, gpointer data)  // unpaused
{
	g_print ("MP : %s ()\n", __func__);
	myData.bIsRunning = TRUE;
	
	myData.iPlayingStatus = PLAYER_PLAYING;
	
	cd_musicplayer_relaunch_handler ();
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
}

////////////////////////////
// Definition du backend. //
////////////////////////////

/* Fonction de connexion au bus de QL.
 */
static gboolean _cd_quodlibet_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // cree le proxy.

		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "paused",
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "paused",
			G_CALLBACK(onChangePlaying), NULL, NULL);
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "unpaused",
			G_TYPE_NONE,
			G_TYPE_INVALID);  // idem.
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "unpaused",
			G_CALLBACK(onChangePlaying2), NULL, NULL);
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "song-started",
			QL_DBUS_TYPE_SONG_METADATA,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "song-started",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par le backend.
 */
void cd_quodlibet_free_data (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "paused",
			G_CALLBACK(onChangePlaying), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "unpaused",
			G_CALLBACK(onChangePlaying2), NULL);
				
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "song-started",
			G_CALLBACK(onChangeSong), NULL);
	}
	musicplayer_dbus_disconnect_from_bus();
}

/* Controle du lecteur
 */
void cd_quodlibet_control (MyPlayerControl pControl, const char* song)
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
		cd_debug ("MP : Handeler QuodLibet : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}

/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_quodlibet_read_data (void)
{
	if (myData.dbus_enable)
	{
		if (myData.bIsRunning)
		{
			if (myData.iPlayingStatus == PLAYER_PLAYING)
				_quodlibet_get_time_elapsed ();
			else if (myData.iPlayingStatus != PLAYER_PAUSED)  // en pause le temps reste constant.
				myData.iCurrentTime = 0;
		}
		else 
		{
			myData.iCurrentTime = 0;
		}
		cd_message (" myData.iCurrentTime <- %d", __func__, myData.iCurrentTime);
	}
}

/* Initialise le backend de QL.
 */
static void cd_quodlibet_configure (void)
{
	myData.DBus_commands.service = "net.sacredchao.QuodLibet";
	myData.DBus_commands.path = "/net/sacredchao/QuodLibet";
	myData.DBus_commands.interface = "net.sacredchao.QuodLibet";
	
	myData.dbus_enable = _cd_quodlibet_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de QL.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de QL sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			g_print ("MP : QL is running\n");
			_quodlibet_getPlaying ();
			cd_quodlibet_getSongInfos ();
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


/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_quodlibet_handler (void)
{
	MusicPlayerHandeler *pQuodlibet = g_new0 (MusicPlayerHandeler, 1);
	pQuodlibet->read_data = cd_quodlibet_read_data;
	pQuodlibet->free_data = cd_quodlibet_free_data;
	pQuodlibet->configure = cd_quodlibet_configure;  // renseigne les proprietes DBus et se connecte au bus.
	pQuodlibet->control = cd_quodlibet_control;
	pQuodlibet->get_cover = NULL;
	pQuodlibet->cCoverDir = NULL;  /// il me semble que QL gere les pochettes ...
	
	pQuodlibet->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT;
	pQuodlibet->appclass = "quodlibet";
	pQuodlibet->launch = "quodlibet";
	pQuodlibet->name = "QuodLibet";
	pQuodlibet->iPlayer = MP_QUODLIBET;
	pQuodlibet->bSeparateAcquisition = FALSE;
	pQuodlibet->iLevel = PLAYER_GOOD;  // n'a besoin d'une boucle que pour afficher le temps ecoule.
	cd_musicplayer_register_my_handler (pQuodlibet, "QuodLibet");
}

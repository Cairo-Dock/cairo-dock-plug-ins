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
#include "applet-mpris2.h"

/*
Interface MediaPlayer2

Methods
Raise () -> nothing  
Quit () -> nothing  

Properties
CanQuit b  Read only   
CanRaise b  Read only   
HasTrackList b  Read only   
Identity s  Read only   
DesktopEntry s  Read only   
SupportedUriSchemes as  Read only   
SupportedMimeTypes as  Read only 

Interface MediaPlayer2.Player

Methods
Next () -> nothing  
Previous () -> nothing  
Pause () -> nothing  
PlayPause () -> nothing  
Stop () -> nothing  
Play () -> nothing  
Seek (x: Offset) -> nothing  
SetPosition (o: TrackId, x: Position) -> nothing  
OpenUri (s: Uri) -> nothing  

Signals
Seeked (x: Position)  

Properties
PlaybackStatus s (Playback_Status)  Read only   
LoopStatus s (Loop_Status)  Read/Write   
Rate d (Playback_Rate)  Read/Write   
Shuffle b  Read/Write   
Metadata a{sv} (Metadata_Map)  Read only   
Volume d (Volume)  Read/Write   
Position x (Time_In_Us)  Read only   
MinimumRate d (Playback_Rate)  Read only   
MaximumRate d (Playback_Rate)  Read only   
CanGoNext b  Read only   
CanGoPrevious b  Read only   
CanPlay b  Read only   
CanPause b  Read only   
CanSeek b  Read only   
CanControl b  Read only   

Types
Track_Id Simple Type o  
Playback_Rate Simple Type d  
Volume Simple Type d  
Time_In_Us Simple Type x  
Playback_Status Enum s  
Loop_Status Enum s 

*/

  /////////////////////////////////
 // Les Fonctions propres a MP. //
/////////////////////////////////
static gboolean s_bIsLoop = FALSE;
// static gboolean s_bGotLoopStatus = FALSE;
static gboolean s_bIsShuffle = FALSE;
// static gboolean s_bGotShuffleStatus = FALSE;
static gboolean s_bCanRaise = FALSE;
// static gboolean s_bGotCanRaise = FALSE;
static gboolean s_bCanQuit = FALSE;
// static gboolean s_bGotCanQuit = FALSE;
static gboolean s_bCanPlay = TRUE;
static gboolean s_bCanPause = TRUE;
static gboolean s_bCanGoNext = TRUE;
static gboolean s_bCanGoPrev = TRUE;
static gboolean s_bCanControl = TRUE;
static gboolean s_bHasTrackList = FALSE;
static gdouble s_fVolume = 0.0;

static gboolean get_loop_status (void)
{
	return s_bIsLoop;
}

static gboolean get_shuffle_status (void)
{
	return s_bIsShuffle;
}

static gboolean _raise (void)
{
	if (s_bCanRaise)
	{
		g_dbus_proxy_call (myData.pProxyMain, "Raise", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		return TRUE;
	}
	else return FALSE;
}

static gboolean _quit (void)
{
	if (s_bCanQuit)
	{
		g_dbus_proxy_call (myData.pProxyMain, "Quit", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		return TRUE;
	}
	else return FALSE;
}

static MyPlayerStatus _extract_status (const gchar *cStatus)
{
	if (cStatus == NULL)
		return PLAYER_BROKEN;
	if (strcmp (cStatus, "Playing") == 0)
		return PLAYER_PLAYING;
	if (strcmp (cStatus, "Paused") == 0)
		return PLAYER_PAUSED;
	if (strcmp (cStatus, "Stopped") == 0)
		return PLAYER_STOPPED;
	return PLAYER_BROKEN;
}

static void cd_mpris2_get_time_elapsed (void)
{
	// this function runs in a separate thread, we can just use the _sync method and wait
	GVariant *res = g_dbus_connection_call_sync (g_dbus_proxy_get_connection (myData.pProxyPlayer),
		myData.pCurrentHandler->cMprisService, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties",
		"Get", g_variant_new ("(ss)", CD_MPRIS2_PLAYER_IFACE, "Position"), G_VARIANT_TYPE ("(v)"),
		G_DBUS_CALL_FLAGS_NONE, 500, NULL, NULL);
	
	if (res)
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("x")))
			myData.iCurrentTime = g_variant_get_int64 (tmp2) / 1e6;
		else if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("t")))
			myData.iCurrentTime = g_variant_get_uint64 (tmp2) / 1e6;
		else if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("i")))
			myData.iCurrentTime = g_variant_get_int32 (tmp2) / 1e6;
		else myData.iCurrentTime = -1;
		//!! TODO: gmusicbrowser v1.1.7 -> string; show a warning the first time?
		
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
		g_variant_unref (res);
	}
	else myData.iCurrentTime = -1;
}

static gboolean _is_a_new_track (const gchar *cTrackID)
{
	cd_message ("  TrackId <- %s (was: %s)", cTrackID, myData.cTrackID);
	if (g_strcmp0 (myData.cTrackID, cTrackID))  // track has changed.
	{
		g_free (myData.cTrackID);
		myData.cTrackID = g_strdup (cTrackID);
		return TRUE;
	}
	return FALSE;
}

static gboolean _extract_metadata (GVariant *v1)
{
	gboolean bTrackHasChanged = FALSE;
	GVariantDict *pMetadata = g_variant_dict_new (v1);
	GVariant *v;
	
	// a string or a dbus object path that uniquely identifies the track within the scope of the playlist
	v = g_variant_dict_lookup_value (pMetadata, "mpris:trackid", NULL);
	if (v != NULL)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("s")) || g_variant_is_of_type (v, G_VARIANT_TYPE ("o")))
			bTrackHasChanged = _is_a_new_track (g_variant_get_string (v, NULL));
		else cd_warning ("Unexpected type for 'mpris:trackid': %s", g_variant_get_type_string (v));
		g_variant_unref (v);
	}

	v = g_variant_dict_lookup_value (pMetadata, "mpris:length", NULL);
	if (v != NULL)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE("x"))) // should be a int64
			myData.iSongLength = g_variant_get_int64 (v) / 1000000;
		else if (g_variant_is_of_type (v, G_VARIANT_TYPE("i"))) // but some players doesn't respect that... maybe a limitation?
			myData.iSongLength = g_variant_get_int32 (v) / 1000000;
		else
			cd_warning ("Length has a wrong type: %s", g_variant_get_type_string (v));
		cd_debug ("Length: %d", myData.iSongLength);
		g_variant_unref (v);
	}

	gchar *cOldArtist = myData.cArtist;
	myData.cArtist = NULL;
	const gchar **artists = NULL;
	if (g_variant_dict_lookup (pMetadata, "xesam:artist", "^a&s", &artists) && artists != NULL)
	{
		myData.cArtist = g_strjoinv ("; ", (gchar**)artists);
		g_free (artists); // we need to free the vector, but not the individual strings
	}
	cd_message ("  cArtist <- %s", myData.cArtist);
	
	// maybe the user has renamed the tags of the current song...
	if (! bTrackHasChanged && g_strcmp0 (myData.cArtist, cOldArtist))
		bTrackHasChanged = TRUE;
	g_free (cOldArtist);
	
	g_free (myData.cAlbum);
	myData.cAlbum = NULL;
	g_variant_dict_lookup (pMetadata, "xesam:album", "s", &myData.cAlbum);
	if (myData.cAlbum && !*myData.cAlbum)
	{
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
	}
	cd_message ("  cAlbum <- %s", myData.cAlbum);

	gchar *cOldTitle = myData.cTitle;
	myData.cTitle = NULL;
	g_variant_dict_lookup (pMetadata, "xesam:title", "s", &myData.cTitle);
	if (myData.cTitle && !*myData.cTitle)
	{
		g_free (myData.cTitle);
		myData.cTitle = NULL;
	}
	cd_message ("  cTitle <- %s", myData.cTitle);

	/* some players doesn't support (well) the trackid. Even if this is not our
	 * problem, it can be interesting to also check if the title has changed.
	 */
	if (! bTrackHasChanged && g_strcmp0 (myData.cTitle, cOldTitle))
		bTrackHasChanged = TRUE;
	g_free (cOldTitle);
	
	g_free (myData.cPlayingUri);
	myData.cPlayingUri = NULL;
	g_variant_dict_lookup (pMetadata, "xesam:url", "s", &myData.cPlayingUri);
	if (myData.cPlayingUri && !*myData.cPlayingUri)
	{
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
	}
	cd_message ("  cUri <- %s", myData.cPlayingUri);

	myData.iTrackNumber = 0;  // not really useful, it's the track-number in the album.
	g_variant_dict_lookup (pMetadata, "xesam:trackNumber", "i", &myData.iTrackNumber);
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);

	const gchar *cCoverPath = NULL;
	g_variant_dict_lookup (pMetadata, "mpris:artUrl", "&s", &cCoverPath);
	if (cCoverPath && *cCoverPath) cCoverPath = NULL;
	cd_musicplayer_set_cover_path (cCoverPath);  // do it at the end (we have to know the artist and the album if (cCoverPath == NULL))

	/// we miss iTrackListIndex and tracklist-length ...
	
	g_variant_dict_unref (pMetadata);
	return bTrackHasChanged;
}

static void _reset_metadata (void)
{
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

static void _mpris2_started (void)
{
	// Get all relevant properties
	GVariant *v;
	
	// main proxy: CanQuit and Can Raise
	v = g_dbus_proxy_get_cached_property (myData.pProxyMain, "CanQuit");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
		s_bCanQuit = g_variant_get_boolean (v);
	else cd_warning ("Cannot get 'CanQuit' property");
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyMain, "CanRaise");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
		s_bCanRaise = g_variant_get_boolean (v);
	else cd_warning ("Cannot get 'CanRaise' property");
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyMain, "HasTrackList");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
		s_bHasTrackList = g_variant_get_boolean (v);
	else cd_warning ("Cannot get 'HasTrackList' property");
	if (v) g_variant_unref (v);
	
	//!! TODO: set icon name based on "Identity" property? + match desktop file (DesktopEntry)
	/// or these are done earlier?
	
	// player proxy: status, current track and capabilities
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "CanControl");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
		s_bCanControl = g_variant_get_boolean (v);
	else cd_warning ("Cannot get 'CanControl' property");
	if (v) g_variant_unref (v);
	
	if (s_bCanControl)
	{
		v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "CanGoNext");
		if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
			s_bCanGoNext = g_variant_get_boolean (v);
		else cd_warning ("Cannot get 'CanGoNext' property");
		if (v) g_variant_unref (v);
		
		v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "CanGoPrev");
		if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
			s_bCanGoPrev = g_variant_get_boolean (v);
		else cd_warning ("Cannot get 'CanGoPrev' property");
		if (v) g_variant_unref (v);
		
		v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "CanPause");
		if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
			s_bCanPause = g_variant_get_boolean (v);
		else cd_warning ("Cannot get 'CanPause' property");
		if (v) g_variant_unref (v);
		
		v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "CanPlay");
		if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
			s_bCanPlay = g_variant_get_boolean (v);
		else cd_warning ("Cannot get 'CanPlay' property");
		if (v) g_variant_unref (v);
	}
	else
	{
		s_bCanGoNext = FALSE;
		s_bCanGoPrev = FALSE;
		s_bCanPause = FALSE;
		s_bCanPlay = FALSE;
	}
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "Shuffle");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
		s_bIsShuffle = g_variant_get_boolean (v);
	else cd_warning ("Cannot get 'Shuffle' property");
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "LoopStatus");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("s")))
		s_bIsLoop = !strcmp (g_variant_get_string (v, NULL), "Playlist"); // _get_string () returns non-NULL always
	else cd_warning ("Cannot get 'LoopStatus' property");
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "PlaybackStatus");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("s")))
		myData.iPlayingStatus = _extract_status (g_variant_get_string (v, NULL));
	else
	{
		cd_warning ("Cannot get 'PlaybackStatus' property");
		myData.iPlayingStatus = PLAYER_BROKEN;
	}
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "Metadata");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("a{sv}")))
		_extract_metadata (v);
	else
		cd_warning ("Cannot get 'Metadata' property");
	if (v) g_variant_unref (v);
	
	v = g_dbus_proxy_get_cached_property (myData.pProxyPlayer, "Volume");
	if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("d")))
		s_fVolume = g_variant_get_double (v);
	else
		cd_warning ("Cannot get 'Volume' property");
	if (v) g_variant_unref (v);
}




  ////////////////
 // Callbacks. //
////////////////

static void _main_prop_changed (G_GNUC_UNUSED GDBusProxy *pProxy, GVariant *pChanged,
	G_GNUC_UNUSED char** invalidated_properties, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	g_variant_lookup (pChanged, "CanQuit", "b", &s_bCanQuit);
	g_variant_lookup (pChanged, "CanRaise", "b", &s_bCanRaise);
	CD_APPLET_LEAVE ();
}

static void _player_prop_changed (G_GNUC_UNUSED GDBusProxy *pProxy, GVariant *pChanged,
	G_GNUC_UNUSED char** invalidated_properties, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GVariantDict *pDict = g_variant_dict_new (pChanged);
	
	// capabilities
	g_variant_dict_lookup (pDict, "CanControl", "b", &s_bCanControl);
	if (s_bCanControl)
	{
		g_variant_dict_lookup (pDict, "CanPlay", "b", &s_bCanPlay);
		g_variant_dict_lookup (pDict, "CanPause", "b", &s_bCanPause);
		g_variant_dict_lookup (pDict, "CanGoNext", "b", &s_bCanGoNext);
		g_variant_dict_lookup (pDict, "CanGoPrev", "b", &s_bCanGoPrev);
	}
	else
	{
		s_bCanPlay = FALSE;
		s_bCanPause = FALSE;
		s_bCanGoNext = FALSE;
		s_bCanGoPrev = FALSE;
	}
	
	// state
	const gchar *tmp = NULL;
	if (g_variant_dict_lookup (pDict, "PlaybackStatus", "&s", &tmp) && tmp && *tmp)
	{
		myData.iPlayingStatus = _extract_status (tmp);
		if (myData.iPlayingStatus == PLAYER_PLAYING) cd_musicplayer_relaunch_handler (); // update position every second
		cd_musicplayer_update_icon ();
	}
	tmp = NULL;
	
	GVariant *v = g_variant_dict_lookup_value (pDict, "Metadata", G_VARIANT_TYPE ("a{sv}"));
	if (v)
	{
		gboolean bTrackHasChanged = _extract_metadata (v);
		
		if (bTrackHasChanged)  // new song (song changed or started playing)
		{
			myData.iPlayingStatus = PLAYER_PLAYING;  // pour les lecteurs bugues comme Exaile qui envoit un statut "stop" au changement de musique sans envoyer de status "play" par la suite. On considere donc que si le lecteur joue une nouvelle musique, c'est qu'il est en "play".
			cd_musicplayer_update_icon ();
		}
	}
	if (g_variant_dict_lookup (pDict, "LoopStatus", "&s", &tmp) && tmp && *tmp)
		s_bIsLoop = !strcmp (tmp, "Playlist");
	
	g_variant_dict_lookup (pDict, "Shuffle", "b", &s_bIsShuffle);
	g_variant_dict_lookup (pDict, "Volume", "d", &s_fVolume);
	
	// note: Position is not updated, need to query it regularly
	
	CD_APPLET_LEAVE ();
}

  ////////////////////////
 // backend definition //
////////////////////////

/* Permet de liberer la memoire prise par le backend.
 */
static void cd_mpris2_stop (void)
{
	if (myData.pCancel)
	{
		g_cancellable_cancel (myData.pCancel);
		g_object_unref (G_OBJECT (myData.pCancel));
		myData.pCancel = NULL;
	}
	if (myData.pProxyMain) g_object_unref (G_OBJECT (myData.pProxyMain));
	if (myData.pProxyPlayer) g_object_unref (G_OBJECT (myData.pProxyPlayer));
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur).
 */
static void cd_mpris2_control (MyPlayerControl pControl, const char* song)
{
	switch (pControl)
	{
		case PLAYER_PREVIOUS :
			if (s_bCanGoPrev) g_dbus_proxy_call (myData.pProxyPlayer, "Previous",
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_STOP :
			// no CanStop property...
			if (s_bCanControl) g_dbus_proxy_call (myData.pProxyPlayer, "Stop",
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_PLAY_PAUSE :
			if (s_bCanPause) g_dbus_proxy_call (myData.pProxyPlayer, "PlayPause",
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_NEXT :
			if (s_bCanGoNext) g_dbus_proxy_call (myData.pProxyPlayer, "Next",
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_JUMPBOX :
			
		break;
		
		case PLAYER_SHUFFLE :
			// note: previously, we would re-read the current shuffle status, not sure if that is necessary though
			g_dbus_connection_call (g_dbus_proxy_get_connection (myData.pProxyPlayer),
				myData.pCurrentHandler->cMprisService, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties", "Set",
				g_variant_new ("(ssv)", CD_MPRIS2_PLAYER_IFACE, "Shuffle", g_variant_new_boolean (!s_bIsShuffle)),
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_REPEAT :
			// note: previously, we would re-read the current loop status, not sure if that is necessary though
			g_dbus_connection_call (g_dbus_proxy_get_connection (myData.pProxyPlayer),
				myData.pCurrentHandler->cMprisService, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties", "Set",
				g_variant_new ("(ssv)", CD_MPRIS2_PLAYER_IFACE, "LoopStatus",
					g_variant_new_string (s_bIsLoop ? "None" : "Playlist")),
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		break;
		
		case PLAYER_ENQUEUE :
		{
			cd_debug ("enqueue %s", song);
			if (s_bHasTrackList) // note: we don't need a separate proxy to the TrackList interface
				g_dbus_connection_call (g_dbus_proxy_get_connection (myData.pProxyPlayer),
					myData.pCurrentHandler->cMprisService, CD_MPRIS2_OBJ, "org.mpris.MediaPlayer2.TrackList",
					"AddTrack", g_variant_new ("(sob)", song, "", TRUE), NULL, G_DBUS_CALL_FLAGS_NONE,
					-1, NULL, NULL, NULL); //!! TODO: check if empty object path actually works !!
			else g_dbus_proxy_call (myData.pProxyPlayer, "OpenUri", g_variant_new ("(s)", song),
				G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		}
		break;
		
		case PLAYER_VOLUME :
		{
			double fVolume = s_fVolume;  // [0, 1]
			if (song && strcmp (song, "up") == 0)
				fVolume += .05;
			else
				fVolume -= .05;
			if (fVolume > 1) fVolume = 1;
			if (fVolume < 0) fVolume = 0;
			cd_debug ("volume <- %f", fVolume);
			g_dbus_connection_call (g_dbus_proxy_get_connection (myData.pProxyPlayer),
				myData.pCurrentHandler->cMprisService, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties", "Set",
				g_variant_new ("(ssv)", CD_MPRIS2_PLAYER_IFACE, "Volume", g_variant_new_double (fVolume)),
				NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
		}
		
		default :
		break;
	}
}


/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_mpris2_get_data (void)
{
	if (myData.iPlayingStatus == PLAYER_PLAYING)
	{
		cd_mpris2_get_time_elapsed ();
		if (myData.iCurrentTime < 0)  // aucune info de temps sur le bus => lecteur ferme.
		{
			myData.iGetTimeFailed ++;  // certains lecteurs (qmmp par exemple) envoient le signal 'playing' trop tot lorsqu'on les relance, ils ne fournissent pas de duree tout de suite, et donc l'applet stoppe. On fait donc 3 tentatives avant de declarer le lecteur ferme.
			cd_debug ("failed to get time %d time(s)", myData.iGetTimeFailed);
			if (myData.iGetTimeFailed > 2)
			{
				cd_debug (" => player is likely closed");
				myData.iPlayingStatus = PLAYER_NONE;
				myData.iCurrentTime = -2;  // le temps etait a -1, on le change pour provoquer un redraw.
			}
		}
		else
			myData.iGetTimeFailed = 0;
	}
	else if (myData.iPlayingStatus != PLAYER_PAUSED)  // en pause le temps reste constant.
	{
		myData.iCurrentTime = 0;
		myData.iGetTimeFailed = 0;
	}
}

static void _got_main_proxy (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot connect to MPRIS2 interface: %s", err->message);
			//!! TODO: handle error and stop backend
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	myData.pProxyMain = pProxy;
	
	// next: connect to properties changed signal
	g_signal_connect (pProxy, "g-properties-changed", G_CALLBACK (_main_prop_changed), NULL);
	
	if (myData.pProxyPlayer) _mpris2_started (); // get initial properties
	
	CD_APPLET_LEAVE ();
}

static void _got_player_proxy (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot connect to MPRIS2 interface: %s", err->message);
			//!! TODO: handle error and stop backend
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	myData.pProxyPlayer = pProxy;
	
	// next: connect to properties changed signal
	g_signal_connect (pProxy, "g-properties-changed", G_CALLBACK (_player_prop_changed), NULL);
	
	if (myData.pProxyMain) _mpris2_started (); // get initial properties
	
	CD_APPLET_LEAVE ();
}

static void cd_mpris2_start (void)
{
	myData.pCancel = g_cancellable_new (); // should be NULL at this point, maybe check?
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS // no signals (assuming that we still get property notifications)
		| G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES, // we want to always get newest state
		NULL, // interface info -- maybe we could supply it since it is known
		myData.pCurrentHandler->cMprisService, // maybe this should be a parameter
		CD_MPRIS2_OBJ, // "/org/mpris/MediaPlayer2"
		CD_MPRIS2_MAIN_IFACE, // "org.mpris.MediaPlayer2"
		myData.pCancel,
		_got_main_proxy,
		NULL); // maybe myApplet (if we allow multiple instance)
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS // only "Seeked" signal, we don't care
		| G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES, // we want to always get newest state
		NULL, // interface info -- maybe we could supply it since it is known
		myData.pCurrentHandler->cMprisService, // maybe this should be a parameter
		CD_MPRIS2_OBJ, // "/org/mpris/MediaPlayer2"
		CD_MPRIS2_PLAYER_IFACE, // "org.mpris.MediaPlayer2.Player"
		myData.pCancel,
		_got_player_proxy,
		NULL); // maybe myApplet (if we allow multiple instance)
	
	// reset to the default state
	s_bCanQuit = FALSE;
	s_bCanRaise = FALSE;
	s_bCanPlay = TRUE;
	s_bCanPause = TRUE;
	s_bCanGoNext = TRUE;
	s_bCanGoPrev = TRUE;
	s_bCanControl = TRUE;
	s_bHasTrackList = FALSE;
	myData.iTrackListLength = 0;
	myData.iTrackListIndex = 0;
	s_bIsLoop = FALSE;
	s_bIsShuffle = FALSE;
	_reset_metadata ();
}

void cd_musicplayer_register_mpris2_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Mpris2";
	pHandler->get_data = cd_mpris2_get_data;
	pHandler->stop = cd_mpris2_stop;
	pHandler->start = cd_mpris2_start;
	pHandler->control = cd_mpris2_control;
	pHandler->get_loop_status = get_loop_status;
	pHandler->get_shuffle_status = get_shuffle_status;
	pHandler->raise = _raise;
	pHandler->quit = _quit;
	pHandler->bSeparateAcquisition = TRUE;
	pHandler->iLevel = PLAYER_GOOD;
	
	pHandler->cMprisService = NULL;  // service is left NULL until an actual MPRIS2 player is present.
	pHandler->path = "/org/mpris/MediaPlayer2";
	pHandler->interface = DBUS_INTERFACE_PROPERTIES;  // "org.mpris.MediaPlayer2.Player"
	pHandler->path2 = "/org/mpris/MediaPlayer2";
	pHandler->interface2 = "org.mpris.MediaPlayer2.Player";
	
	pHandler->appclass = NULL;  // will be filled later.
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE | PLAYER_VOLUME;
	cd_musicplayer_register_my_handler (pHandler);
}


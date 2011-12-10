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

static DBusGProxyCall *s_pGetSongInfosCall = NULL;
static DBusGProxyCall *s_pGetStatusCall = NULL;

static void cd_mpris2_getSongInfos_async (void);

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

static void _on_got_playing_status (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	s_pGetStatusCall = NULL;
	
	gchar *cStatus = NULL;
	GValue v = G_VALUE_INIT;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_VALUE, &v,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't get MPRIS status (%s)\n", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		if (G_VALUE_HOLDS_STRING (&v))
		{
			cStatus = (gchar*)g_value_get_string (&v);
			myData.iPlayingStatus = _extract_status (cStatus);
			g_free (cStatus);  // since we don't destroy the value, we destroy its content.
		}
	}
	
	cd_mpris2_getSongInfos_async ();
	
	CD_APPLET_LEAVE ();
}
static void cd_mpris2_getPlaying_async (void)  // used by Audacious.
{
	if (s_pGetStatusCall != NULL)
		return;
	s_pGetStatusCall = dbus_g_proxy_begin_call (myData.dbus_proxy_player,
		"Get",
		(DBusGProxyCallNotify)_on_got_playing_status,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_STRING, "org.mpris.MediaPlayer2.Player",
		G_TYPE_STRING, "PlaybackStatus",
		G_TYPE_INVALID);
}

static gboolean cd_mpris2_is_loop (void)
{
	gchar *cLoopStatus = cairo_dock_dbus_get_property_as_string (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "LoopStatus");
	gboolean bLoop = (cLoopStatus != NULL && strcmp (cLoopStatus, "Playlist") == 0);
	g_free (cLoopStatus);
	return bLoop;
}

static gboolean cd_mpris2_is_shuffle (void)
{
	return cairo_dock_dbus_get_property_as_boolean (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "Shuffle");
}

static void cd_mpris2_get_time_elapsed (void)
{
	GValue v = G_VALUE_INIT;
	cairo_dock_dbus_get_property_in_value (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "Position", &v);
	if (G_VALUE_HOLDS_INT64 (&v))
	{
		myData.iCurrentTime =  g_value_get_int64 (&v) / 1e6;
	}
	else if (G_VALUE_HOLDS_UINT64 (&v))
	{
		myData.iCurrentTime =  g_value_get_uint64 (&v) / 1e6;
	}
	else
	{
		cd_warning ("wrong type for the 'Position' property, please report this bug to the %s team", myData.pCurrentHandler->appclass);
		myData.iCurrentTime = -1;
	}
}

/*static gboolean _can_control (void)
{
	return cairo_dock_dbus_get_property_as_int (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "CanControl");
}*/

static double cd_mpris2_get_volume (void)
{
	return cairo_dock_dbus_get_property_as_double (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "Volume");
}

static gboolean _extract_metadata (GHashTable *pMetadata)
{
	gboolean bTrackHasChanged = FALSE;
	GValue *v;
	const gchar *str;
	
	v = g_hash_table_lookup (pMetadata, "mpris:trackid");  // a string that uniquely identifies the track within the scope of the playlist
	if (v != NULL && G_VALUE_HOLDS_STRING (v))
	{
		const gchar *cTrackID = g_value_get_string (v);
		if (cairo_dock_strings_differ (myData.cTrackID, cTrackID))  // track has changed.
		{
			g_free (myData.cTrackID);
			myData.cTrackID = g_strdup (cTrackID);
			bTrackHasChanged = TRUE;
		}
	}

	v = g_hash_table_lookup (pMetadata, "mpris:length");  // length of the track, in microseconds (signed 64-bit integer)
	if (v != NULL && G_VALUE_HOLDS_INT64 (v))
	{
		myData.iSongLength = g_value_get_int64 (v) / 1000000;
		cd_debug ("Length: %d", myData.iSongLength);
	}

	g_free (myData.cArtist);
	myData.cArtist = NULL;
	v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:artist");
	if (v != NULL && G_VALUE_HOLDS(v, G_TYPE_STRV))
	{
		gchar **artists = g_value_get_boxed(v);
		if (artists != NULL)
			myData.cArtist = g_strjoinv (NULL, artists);
	}
	cd_message ("  cArtist <- %s", myData.cArtist);
	
	g_free (myData.cAlbum);
	myData.cAlbum = NULL;
	v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:album");
	if (v != NULL && G_VALUE_HOLDS_STRING(v))
	{
		str = g_value_get_string(v);
		if (str && *str != '\0')
			myData.cAlbum = g_strdup (str);
	}
	cd_message ("  cAlbum <- %s", myData.cAlbum);
	
	g_free (myData.cTitle);
	myData.cTitle = NULL;
	v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:title");
	if (v != NULL && G_VALUE_HOLDS_STRING(v))
	{
		str = g_value_get_string(v);
		if (str && *str != '\0')
			myData.cTitle = g_strdup (str);
	}
	cd_message ("  cTitle <- %s", myData.cTitle);
	
	g_free (myData.cPlayingUri);
	myData.cPlayingUri = NULL;
	v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:url");
	if (!v)
		v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:uri");
	if (v != NULL && G_VALUE_HOLDS_STRING(v))
	{
		str = g_value_get_string(v);
		if (str && *str != '\0')
			myData.cPlayingUri = g_strdup (str);
	}
	cd_message ("  cUri <- %s", myData.cPlayingUri);

	myData.iTrackNumber = 0;  // not really useful, it's the track-number in the album.
	v = (GValue *) g_hash_table_lookup(pMetadata, "xesam:trackNumber");
	if (v != NULL && G_VALUE_HOLDS_INT(v))
	{
		myData.iTrackNumber = g_value_get_int (v);
	}
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);

	const gchar *cCoverPath = NULL;
	v = g_hash_table_lookup(pMetadata, "mpris:artUrl");
	if (v != NULL && G_VALUE_HOLDS_STRING(v))
	{
		cCoverPath = g_value_get_string(v);
	}
	cd_musicplayer_get_cover_path (cCoverPath, TRUE); // did it at the end (we have to know the artist and the album if (cCoverPath == NULL))

	/// we miss iTrackListIndex and tracklist-length ...
	
	
	return bTrackHasChanged;
}

static void _on_got_song_infos (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	s_pGetSongInfosCall = NULL;
	
	GHashTable *pMetadata = NULL;
	GValue v = G_VALUE_INIT;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_VALUE, &v,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't get MPRIS song infos (%s)\n", erreur->message);
		g_error_free (erreur);
		pMetadata = NULL;
	}
	else
	{
		if (G_VALUE_HOLDS_BOXED (&v))
		{
			pMetadata = g_value_get_boxed (&v);  // since we don't destroy the value, we'll take care of the hash-table when we're done with it.
		}
	}
	if (pMetadata != NULL)
	{
		_extract_metadata (pMetadata);
		
		g_hash_table_destroy (pMetadata);
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
		g_free (myData.cArtist);
		myData.cArtist = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
		myData.iSongLength = 0;
		myData.iTrackNumber = 0;
		myData.cover_exist = FALSE;
	}
	
	cd_musicplayer_update_icon (TRUE);
	cd_musicplayer_relaunch_handler ();
	
	CD_APPLET_LEAVE ();
}
static void cd_mpris2_getSongInfos_async (void)
{
	if (s_pGetSongInfosCall != NULL)
		return;
	s_pGetSongInfosCall = dbus_g_proxy_begin_call (myData.dbus_proxy_player,
		"Get",
		(DBusGProxyCallNotify)_on_got_song_infos,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_STRING, "org.mpris.MediaPlayer2.Player",
		G_TYPE_STRING, "Metadata",
		G_TYPE_INVALID);
}


  ////////////////
 // Callbacks. //
////////////////

static void on_properties_changed (DBusGProxy *player_proxy, const gchar *cInterface, GHashTable *pChangedProps, const gchar **cInvalidProps, gpointer data)
{
	g_return_if_fail (cInterface != NULL);
	cd_debug ("");
	GValue *v;
	if (strcmp (cInterface, "org.mpris.MediaPlayer2.Player") == 0)
	{
		v = g_hash_table_lookup (pChangedProps, "PlaybackStatus");
		if (v != NULL && G_VALUE_HOLDS_STRING (v))
		{
			const gchar *cStatus = g_value_get_string (v);  // "Playing", "Paused" or "Stopped"
			myData.iPlayingStatus = _extract_status (cStatus);
			cd_debug ("PlaybackStatus: Status: %s, %d", cStatus, myData.iPlayingStatus);
			
			if (myData.iPlayingStatus == PLAYER_PLAYING)  // le handler est stoppe lorsque le lecteur ne joue rien.
				cd_musicplayer_relaunch_handler ();
			
			if (! myData.cover_exist && (myData.cPlayingUri != NULL || myData.cTitle != NULL))
			{
				cd_musicplayer_set_surface (myData.iPlayingStatus);
				cd_debug ("cover (%d), cPlayingUri (%s), cTitle (%s)", myData.cover_exist, myData.cPlayingUri, myData.cTitle);
			}
			else
			{
				CD_APPLET_REDRAW_MY_ICON;
			}
		}
		
		v = g_hash_table_lookup (pChangedProps, "Metadata");
		if (v != NULL && G_VALUE_HOLDS_BOXED (v))
		{
			GHashTable *pMetadata = g_value_get_boxed (v);
			gboolean bTrackHasChanged = _extract_metadata (pMetadata);
			
			if (bTrackHasChanged)
			{
				myData.iPlayingStatus = PLAYER_PLAYING;  // pour les lecteurs bugues comme Exaile qui envoit un statut "stop" au changement de musique sans envoyer de status "play" par la suite. On considere donc que si le lecteur joue une nouvelle musique, c'est qu'il est en "play".
				cd_musicplayer_update_icon (TRUE);
			}
			else if (myData.iPlayingStatus == PLAYER_STOPPED)
				cd_musicplayer_update_icon (TRUE); // Force the update of the icon if the player is now stopped.
		}
	}
	else /*if (strcmp (cInterface, "org.mpris.MediaPlayer2.TrackList") == 0)
	{
		
	}*/
		cd_debug ("Another interface: %s", cInterface);
}

  ////////////////////////
 // backend definition //
////////////////////////

/* Permet de liberer la memoire prise par le backend.
 */
static void cd_mpris2_stop (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		if (s_pGetSongInfosCall != NULL)
		{
			dbus_g_proxy_cancel_call (myData.dbus_proxy_player, s_pGetSongInfosCall);
			s_pGetSongInfosCall = NULL;
		}
		
		if (s_pGetStatusCall != NULL)
		{
			dbus_g_proxy_cancel_call (myData.dbus_proxy_player, s_pGetStatusCall);
			s_pGetStatusCall = NULL;
		}
	}
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur).
 */
static void cd_mpris2_control (MyPlayerControl pControl, const char* song)
{
	static GValue s_pValue = G_VALUE_INIT;
	gboolean bToggleValue;
	switch (pControl)
	{
		case PLAYER_PREVIOUS :
			cairo_dock_dbus_call (myData.dbus_proxy_shell, "Previous");
		break;
		
		case PLAYER_STOP :
			cairo_dock_dbus_call (myData.dbus_proxy_shell, "Stop");
		break;
		
		case PLAYER_PLAY_PAUSE :
			if (myData.iPlayingStatus != PLAYER_PLAYING)
				cairo_dock_dbus_call (myData.dbus_proxy_shell, "Play");
			else
				cairo_dock_dbus_call (myData.dbus_proxy_shell, "Pause");
		break;
		
		case PLAYER_NEXT :
			cairo_dock_dbus_call (myData.dbus_proxy_shell, "Next");
		break;
		
		case PLAYER_JUMPBOX :
			
		break;
		
		case PLAYER_SHUFFLE :
			bToggleValue = cd_mpris2_is_shuffle ();
			cd_debug ("SetRandom <- %d\n", !bToggleValue);
			g_value_init (&s_pValue, G_TYPE_BOOLEAN);
			g_value_set_boolean (&s_pValue, !bToggleValue);
			cairo_dock_dbus_set_property (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "Shuffle", &s_pValue);
			g_value_unset (&s_pValue);
		break;
		
		case PLAYER_REPEAT :
			bToggleValue = cd_mpris2_is_loop ();
			cd_debug ("SetLoop <- %d\n", !bToggleValue);
			g_value_init (&s_pValue, G_TYPE_STRING);
			g_value_set_static_string (&s_pValue, bToggleValue ? "None" : "Playlist");
			cairo_dock_dbus_set_property (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "LoopStatus", &s_pValue);
			g_value_unset (&s_pValue);
		break;
		
		case PLAYER_ENQUEUE :
		{
			cd_debug ("enqueue %s\n", song);
			GError *erreur = NULL;
			DBusGProxy *proxy = cairo_dock_create_new_session_proxy ("org.mpris.MediaPlayer2",
				"/org/mpris/MediaPlayer2",
				"org.mpris.MediaPlayer2.TrackList");
			dbus_g_proxy_call (proxy, "AddTrack", &erreur,
				G_TYPE_INVALID,
				G_TYPE_STRING, song,  // Uri
				DBUS_TYPE_G_OBJECT_PATH, "",  // AfterTrack
				G_TYPE_BOOLEAN, TRUE,  // SetAsCurrent
				G_TYPE_INVALID);
			g_object_unref (proxy);
			
			if (erreur != NULL)  // the TrackList interface may not exist.
			{
				g_error_free (erreur);
				erreur = NULL;
				dbus_g_proxy_call_no_reply (proxy, "OpenUri",
					G_TYPE_INVALID,
					G_TYPE_STRING, song,
					G_TYPE_INVALID);
			}
		}
		break;
		
		case PLAYER_VOLUME :
		{
			double fVolume = cd_mpris2_get_volume ();  // [0, 1]
			if (song && strcmp (song, "up") == 0)
				fVolume += .05;
			else
				fVolume -= .05;
			cd_debug ("volume <- %f\n", fVolume);
			g_value_init (&s_pValue, G_TYPE_DOUBLE);
			g_value_set_double (&s_pValue, fVolume);
			cairo_dock_dbus_set_property (myData.dbus_proxy_player, "org.mpris.MediaPlayer2.Player", "Volume", &s_pValue);
			g_value_unset (&s_pValue);
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

static void _cd_cclosure_marshal_VOID__STRING_HASH_STRV (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	typedef void (*GMarshalFunc_VOID__STRING_HASH_STRV) (
		gpointer     data1,
		gchar      *arg_1,
		GHashTable *arg_2,
		gchar     **arg_3,
		gpointer     data2);
	register GMarshalFunc_VOID__STRING_HASH_STRV callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 4);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_HASH_STRV) (marshal_data ? marshal_data : cc->callback);
	
	g_return_if_fail (callback != NULL);
	g_return_if_fail (G_VALUE_HOLDS_STRING (param_values + 1));
	g_return_if_fail (G_VALUE_HOLDS_BOXED (param_values + 2));
	g_return_if_fail (G_VALUE_HOLDS (param_values + 3, G_TYPE_STRV));
	
	callback (data1,
		(char*) g_value_get_string (param_values + 1),
		(GHashTable*) g_value_get_boxed (param_values + 2),
		(char**) g_value_get_boxed (param_values + 3),
		data2);
}

static void cd_mpris2_start (void)
{
	// register to the signals
	cd_debug ("%s ()", __func__);
	dbus_g_object_register_marshaller (_cd_cclosure_marshal_VOID__STRING_HASH_STRV,
		G_TYPE_NONE,
		G_TYPE_STRING,
		dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
		G_TYPE_STRV,
		G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "PropertiesChanged",
		G_TYPE_STRING,  // interface
		dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),  // dict of (property, new value)
		G_TYPE_STRV,  // array of properties unvalidated
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "PropertiesChanged",
		G_CALLBACK(on_properties_changed), NULL, NULL);

	// get the current state.
	myData.iTrackListLength = 0;
	myData.iTrackListIndex = 0;
	cd_mpris2_getPlaying_async ();  // will get song infos after playing status.
}


void cd_musicplayer_register_mpris2_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Mpris2";
	pHandler->get_data = cd_mpris2_get_data;
	pHandler->stop = cd_mpris2_stop;
	pHandler->start = cd_mpris2_start;
	pHandler->control = cd_mpris2_control;
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_GOOD;
	
	pHandler->cMprisService = NULL;  // service is left NULL until an actual MPRIS2 player is present.
	pHandler->path = "/org/mpris/MediaPlayer2";
	pHandler->interface = DBUS_INTERFACE_PROPERTIES;  // "org.mpris.MediaPlayer2.Player"
	pHandler->path2 = "/org/mpris/MediaPlayer2";
	pHandler->interface2 = "org.mpris.MediaPlayer2.Player";
	
	pHandler->appclass = NULL;  // will be filled later.
	pHandler->launch = NULL;  // will be filled later.
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE | PLAYER_VOLUME;
	cd_musicplayer_register_my_handler (pHandler);
}

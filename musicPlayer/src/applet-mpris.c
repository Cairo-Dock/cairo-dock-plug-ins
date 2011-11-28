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
#include "applet-mpris.h"

#define MP_DBUS_TYPE_PLAYER_STATUS_MPRIS (dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID))
#define MP_DBUS_TYPE_TRACKLIST_DATA G_TYPE_INT

static DBusGProxyCall *s_pGetSongInfosCall = NULL;
static DBusGProxyCall *s_pGetStatusCall = NULL;
static DBusGProxyCall *s_pGetCurrentTrackCall = NULL;

static void cd_mpris_getSongInfos_async (void);

/*
<node name="/Player">
  <interface name="org.freedesktop.MediaPlayer">

    <method name="Pause">
    </method>

    <method name="Stop">
    </method>

    <method name="Play">
    </method>

    <method name="Prev">
    </method>

    <method name="Next">
    </method>

    <method name="Repeat">
        <arg type="b" direction="in"/>
    </method>

    <method name="GetStatus">
        <arg type="(iiii)" direction="out"/>  // 0 = Playing, 1 = Paused, 2 = Stopped / 0 = Playing linearly , 1 = Playing randomly / 0 = Go to the next element once the current has finished playing , 1 = Repeat the current / 0 = Stop playing once the last element has been played, 1 = Never give up playing

        <annotation name="com.trolltech.QtDBus.QtTypeName.Out0" value="DBusStatus"/>
    </method>

    <method name="VolumeSet">
        <arg type="i" direction="in"/>
    </method>

    <method name="VolumeGet">
        <arg type="i" direction="out"/>
    </method>

    <method name="PositionSet">
        <arg type="i" direction="in"/>
    </method>

    <method name="PositionGet">
        <arg type="i" direction="out"/>
    </method>

    <method name="GetMetadata">
        <arg type="a{sv}" direction="out"/>
        <annotation name="com.trolltech.QtDBus.QtTypeName.Out0" value="QVariantMap"/>
    </method>

    <method name="GetCaps">
        <arg type="i" direction="out" />
    </method>

    <signal name="TrackChange">
        <arg type="a{sv}"/>
        <annotation name="com.trolltech.QtDBus.QtTypeName.In0" value="QVariantMap"/>
    </signal>

    <signal name="StatusChange">
        <arg type="(iiii)"/>
        <annotation name="com.trolltech.QtDBus.QtTypeName.In0" value="DBusStatus"/>
    </signal>

    <signal name="CapsChange">
        <arg type="i" />
    </signal>

    <!-- NB: Amarok extensions to the mpris spec -->
    <method name="VolumeUp">
        <arg type="i" drection="in"/>
    </method>

    <method name="VolumeDown">
        <arg type="i" drection="in"/>
    </method>

    <method name="Mute">
    </method>

  </interface>
</node>

<node name="/TrackList">
  <interface name="org.freedesktop.MediaPlayer">

    <method name="GetMetadata">
        <arg type="i" direction="in" />
        <arg type="a{sv}" direction="out" />
        <annotation name="com.trolltech.QtDBus.QtTypeName.Out0" value="QVariantMap"/>
    </method>

    <method name="GetCurrentTrack">  // position of current URI in the TrackList. The position of the first URI in the TrackList is 0.
        <arg type="i" direction="out" />
    </method>

    <method name="GetLength">  // Number of elements in the TrackList.
        <arg type="i" direction="out" />
    </method>

    <method name="AddTrack">
        <arg type="s" direction="in" />
        <arg type="b" direction="in" />  // TRUE if the item should be played immediately, FALSE otherwise.
        <arg type="i" direction="out" />
    </method>

    <method name="DelTrack">
        <arg type="i" />
    </method>

    <method name="SetLoop">
        <arg type="b" />
    </method>

    <method name="SetRandom">
        <arg type="b" />
    </method>

    <signal name="TrackListChange">
        <arg type="i" />
    </signal>

  </interface>
</node>
*/


/////////////////////////////////
// Les Fonctions propres a MP. //
/////////////////////////////////
static inline void _set_playing_status_mpris (int iStatus)
{
	switch (iStatus)
	{
		case 0:
			myData.iPlayingStatus = PLAYER_PLAYING;
		break;
		case 1:
			myData.iPlayingStatus = PLAYER_PAUSED;
		break;
		case 2:
		default:
			myData.iPlayingStatus = PLAYER_STOPPED;
		break;
	}
}

static inline int _extract_status_mpris (GValueArray *status, int iStatusIndex)
{
	int iStatus;
	GValue *value = g_value_array_get_nth (status, iStatusIndex);
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		iStatus = g_value_get_int (value);
	else
		iStatus = -1;
	return iStatus;
}

static int _mpris_get_status (int iStatusIndex)
{
	GError *erreur = NULL;
	GValueArray *status;
	dbus_g_proxy_call (myData.dbus_proxy_player, "GetStatus", &erreur,  // Audacious's GetStatus() does not comply with MPRIS spec, it returns a single Int32
		G_TYPE_INVALID,
		MP_DBUS_TYPE_PLAYER_STATUS_MPRIS, &status,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return -1;
	}
	else
	{
		int iStatus = _extract_status_mpris (status, iStatusIndex);
		g_value_array_free (status);
		return iStatus;
	}
}

static void _on_got_playing_status (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	s_pGetStatusCall = NULL;
	
	int iStatus = -1;
	GValueArray *status = NULL;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		MP_DBUS_TYPE_PLAYER_STATUS_MPRIS, &status,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't get MPRIS status (%s)\n", erreur->message);
		g_error_free (erreur);
	}
	else if (status != NULL)
	{
		iStatus = _extract_status_mpris (status, 0);
		g_value_array_free (status);
	}
	
	_set_playing_status_mpris (iStatus);
	
	cd_mpris_getSongInfos_async ();
	
	CD_APPLET_LEAVE ();
}
void cd_mpris_getPlaying_async (void)
{
	if (s_pGetStatusCall != NULL)
		return;
	s_pGetStatusCall = dbus_g_proxy_begin_call (myData.dbus_proxy_player,
		"GetStatus",
		(DBusGProxyCallNotify)_on_got_playing_status,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

int cd_mpris_get_volume (void)
{
	GError *erreur = NULL;
	int iVolume;
	dbus_g_proxy_call (myData.dbus_proxy_player, "VolumeGet", &erreur,
		G_TYPE_INVALID,
		G_TYPE_INT, &iVolume,
		G_TYPE_INVALID);
	return iVolume;
}

void cd_mpris_set_volume (int iVolume)
{
	dbus_g_proxy_call_no_reply (myData.dbus_proxy_player, "VolumeSet",
		G_TYPE_INT, iVolume,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}

/* Teste si MP joue de la musique ou non
 */
void cd_mpris_getPlaying (void)  // sync version, used by Audacious.
{
	cd_debug ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (0);
	_set_playing_status_mpris (iStatus);
}

/* Teste si MP joue en boucle ou non
 */
static gboolean cd_mpris_is_loop (void)
{
	cd_debug ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (3);  // Fourth integer: 0 = Stop playing once the last element has been played, 1 = Never give up playing.
	g_return_val_if_fail (iStatus != -1, FALSE);
	return iStatus;
}

/* Teste si MP joue aleatoirement ou non
 */
static gboolean cd_mpris_is_shuffle (void)
{
	cd_debug ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (1);  // Second interger: 0 = Playing linearly , 1 = Playing randomly.
	g_return_val_if_fail (iStatus != -1, FALSE);
	return iStatus;
}

/* Renvoie le temps ecoule en secondes..
 */
void cd_mpris_get_time_elapsed (void)  // used by Audacious too.
{
	myData.iCurrentTime = cairo_dock_dbus_get_integer (myData.dbus_proxy_player, "PositionGet");
	if (myData.iCurrentTime > 0)  // -1 signifie que la valeur n'a pas pu etre retrouvee (lecteur ferme).
		myData.iCurrentTime /= 1000;
}

/* Renvoie le numero de la chanson courante dans la playlist.
 */
static void _on_get_current_track (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	s_pGetCurrentTrackCall = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		NULL,
		G_TYPE_INT,
		&myData.iTrackListIndex,
		G_TYPE_INVALID);
	//g_print ("myData.iTrackListIndex <- %d\n", myData.iTrackListIndex);
	if (myConfig.iQuickInfoType == MY_APPLET_TRACK && myData.iTrackListIndex > 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%d", (myDesklet && myDesklet->container.iWidth >= 64 ? D_("Track") : ""), myData.iTrackListIndex);
		CD_APPLET_REDRAW_MY_ICON;
	}
}
static void cd_mpris_get_track_index_async (void)
{
	if (s_pGetCurrentTrackCall != NULL)
		return;
	s_pGetCurrentTrackCall = dbus_g_proxy_begin_call (myData.dbus_proxy_shell, "GetCurrentTrack",
		(DBusGProxyCallNotify)_on_get_current_track,
		NULL,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
	/**myData.iTrackListIndex = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetCurrentTrack");
	//g_print ("myData.iTrackListIndex <- %d\n", myData.iTrackListIndex);*/
}

static inline int _get_integer_value (GValue *value)
{
	if (G_VALUE_HOLDS_INT(value))
	{
		return g_value_get_int(value);
	}
	else if (G_VALUE_HOLDS_UINT(value))
	{
		return g_value_get_uint(value);
	}
	else if (G_VALUE_HOLDS_INT64(value))
	{
		return g_value_get_int64(value);
	}
	else if (G_VALUE_HOLDS_UINT64(value))
	{
		return g_value_get_uint64(value);
	}
	else if (G_VALUE_HOLDS_LONG(value))
	{
		return g_value_get_long(value);
	}
	else if (G_VALUE_HOLDS_ULONG(value))
	{
		return g_value_get_ulong(value);
	}
	else
		return 0;
}
static inline void _extract_metadata (GHashTable *data_list)
{
	GValue *value;
	const gchar *str;
	g_free (myData.cArtist);
	myData.cArtist = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "artist");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
	{
		str = g_value_get_string(value);
		if (str && *str != '\0')
			myData.cArtist = g_strdup (str);
	}
	cd_message ("  cArtist <- %s", myData.cArtist);
	
	g_free (myData.cAlbum);
	myData.cAlbum = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "album");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
	{
		str = g_value_get_string(value);
		if (str && *str != '\0')
			myData.cAlbum = g_strdup (str);
	}
	cd_message ("  cAlbum <- %s", myData.cAlbum);
	
	g_free (myData.cTitle);
	myData.cTitle = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "title");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
	{
		str = g_value_get_string(value);
		if (str && *str != '\0')
			myData.cTitle = g_strdup (str);
	}
	cd_message ("  cTitle <- %s", myData.cTitle);
	
	value = (GValue *) g_hash_table_lookup(data_list, "tracknumber");
	if (value == NULL)
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");  // au cas ou
	if (value != NULL)
	{
		if (G_VALUE_HOLDS_INT (value))  // amarok
			myData.iTrackNumber = g_value_get_int (value);
		else if (G_VALUE_HOLDS_UINT (value))
			myData.iTrackNumber = g_value_get_uint (value);
		else if (G_VALUE_HOLDS_STRING (value))  // qmmp
		{
			const gchar *s = g_value_get_string (value);
			myData.iTrackNumber = (s ? atoi(s) : 0);
		}
	}
	else
		myData.iTrackNumber = 0;
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);
	
	myData.iSongLength = 0;
	value = (GValue *) g_hash_table_lookup(data_list, "mtime");  // duree en ms.
	if (value != NULL)
	{
		myData.iSongLength = _get_integer_value (value) / 1000;
	}
	else
	{
		value = (GValue *) g_hash_table_lookup(data_list, "length");
		if (value == NULL)
			value = (GValue *) g_hash_table_lookup(data_list, "time");
		if (value != NULL)
		{
			myData.iSongLength = _get_integer_value (value);
			if (myData.iSongLength > 7200)  // on est dans le cas ou "mtime" n'existe pas, donc on n'est pas totalement sur que le lecteur ne nous ai pas refile le temps en ms. On considere qu'une chanson de plus de 2h ca n'existe pas.
				myData.iSongLength /= 1000;
		}
	}
	cd_message ("  iSongLength <- %ds", myData.iSongLength);
	
	g_free (myData.cPlayingUri);
	value = (GValue *) g_hash_table_lookup(data_list, "location");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cPlayingUri = g_strdup (g_value_get_string(value));
	else myData.cPlayingUri = NULL;
	cd_message ("  cUri <- %s", myData.cPlayingUri);
	
	const gchar *cCoverPath = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "arturl");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
	{
		cCoverPath = g_value_get_string(value);
	}
	cd_musicplayer_get_cover_path (cCoverPath, TRUE);
}

static void _on_got_song_infos (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	s_pGetSongInfosCall = NULL;
	
	GHashTable *data_list = NULL;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		MP_DBUS_TYPE_SONG_METADATA, &data_list,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't get MPRIS song infos (%s)\n", erreur->message);
		g_error_free (erreur);
		data_list = NULL;
	}
	
	if (data_list != NULL)
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
static void cd_mpris_getSongInfos_async (void)
{
	if (s_pGetSongInfosCall != NULL)
		return;
	s_pGetSongInfosCall = dbus_g_proxy_begin_call (myData.dbus_proxy_player,
		"GetMetadata",
		(DBusGProxyCallNotify)_on_got_song_infos,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executee a chaque changement de musique.
 */
static void onChangeSong_mpris(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s ()\n", __func__);
	
	if (metadata != NULL)
	{
		_extract_metadata (metadata);
		myData.iPlayingStatus = PLAYER_PLAYING;  // pour les lecteurs bugues comem Exaile qui envoit un statut "stop" au changement de musique sans envoyer de status "play" par la suite. On considere donc que si le lecteur joue une nouvelle musique, c'est qu'il est en "play".
	}
	else
	{
		cd_warning ("  no song properties");
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

/* Fonction executee a chaque changement play/pause
 */
void onChangePlaying_mpris (DBusGProxy *player_proxy, GValueArray *status, gpointer data)  // used by Audacious too.
{
	CD_APPLET_ENTER;
	//cd_debug ("MP : %s (%x)\n", __func__, status);
	myData.iGetTimeFailed = 0;
	int iStatus = _extract_status_mpris (status, 0);
	_set_playing_status_mpris (iStatus);
	cd_debug ("myData.iPlayingStatus <- %d", myData.iPlayingStatus);
	
	if (myData.iPlayingStatus == PLAYER_PLAYING)  // le handler est stoppe lorsque le lecteur ne joue rien.
		cd_musicplayer_relaunch_handler ();
	
	if (myData.iPlayingStatus == PLAYER_STOPPED)
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	
	if(! myData.cover_exist/** && (myData.cPlayingUri != NULL || myData.cTitle != NULL)*/)
	{
		cd_musicplayer_set_surface (myData.iPlayingStatus);
	}
	else
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
	CD_APPLET_LEAVE ();
}

/* Fonction execute a chaque changement dans la TrackList.
 */
static void onChangeTrackList_mpris (DBusGProxy *player_proxy, gint iNewTrackListLength, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s (%d)", __func__, iNewTrackListLength);
	myData.iTrackListLength = iNewTrackListLength;
	cd_mpris_get_track_index_async ();
	CD_APPLET_LEAVE ();
}

////////////////////////////
// Definition du backend. //
////////////////////////////

/* Permet de liberer la memoire prise par le backend.
 */
static void cd_mpris_stop (void)
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
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangePlaying_mpris), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeSong_mpris), NULL);
	}
	if (myData.dbus_proxy_shell!= NULL)
	{
		if (s_pGetCurrentTrackCall != NULL)
		{
			dbus_g_proxy_cancel_call (myData.dbus_proxy_player, s_pGetCurrentTrackCall);
			s_pGetCurrentTrackCall = NULL;
		}
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_shell, "TrackListChange",
			G_CALLBACK(onChangeTrackList_mpris), NULL);
	}
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur).
 */
static void cd_mpris_control (MyPlayerControl pControl, const char* song)
{
	gboolean bToggleValue;
	switch (pControl)
	{
		case PLAYER_PREVIOUS :
			cairo_dock_dbus_call (myData.dbus_proxy_player, "Prev");
		break;
		
		case PLAYER_STOP :
			cairo_dock_dbus_call (myData.dbus_proxy_player, "Stop");
		break;
		
		case PLAYER_PLAY_PAUSE :
			if (myData.iPlayingStatus != PLAYER_PLAYING)
				cairo_dock_dbus_call (myData.dbus_proxy_player, "Play");
			else
				cairo_dock_dbus_call (myData.dbus_proxy_player, "Pause");
		break;
		
		case PLAYER_NEXT :
			cairo_dock_dbus_call (myData.dbus_proxy_player, "Next");
		break;
		
		case PLAYER_JUMPBOX :
			
		break;
		
		case PLAYER_SHUFFLE :
			bToggleValue = cd_mpris_is_shuffle ();
			cd_debug ("SetRandom <- %d\n", !bToggleValue);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetRandom",
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, !bToggleValue,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_REPEAT :
			bToggleValue = cd_mpris_is_loop ();
			cd_debug ("SetLoop <- %d\n", !bToggleValue);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetLoop",
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, !bToggleValue,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_ENQUEUE :
			cd_debug ("enqueue %s\n", song);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "AddTrack",
				G_TYPE_INVALID,
				G_TYPE_STRING, song,
				G_TYPE_BOOLEAN, FALSE,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_VOLUME :
		{
			int iVolume = cd_mpris_get_volume ();  // [0, 100]
			if (song && strcmp (song, "up") == 0)
				iVolume += 5;
			else
				iVolume -= 5;
			
			if (iVolume > 100)
				iVolume = 100;
			else if (iVolume < 0)
			iVolume = 0;
			
			cd_mpris_set_volume (iVolume);
		}
		
		default :
		break;
	}
}


/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_mpris_get_data (void)
{
	if (myData.iPlayingStatus == PLAYER_PLAYING)
	{
		cd_mpris_get_time_elapsed ();
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

static void cd_mpris_start (void)
{
	// register to the signals
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "StatusChange",
		MP_DBUS_TYPE_PLAYER_STATUS_MPRIS,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "StatusChange",
		G_CALLBACK(onChangePlaying_mpris), NULL, NULL);

	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "TrackChange",
		MP_DBUS_TYPE_SONG_METADATA,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "TrackChange",
		G_CALLBACK(onChangeSong_mpris), NULL, NULL);

	dbus_g_proxy_add_signal(myData.dbus_proxy_shell, "TrackListChange",
		MP_DBUS_TYPE_TRACKLIST_DATA,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_shell, "TrackListChange",
		G_CALLBACK(onChangeTrackList_mpris), NULL, NULL);
	
	// get the current state.
	cd_mpris_getPlaying_async ();  // will get song infos after playing status.
	/**cd_mpris_getPlaying ();
	cd_mpris_getSongInfos ();
	cd_musicplayer_update_icon (TRUE);
	cd_musicplayer_relaunch_handler ();*/
}

MusicPlayerHandler *cd_mpris_new_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->get_data = cd_mpris_get_data;
	pHandler->stop = cd_mpris_stop;
	pHandler->start = cd_mpris_start;
	pHandler->control = cd_mpris_control;
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE | PLAYER_VOLUME;
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_GOOD;
	
	pHandler->path = "/Player";
	pHandler->interface = "org.freedesktop.MediaPlayer";
	pHandler->path2 = "/TrackList";
	pHandler->interface2 = "org.freedesktop.MediaPlayer";
	
	return pHandler;
}

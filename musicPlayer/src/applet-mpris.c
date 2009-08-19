
/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

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
#include "applet-cover.h"
#include "applet-mpris.h"

#define MP_DBUS_TYPE_PLAYER_STATUS_MPRIS (dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID))
#define MP_DBUS_TYPE_TRACKLIST_DATA G_TYPE_INT

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
static inline void _extract_playing_status_mpris (int iStatus)
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

static inline int _extract_status_mpris (GValueArray *status, int i)
{
	int iStatus;
	GValue *value = g_value_array_get_nth (status, i);
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		iStatus = g_value_get_int (value);
	else
		iStatus = -1;
	return iStatus;
}

static int _mpris_get_status (int i)
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
		int iStatus = _extract_status_mpris (status, i);
		g_value_array_free (status);
		return iStatus;
	}
}

/* Teste si MP joue de la musique ou non
 */
void cd_mpris_getPlaying (void)
{
	g_print ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (0);
	_extract_playing_status_mpris (iStatus);
}

/* Teste si MP joue en boucle ou non
 */
gboolean cd_mpris_is_loop (void)
{
	g_print ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (3);  // Fourth integer: 0 = Stop playing once the last element has been played, 1 = Never give up playing.
	g_return_val_if_fail (iStatus != -1, FALSE);
	return iStatus;
}

/* Teste si MP joue aleatoirement ou non
 */
gboolean cd_mpris_is_shuffle (void)
{
	g_print ("%s ()\n", __func__);
	int iStatus = _mpris_get_status (1);  // Second interger: 0 = Playing linearly , 1 = Playing randomly.
	g_return_val_if_fail (iStatus != -1, FALSE);
	return iStatus;
}

/* Renvoie le temps ecoule en secondes..
 */
void cd_mpris_get_time_elapsed (void)
{
	myData.iCurrentTime = cairo_dock_dbus_get_integer (myData.dbus_proxy_player, "PositionGet") / 1000;
	//g_print ("myData.iCurrentTime <- %d\n", myData.iCurrentTime);
}

/* Renvoie le temps ecoule en secondes..
 */
void cd_mpris_get_track_index (void)
{
	myData.iTrackListIndex = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetCurrentTrack");
	//g_print ("myData.iTrackListIndex <- %d\n", myData.iTrackListIndex);
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
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		myData.iTrackNumber = g_value_get_int(value);
	else
		myData.iTrackNumber = 0;
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);
	
	value = (GValue *) g_hash_table_lookup(data_list, "length");
	if (value == NULL)
		value = (GValue *) g_hash_table_lookup(data_list, "time");
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		myData.iSongLength = g_value_get_int(value) / 1000;
	else
		myData.iSongLength = 0;
	cd_message ("  iSongLength <- %ds", myData.iSongLength);
	
	g_free (myData.cPlayingUri);
	value = (GValue *) g_hash_table_lookup(data_list, "location");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cPlayingUri = g_strdup (g_value_get_string(value));
	else myData.cPlayingUri = NULL;
	cd_message ("  cUri <- %s", myData.cPlayingUri);
	
	gchar *cCoverPath = NULL;
	value = (GValue *) g_hash_table_lookup(data_list, "arturl");
	if (value != NULL && G_VALUE_HOLDS_STRING(value))
	{
		cCoverPath = g_value_get_string(value);
	}
	cd_musicplayer_get_cover_path (cCoverPath, TRUE);
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
void cd_mpris_getSongInfos ()
{
	GHashTable *data_list = NULL;
	const gchar *data;
		
	if(dbus_g_proxy_call (myData.dbus_proxy_player, "GetMetadata", NULL,
		G_TYPE_INVALID,
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
void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	g_print ("MP : %s ()\n", __func__);
	
	if (metadata != NULL)
	{
		_extract_metadata (metadata);
		myData.bIsRunning = TRUE;
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
		
		cd_musicplayer_dbus_detect_player ();
	}
	cd_musicplayer_update_icon (TRUE);
}

/* Fonction executée à chaque changement play/pause
 */
void onChangePlaying_mpris (DBusGProxy *player_proxy, GValueArray *status, gpointer data)
{
	g_print ("MP : %s (%x)\n", __func__, status);
	myData.bIsRunning = TRUE;
	int iStatus = _extract_status_mpris (status, 0);
	_extract_playing_status_mpris (iStatus);
	g_print ("-> myData.iPlayingStatus : %d\n", myData.iPlayingStatus);
	
	if (myData.iPlayingStatus == PLAYER_PLAYING)  // le handler est stoppe lorsque le lecteur ne joue rien.
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

/* Fonction executée à chaque changement dans la TrackList.
 */
void onChangeTrackList (DBusGProxy *player_proxy, gint iNewTrackListLength, gpointer data)
{
	g_print ("MP : %s (%d)\n", __func__, iNewTrackListLength);
	myData.iTrackListLength = iNewTrackListLength;
	cd_mpris_get_track_index ();
}

////////////////////////////
// Definition du backend. //
////////////////////////////

/* Fonction de connexion au bus de MP.
 */
gboolean cd_mpris_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		// cree les proxys.
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus ();
		
		myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell ();
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "StatusChange",
			MP_DBUS_TYPE_PLAYER_STATUS_MPRIS,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangePlaying_mpris), NULL, NULL);
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "TrackChange",
			MP_DBUS_TYPE_SONG_METADATA,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_shell, "TrackListChange",
			MP_DBUS_TYPE_TRACKLIST_DATA,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.dbus_proxy_shell, "TrackListChange",
			G_CALLBACK(onChangeTrackList), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par le backend.
 */
void cd_mpris_free_data (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangePlaying_mpris), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeSong), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_shell, "TrackListChange",
			G_CALLBACK(onChangeTrackList), NULL);
	}
	
	musicplayer_dbus_disconnect_from_bus();
	musicplayer_dbus_disconnect_from_bus_Shell();
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur).
 */
void cd_mpris_control (MyPlayerControl pControl, const char* song)
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
			g_print ("SetRandom <- %d\n", !bToggleValue);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetRandom",
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, !bToggleValue,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_REPEAT :
			bToggleValue = cd_mpris_is_loop ();
			g_print ("SetLoop <- %d\n", !bToggleValue);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "SetLoop",
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, !bToggleValue,
				G_TYPE_INVALID);
		break;
		
		case PLAYER_ENQUEUE :
			g_print ("enqueue %s\n", song);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "AddTrack",
				G_TYPE_INVALID,
				G_TYPE_STRING, song,
				G_TYPE_BOOLEAN, FALSE,
				G_TYPE_INVALID);
		break;
		
		default :
		break;
	}
}


/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
void cd_mpris_read_data (void)
{
	if (myData.dbus_enable)
	{
		if (myData.bIsRunning)
		{
			if (myData.iPlayingStatus == PLAYER_PLAYING)
				cd_mpris_get_time_elapsed ();
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

/* Initialise le backend de MP.
 */
void cd_mpris_configure (void)
{
	myData.DBus_commands.path = "/Player";
	myData.DBus_commands.path2 = "/TrackList";
	myData.DBus_commands.interface = "org.freedesktop.MediaPlayer";
	myData.DBus_commands.interface2 = "org.freedesktop.MediaPlayer";
	
	myData.dbus_enable = cd_mpris_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de MP.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de MP sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			g_print ("MP : MP is running\n");
			cd_mpris_getPlaying ();
			cd_mpris_getSongInfos ();
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

MusicPlayerHandeler *cd_mpris_new_handler (void)
{
	MusicPlayerHandeler *pHandler = g_new0 (MusicPlayerHandeler, 1);
	pHandler->read_data = cd_mpris_read_data;
	pHandler->free_data = cd_mpris_free_data;
	pHandler->configure = cd_mpris_configure;
	pHandler->control = cd_mpris_control;
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE;
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_GOOD;
	return pHandler;
}

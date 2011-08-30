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
#include "applet-audacious.h"

#define MP_DBUS_TYPE_PLAYER_STATUS_MPRIS (dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID))
#define MP_DBUS_TYPE_PLAYER_STATUS G_TYPE_INT

/*
<node name="/Player">
22 <interface name="org.freedesktop.MediaPlayer">
23 <method name="Next">
24 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
25 </method>
26 <method name="Prev">
27 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
28 </method>
29 <method name="Pause">
30 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
31 </method>
32 <method name="Stop">
33 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
34 </method>
35 <method name="Play">
36 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
37 </method>
38 <method name="Repeat">
39 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
40 <arg type="b" direction="in" />
41 </method>
42 <method name="Quit">
43 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
44 </method>
45 <method name="GetStatus">
46 <arg type="(iiii)" direction="out" />
47 </method>
48 <method name="GetMetadata">
49 <arg type="a{sv}" direction="out" />
50 </method>
51 <method name="GetCaps">
52 <arg type="i" direction="out" />
53 </method>
54 <method name="VolumeSet">
55 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
56 <arg type="i" direction="in" />
57 </method>
58 <method name="VolumeGet">
59 <arg type="i" direction="out" />
60 </method>
61 <method name="PositionSet">
62 <annotation name="org.freedesktop.DBus.GLib.NoReply" value=""/>
63 <arg type="i" direction="in" />
64 </method>
65 <method name="PositionGet">
66 <arg type="i" direction="out" />
67 </method>
68
69 <signal name="TrackChange">
70 <arg type="a{sv}" />
71 </signal>
72 <signal name="StatusChange">
73 <arg type="(iiii)" />
74 </signal>
75 <signal name="CapsChange">
76 <arg type="i" />
77 </signal>
78 </interface>
79 </node> 

service : org.mpris.audacious
interface : org.freedesktop.MediaPlayer

path : /Player
	Next
	Prev
	Pause
	Stop
	Play
	Repeat(bool)
	GetStatus(int[4]: status:play=0,pause,stop shuffle=random no_playlist_advance repeat=loop)
	GetMetadata(hash: length title artist album genre codec quality track-number location)
	PositionGet(int=time_in_second)
	
	TrackChange(hash: idem)
	StatusChange(int[4])

path : /TrackList
	AddTrack(string,bool=play_it_now)
	Random(bool) not yet implemented
	Loop(bool) not yet implemented

service : org.atheme.audacious
interface : org.atheme.audacious

path : /org/atheme/audacious
	PlaylistVisible(bool=is_pl_win)
	ShowPlaylist(bool=show)
	ToggleShuffle
	ToggleRepeat
*/


/////////////////////////////////
// Les Fonctions propres a AU. //
/////////////////////////////////

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
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");  // old parameter.
	if (value != NULL && G_VALUE_HOLDS_INT(value))
		myData.iTrackNumber = g_value_get_int(value);
	else
		myData.iTrackNumber = 0.;
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);
	
	value = (GValue *) g_hash_table_lookup(data_list, "mtime");
	if (value == NULL)
		value = (GValue *) g_hash_table_lookup(data_list, "length");  // old parameter.
	if (value != NULL && G_VALUE_HOLDS_INT(value)) myData.iSongLength = g_value_get_int(value) / 1000;
	else myData.iSongLength = 0;
	cd_message ("  iSongLength <- %ds", myData.iSongLength);
	
	g_free (myData.cPlayingUri);
	value = (GValue *) g_hash_table_lookup(data_list, "location");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cPlayingUri = g_strdup (g_value_get_string(value));
	else myData.cPlayingUri = NULL;
	cd_message ("  cUri <- %s", myData.cPlayingUri);
	
	cd_musicplayer_get_cover_path (NULL, TRUE);
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_audacious_getSongInfos (void)
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
		myData.iTrackListIndex = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetCurrentTrack");
		myData.iTrackListLength = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetLength");
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
	}
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("MP : %s ()\n", __func__);
	
	if (metadata != NULL)
	{
		_extract_metadata (metadata);
		myData.iTrackListIndex = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetCurrentTrack");
		myData.iTrackListLength = cairo_dock_dbus_get_integer (myData.dbus_proxy_shell, "GetLength");
		myData.iPlayingStatus = PLAYER_PLAYING;  // the current song has changed, means we are playing, so let's update the status now, for the coming update_icon().
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


////////////////////////////
// Definition du backend. //
////////////////////////////

/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur).
 */
static void cd_audacious_control (MyPlayerControl pControl, const char* song)
{
	const gchar *cCommand = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "Prev";
		break;
		
		case PLAYER_STOP :
			cCommand = "Stop";
		break;
		
		case PLAYER_PLAY_PAUSE :
			if (myData.iPlayingStatus != PLAYER_PLAYING)
				cCommand = "Play";
			else
				cCommand = "Pause";
		break;

		case PLAYER_NEXT :
			cCommand = "Next";
		break;
		
		case PLAYER_JUMPBOX :
		case PLAYER_SHUFFLE :
		case PLAYER_REPEAT :
		{
			
			DBusGProxy *dbus_proxy_atheme = cairo_dock_create_new_session_proxy (
				"org.atheme.audacious",
				"/org/atheme/audacious",
				"org.atheme.audacious");  // currently buggy ...
			if (dbus_proxy_atheme != NULL)
			{
				if (pControl == PLAYER_JUMPBOX)
				{
					cd_debug ("ShowPlaylist\n");
					cairo_dock_launch_command ("audacious2 --show-jump-box");
					/*dbus_g_proxy_call_no_reply (dbus_proxy_atheme, "ShowPlaylist",
						G_TYPE_INVALID,
						G_TYPE_BOOLEAN, TRUE,
						G_TYPE_INVALID);*/
				}
				else if (pControl == PLAYER_SHUFFLE)  // a terme, utiliser la methode "Random" du proxy_shell
				{
					cd_debug ("ToggleShuffle\n");
					cairo_dock_dbus_call (dbus_proxy_atheme, "ToggleShuffle");
				}
				else  // a terme, utiliser la methode "Loop" du proxy_shell
				{
					cd_debug ("ToggleRepeat\n");
					cairo_dock_dbus_call (dbus_proxy_atheme, "ToggleRepeat");
				}
				g_object_unref (dbus_proxy_atheme);
			}
			else
				cd_warning ("org.atheme.audacious not valid !");
		}
		break;
		
		case PLAYER_ENQUEUE :
			cd_debug ("enqueue %s\n", song);
			dbus_g_proxy_call_no_reply (myData.dbus_proxy_shell, "AddTrack",
				G_TYPE_INVALID,
				G_TYPE_STRING, song,
				G_TYPE_BOOLEAN, FALSE,
				G_TYPE_INVALID);
		break;
		
		default :
			return;
		break;
	}
	
	if (cCommand != NULL) 
	{
		cd_debug ("MP : Handler audacious : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}


/* Recupere le temps ecoule chaque seconde (pas de signal pour ca).
 */
static void cd_audacious_get_data (void)
{
	if (myData.iPlayingStatus == PLAYER_PLAYING)
	{
		cd_mpris_get_time_elapsed ();
		if (myData.iCurrentTime < 0)
			myData.iPlayingStatus = PLAYER_STOPPED;
	}
	else if (myData.iPlayingStatus != PLAYER_PAUSED)  // en pause le temps reste constant.
		myData.iCurrentTime = 0;
}

/* Initialise le backend de AU.
 */
static void cd_audacious_start (void)
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
		G_CALLBACK(onChangeSong), NULL, NULL);  // pas de TrackListChange, donc on fera tout dedans.
	
	// get the current state.
	cd_mpris_getPlaying ();  ///_audacious_getPlaying_mpris ();
	cd_audacious_getSongInfos ();
	cd_musicplayer_update_icon (TRUE);
}

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_audacious_handler (void)
{
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Audacious";
	pHandler->get_data = cd_audacious_get_data;
	pHandler->stop = NULL;
	pHandler->start = cd_audacious_start;
	pHandler->control = cd_audacious_control;
	pHandler->get_cover = NULL;
	pHandler->cCoverDir = NULL;

	// TODO: to check because I think that the class was audacious2 before and not audacious...
	/*gchar *cResult = cairo_dock_launch_command_sync ("which audacious2");
	if (cResult != NULL && *cResult == '/')
	{
		cd_debug ("MusicPlayer: Audacious2");
		pHandler->appclass = "audacious2";
		pHandler->launch = "audacious2";
	}
	else
	{
		cd_debug ("MusicPlayer: Audacious (without 2)");
		pHandler->appclass = "audacious"; // now it's without this '2'
		pHandler->launch = "audacious";
	}
	g_free (cResult);*/
	pHandler->appclass = "audacious";  // desktop file is audasious2.desktop, and command is audacious2, but audacious also exists, and the class is Audacious ... this is really not serious.
	pHandler->launch = "audacious";
	pHandler->cMprisService = "org.mpris.audacious";
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_JUMPBOX | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE;
	pHandler->iLevel = PLAYER_GOOD;
	
	pHandler->cMprisService = "org.mpris.audacious";
	pHandler->path = "/Player";
	pHandler->interface = "org.freedesktop.MediaPlayer";
	pHandler->path2 = "/TrackList";
	pHandler->interface2 = "org.freedesktop.MediaPlayer";
	
	cd_musicplayer_register_my_handler(pHandler);
}

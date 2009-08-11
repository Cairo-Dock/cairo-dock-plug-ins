
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
#include "applet-audacious.h"

//#define MP_DBUS_TYPE_PLAYER_STATUS (dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID))
#define MP_DBUS_TYPE_PLAYER_STATUS G_TYPE_UINT

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

static inline void _extract_playing_status (guint iStatus)
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

/* Teste si Audacious joue de la musique ou non
 */
static void _audacious_getPlaying (void)
{
	cd_message ("");
	GError *erreur = NULL;
	guint iStatus = 0;
	dbus_g_proxy_call (myData.dbus_proxy_player, "GetStatus", &erreur,  // Audacious's GetStatus() does not comply with MPRIS spec, it returns a single Int32
		G_TYPE_INVALID,
		MP_DBUS_TYPE_PLAYER_STATUS, &iStatus,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		myData.iPlayingStatus = PLAYER_STOPPED;
	}
	else
	{
		_extract_playing_status (iStatus);
	}
}


/* Renvoie le temps ecoule en secondes..
 */
static void _audacious_get_time_elapsed (void)
{
	cd_message ("");
	myData.iCurrentTime = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_player, "PositionGet");
	g_print ("myData.iCurrentTime <- %d\n", myData.iCurrentTime);
}


static inline void _extract_metadata (GHashTable *data_list)
{
	GValue *value;
	g_free (myData.cArtist);
	value = (GValue *) g_hash_table_lookup(data_list, "artist");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cArtist = g_strdup (g_value_get_string(value));
	else myData.cArtist = NULL;
	cd_message ("  cArtist <- %s", myData.cArtist);
	
	g_free (myData.cAlbum);
	value = (GValue *) g_hash_table_lookup(data_list, "album");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cAlbum = g_strdup (g_value_get_string(value));
	else myData.cAlbum = NULL;
	cd_message ("  cAlbum <- %s", myData.cAlbum);
	
	g_free (myData.cTitle);
	value = (GValue *) g_hash_table_lookup(data_list, "title");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cTitle = g_strdup (g_value_get_string(value));
	else myData.cTitle = NULL;
	cd_message ("  cTitle <- %s", myData.cTitle);
	
	value = (GValue *) g_hash_table_lookup(data_list, "track-number");
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iTrackNumber = g_value_get_uint(value);
	else myData.iTrackNumber = 0;
	cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);
	
	value = (GValue *) g_hash_table_lookup(data_list, "length");
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iSongLength = g_value_get_uint(value);
	else myData.iSongLength = 0;
	cd_message ("  iSongLength <- %ds", myData.iSongLength);
	
	
	g_free (myData.cPlayingUri);
	value = (GValue *) g_hash_table_lookup(data_list, "location");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cPlayingUri = g_strdup (g_value_get_string(value));
	else myData.cPlayingUri = NULL;
	cd_message ("  cUri <- %s", myData.cPlayingUri);
	
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // on memorise la precedente couverture.
	myData.cCoverPath = NULL;
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_audacious_getSongInfos ()
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
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy, GHashTable *metadata, gpointer data)
{
	g_print ("MP : %s ()", __func__);
	
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
static void onChangePlaying(DBusGProxy *player_proxy, /*int *iStatus, */ guint iStatus, gpointer data)  // int iStatus[4]
{
	g_print ("MP : %s (%d)\n", __func__, iStatus);
	myData.bIsRunning = TRUE;
	_extract_playing_status (iStatus);
	
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
gboolean cd_audacious_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // cree le proxy.
		
		myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell ();  // cree le proxy pour la 2eme interface car AU en a 2.
		
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "StatusChange",
			MP_DBUS_TYPE_PLAYER_STATUS,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(myData.dbus_proxy_player, "TrackChange",
			MP_DBUS_TYPE_SONG_METADATA,
			G_TYPE_INVALID);
		
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangePlaying), NULL, NULL);
			
		dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par notre controleur
 */
void cd_audacious_free_data (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangePlaying), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeSong), NULL);
	}
	
	musicplayer_dbus_disconnect_from_bus();
	musicplayer_dbus_disconnect_from_bus_Shell();
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur)
 */
void cd_audacious_control (MyPlayerControl pControl, const char* song)
{
	cd_debug ("");
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
				"org.atheme.audacious");
			if (dbus_proxy_atheme != NULL)
			{
				if (PLAYER_JUMPBOX)
				{
					dbus_g_proxy_call_no_reply (dbus_proxy_atheme, "ShowPlaylist",
						G_TYPE_INVALID,
						G_TYPE_BOOLEAN, TRUE,
						G_TYPE_INVALID);
				}
				else if (PLAYER_SHUFFLE)  // a terme, utiliser la methode "Random" du proxy_shell
				{
					cairo_dock_dbus_call (dbus_proxy_atheme, "ToggleShuffle");
				}
				else  // a terme, utiliser la methode "Loop" du proxy_shell
				{
					cairo_dock_dbus_call (dbus_proxy_atheme, "ToggleRepeat");
				}
				g_object_unref (dbus_proxy_atheme);
			}
		}
		break;
		
		case PLAYER_ENQUEUE :
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


void cd_audacious_read_data (void)
{
	if (myData.dbus_enable)
	{
		if (myData.bIsRunning && myData.iPlayingStatus == PLAYER_PLAYING)
		{
			_audacious_get_time_elapsed ();
		}
		else
		{
			myData.iCurrentTime = 0;
		}
		g_print ("%s () : %d\n", __func__, myData.iCurrentTime);
	}
}

/* Initialise le backend de AU.
 */
void cd_audacious_configure (void)
{
	cd_debug ("");
	myData.DBus_commands.service = "org.mpris.audacious";
	myData.DBus_commands.path = "/Player";
	myData.DBus_commands.path2 = "/TrackList";
	myData.DBus_commands.interface = "org.freedesktop.MediaPlayer";
	myData.DBus_commands.interface2 = "org.freedesktop.MediaPlayer";
	
	myData.dbus_enable = cd_audacious_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de AU.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de AU sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			g_print ("MP : AU is running");
			_audacious_getPlaying();
			cd_audacious_getSongInfos ();
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
void cd_musicplayer_register_audacious_handler (void)
{
	cd_debug ("");
	MusicPlayerHandeler *pAudacious = g_new0 (MusicPlayerHandeler, 1);
	pAudacious->read_data = cd_audacious_read_data;  // recupere le temps ecoule car on n'a pas de signal pour ca.
	pAudacious->free_data = cd_audacious_free_data;
	pAudacious->configure = cd_audacious_configure;  // renseigne les proprietes DBus et se connecte au bus.
	pAudacious->control = cd_audacious_control;
	pAudacious->get_cover = NULL;
	pAudacious->cCoverDir = NULL;  // audacious ne gere pas les pochettes.
	
	pAudacious->appclass = "audacious";
	pAudacious->name = "Audacious";
	pAudacious->launch = "audacious";
	pAudacious->iPlayer = MP_AUDACIOUS;
	pAudacious->bSeparateAcquisition = FALSE;  // inutile de threader.
	pAudacious->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_JUMPBOX | PLAYER_SHUFFLE | PLAYER_REPEAT | PLAYER_ENQUEUE;
	pAudacious->iLevel = PLAYER_GOOD;  // n'a besoin d'une boucle que pour afficher le temps ecoule.
	
	cd_musicplayer_register_my_handler(pAudacious, "audacious");
}

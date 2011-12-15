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
#include "applet-rhythmbox.h"

/////////////////////////////////
// Les Fonctions propres a RB. //
/////////////////////////////////

/* Teste si Rhythmbox joue de la musique ou non
 */
static void _rhythmbox_getPlaying (void)
{
	cd_message ("");
	if (cairo_dock_dbus_get_boolean (myData.dbus_proxy_player,"getPlaying"))
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
}

/* Retourne l'adresse de la musique jouée
 */
static void _rhythmbox_getPlayingUri(void)
{
	cd_message ("");
	g_free (myData.cPlayingUri);
	myData.cPlayingUri = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "getPlayingUri");
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_rhythmbox_getSongInfos (gboolean bGetAll)
{
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
		
	if(dbus_g_proxy_call (myData.dbus_proxy_shell, "getSongProperties", NULL,
		G_TYPE_STRING, myData.cPlayingUri,
		G_TYPE_INVALID,
		MP_DBUS_TYPE_SONG_METADATA,
		&data_list,
		G_TYPE_INVALID))
	{
		if (bGetAll)
		{
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
			
			value = (GValue *) g_hash_table_lookup(data_list, "duration");
			if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iSongLength = g_value_get_uint(value);
			else myData.iSongLength = 0;
			cd_message ("  iSongLength <- %ds", myData.iSongLength);
			myData.bCoverNeedsTest = FALSE;
		}
		
		const gchar *cString = NULL;
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		if (value != NULL && G_VALUE_HOLDS_STRING(value))  // RB nous donne une adresse, eventuellement distante.
			cString = g_value_get_string(value);
		cd_musicplayer_get_cover_path (cString, ! bGetAll);  // la 2eme fois on ne recupere que la couverture et donc on cherche aussi en cache.
		cd_debug ("MP :  cCoverPath <- %s\n", myData.cCoverPath);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_debug ("  can't get song properties");
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executee a chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	CD_APPLET_ENTER;
	cd_message ("MP : %s (%s)",__func__,uri);
	
	g_free (myData.cPlayingUri);
	if(uri != NULL && *uri != '\0')
	{
		myData.cPlayingUri = g_strdup (uri);
		cd_rhythmbox_getSongInfos (TRUE);  // TRUE <=> get all
	}
	else
	{
		myData.cPlayingUri = NULL;
		myData.cover_exist = FALSE;
		
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
	}
	cd_musicplayer_update_icon (TRUE);
	CD_APPLET_LEAVE ();
}

/* Fonction executee a chaque changement play/pause
 */
static void onChangePlaying(DBusGProxy *player_proxy, gboolean playing, gpointer data)
{
	CD_APPLET_ENTER;
	if (playing)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
	if(! myData.cover_exist && myData.cPlayingUri != NULL)
	{
		cd_message ("  cPlayingUri : %s", myData.cPlayingUri);
		cd_musicplayer_set_surface (myData.iPlayingStatus);
	}
	else
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
	CD_APPLET_LEAVE ();
}


/* Fonction executee a chaque changement de temps joué
 */
static void onElapsedChanged (DBusGProxy *player_proxy, int elapsed, gpointer data)
{
	CD_APPLET_ENTER;
	myData.iCurrentTime = elapsed;
	if(elapsed > 0)
	{
		cd_debug ("%s (%ds/%ds)", __func__, elapsed, myData.iSongLength);
		if(myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT)  // avec un '-' devant.
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed - myData.iSongLength);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.iQuickInfoType == MY_APPLET_PERCENTAGE)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int) (100.*elapsed/myData.iSongLength));
			CD_APPLET_REDRAW_MY_ICON;
		}
	}
	CD_APPLET_LEAVE ();
}


/*static void onSongPropertyChanged (DBusGProxy *player_proxy, const gchar *a, const gchar *cProperty, GValue *c, GValue *d, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("\n%s (%s)\n\n",__func__,cImageURI);
	g_free (myData.cCoverPath);
	myData.cCoverPath = g_strdup (cImageURI);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCoverPath);
	CD_APPLET_REDRAW_MY_ICON;
	myData.cover_exist = TRUE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
	CD_APPLET_LEAVE ();
}*/


////////////////////////////
// Definition du backend. //
////////////////////////////

/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur)
 */
void cd_rhythmbox_control (MyPlayerControl pControl, const char* song)
{
	cd_debug ("");
	const gchar *cCommand = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "previous";  // ou bien rhythmbox-client --previous
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = "playPause";  // ou bien rhythmbox-client --pause/--play
		break;

		case PLAYER_NEXT :
			cCommand = "next";  // ou bien rhythmbox-client --next
		break;
		
		case PLAYER_ENQUEUE :
		{
			gchar *cCommand2 = g_strdup_printf ("rhythmbox-client --enqueue %s", song);
			g_spawn_command_line_async (cCommand2, NULL);
			g_free (cCommand2);
		}
		break;
		
		default :
			return;
		break;
	}
	
	if (pControl == PLAYER_PLAY_PAUSE) // Cas special pour RB qui necessite un argument pour le PlayPause
	{
		gboolean bStartPlaying = (myData.iPlayingStatus != PLAYER_PLAYING);
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_player, cCommand, 
			G_TYPE_BOOLEAN, bStartPlaying,
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	else if (cCommand != NULL) 
	{
		cd_debug ("MP : Handler rhythmbox : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}

void cd_rhythmbox_get_cover_path (void)
{
	cd_rhythmbox_getSongInfos (FALSE);  // FALSE <=> on ne recupere que la couverture.
}

/* Initialise le backend de RB.
 */
static void cd_rhythmbox_start (void)
{
	// register to the signals
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "playingChanged",
		G_TYPE_BOOLEAN,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "playingChanged",
		G_CALLBACK(onChangePlaying), NULL, NULL);
	
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "playingUriChanged",
		G_TYPE_STRING,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "playingUriChanged",
		G_CALLBACK(onChangeSong), NULL, NULL);
	
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "elapsedChanged",
		G_TYPE_UINT,
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "elapsedChanged",
		G_CALLBACK(onElapsedChanged), NULL, NULL);
	
	/*TODO (or maybe not, if they included MPRIS2)
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "playingSongPropertyChanged",
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_VALUE,
		G_TYPE_VALUE,
		G_TYPE_INVALID);
	/*dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "playingSongPropertyChanged",
		G_CALLBACK(onSongPropertyChanged), NULL, NULL);*/
	
	// get the current state.
	_rhythmbox_getPlaying();
	_rhythmbox_getPlayingUri();
	cd_rhythmbox_getSongInfos (TRUE);  // TRUE <=> get all
	cd_musicplayer_update_icon (TRUE);
}

/*
 * We can't test if a bus is available because the dock is launched after.
 * Or we have to connect to a bus before and react if there is something after...
 *  but it's a bit annoying because we have to register the handler and then disable it (e.g. MPRIS vs MPRIS2)!
 * But with the version 2.90 of RB, we have to use MPRIS2 because rhythmbox-client is unavailable.
 */
static gboolean _is_MPRIS2_available (void)
{
	gchar *cResult = cairo_dock_launch_command_sync ("which rhythmbox-client");
	gboolean bResult = ! (cResult != NULL && *cResult == '/');
	g_free (cResult);
	return bResult;
}

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_rhythmbox_handler (void)
{
	if (_is_MPRIS2_available ())
	{
		cd_debug ("MP - MPRIS2 for RB seems to be available");
		MusicPlayerHandler *pHandler = cd_mpris_new_handler ();
		pHandler->cMprisService = "org.mpris.MediaPlayer2.rhythmbox";
		pHandler->appclass = "rhythmbox";
		pHandler->launch = "rhythmbox";
		pHandler->name = "Rhythmbox";
		cd_musicplayer_register_my_handler (pHandler);
		myData.bForceCoverNeedsTest = TRUE; // it seems RB copy the cover on its cache but it takes a few time...
	}
	else
	{
		cd_debug ("MP - Used RB DBus methods");
		MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
		pHandler->name = "Rhythmbox";
		pHandler->get_data = NULL;  // rien a faire vu que l'echange de donnees se fait entierement avec les proxys DBus.
		pHandler->stop = NULL;  // signals are disconnected when the proxy is destroyed.
		pHandler->start = cd_rhythmbox_start;  // renseigne les proprietes DBus et se connecte au bus.
		pHandler->control = cd_rhythmbox_control;
		pHandler->get_cover = cd_rhythmbox_get_cover_path;
		
		pHandler->appclass = "rhythmbox";
		pHandler->launch = "rhythmbox";
		pHandler->cMprisService = "org.gnome.Rhythmbox";
		pHandler->cMpris2Service = "org.mpris.MediaPlayer2.rhythmbox3";
		pHandler->path = "/org/gnome/Rhythmbox/Player";
		pHandler->interface = "org.gnome.Rhythmbox.Player";
		pHandler->path2 = "/org/gnome/Rhythmbox/Shell";
		pHandler->interface2 = "org.gnome.Rhythmbox.Shell";
		
		pHandler->cCoverDir = g_strdup_printf ("%s/.cache/rhythmbox/covers", g_getenv ("HOME"));
		pHandler->bSeparateAcquisition = FALSE;
		pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_ENQUEUE;
		pHandler->iLevel = PLAYER_EXCELLENT;
		
		cd_musicplayer_register_my_handler(pHandler);
	}
}

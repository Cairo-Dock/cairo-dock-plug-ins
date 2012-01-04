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
#include "applet-exaile.h"

/*
	myData.DBus_commands.service = "org.exaile.DBusInterface";
	myData.DBus_commands.path = "/DBusInterfaceObject";
	myData.DBus_commands.interface = "org.exaile.DBusInterface";
	myData.DBus_commands.play = "play_pause";
	myData.DBus_commands.pause = "play_pause";
	myData.DBus_commands.stop = "stop";
	myData.DBus_commands.next = "next_track";
	myData.DBus_commands.previous = "prev_track";
	myData.DBus_commands.get_title = "get_title";
	myData.DBus_commands.get_artist = "get_artist";
	myData.DBus_commands.get_album = "get_album";
	myData.DBus_commands.get_cover_path = "get_cover_path";
	myData.DBus_commands.get_status = "status";
	myData.DBus_commands.duration = "get_length";
	myData.DBus_commands.current_position = "current_position";
	get_rating/set_rating
*/

#define MP_DBUS_TYPE_PLAYER_STATUS G_TYPE_STRING

static inline int _get_time_from_string (const gchar *cTime)  // mm:ss
{
	int s=0, m = atoi (cTime);
	gchar *str = strchr (cTime, ':');
	if (str)
	{
		s = atoi (str+1);
	}
	return (60 * m + s);
}
static void cd_exaile_getSongInfos(void)
{
	gint uValue;	
	
	gchar *cQuery = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "query");
	cd_debug ("MP : query : %s\n", cQuery);  // status: playing self: Jiken artist: Yoshihisa Hirano, Hideki Taniuchi album: Death Note Original Soundtrack length: 2:49 position: %4 [0:07]
	if (cQuery == NULL)
	{
		myData.iPlayingStatus = PLAYER_STOPPED;
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
		return ;
	}
	gchar *str, *str2;
	
	str = g_strstr_len (cQuery, -1, "status:");
	g_return_if_fail (str != NULL);
	str += 8;
	if (strncmp (str, "playing", 7) == 0)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else if (strncmp (str, "paused", 6) == 0)
		myData.iPlayingStatus = PLAYER_PAUSED;
	else
		myData.iPlayingStatus = PLAYER_STOPPED;
	cd_debug ("  iPlayingStatus <- %d\n", myData.iPlayingStatus);
	if (myData.iPlayingStatus != PLAYER_PLAYING)
	{
		cd_debug ("exaile ne joue rien, on quitte\n");
		g_free (cQuery);
		return ;
	}
	
	str = g_strstr_len (str, -1, "self:");
	g_return_if_fail (str != NULL);
	str += 6;
	str2 = g_strstr_len (str, -1, "artist:");
	g_return_if_fail (str2 != NULL);
	g_free (myData.cTitle);
	myData.cTitle = g_strndup (str, str2 - str);
	cd_debug ("  cTitle <- %s\n", myData.cTitle);
	
	str = str2 + 8;
	str2 = g_strstr_len (str, -1, "album:");
	g_return_if_fail (str2 != NULL);
	g_free (myData.cArtist);
	myData.cArtist = g_strndup (str, str2 - str);
	cd_debug ("  cArtist <- %s\n", myData.cArtist);
	
	str = str2 + 7;
	str2 = g_strstr_len (str, -1, "length:");
	g_return_if_fail (str2 != NULL);
	g_free (myData.cAlbum);
	myData.cAlbum = g_strndup (str, str2 - str);
	cd_debug ("  cAlbum <- %s\n", myData.cAlbum);
	
	str = str2 + 8;
	str2 = g_strstr_len (str, -1, "position:");
	g_return_if_fail (str2 != NULL);
	myData.iSongLength = _get_time_from_string (str);
	cd_debug ("  iSongLength <- %d\n", myData.iSongLength);
	
	str = str2 + 10;
	str = strchr (str, '[');
	g_return_if_fail (str != NULL);
	myData.iCurrentTime = _get_time_from_string (str+1);
	cd_debug ("  iCurrentTime <- %d\n", myData.iCurrentTime);
	
	g_free (cQuery);
	
	g_free (myData.cRawTitle);
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cAlbum, myData.cTitle);
}


static void cd_exaile_getCoverPath (void)
{
	gchar *cCoverPath = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "get_cover_path");
	if (g_str_has_suffix (cCoverPath, "nocover.png"))
	{
		g_free (cCoverPath);
		cCoverPath = NULL;
	}
	if (cCoverPath != NULL)
		cd_debug ("MP : Couverture de exaile : %s\n", cCoverPath);  /// gerer le cas "nocover.jpg" ...
	else
		cd_debug ("MP : Pas de couverture chez exaile\n");
	cd_musicplayer_set_cover_path (cCoverPath);
	g_free (cCoverPath);
}


  ////////////////////////////
 // Definition du backend. //
////////////////////////////

/* Controle du lecteur
 */
static void cd_exaile_control (MyPlayerControl pControl, const char* file)
{
	const gchar *cCommand = NULL;
	
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "prev_track";
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = "play_pause";
		break;

		case PLAYER_NEXT :
			cCommand = "next_track";
		break;

		default :
			return;
		break;
	}
	
	if (cCommand != NULL)
	{
		cd_debug ("MP : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}

/* Recupere tout chaque seconde (aucun signal).
 */
static void cd_exaile_get_data (void)
{
	cd_debug ("Exaile is running\n");
	cd_exaile_getSongInfos ();
	if (myData.iPlayingStatus == PLAYER_PLAYING && cairo_dock_strings_differ (myData.cRawTitle, myData.cPreviousRawTitle))
		cd_exaile_getCoverPath ();
	else if (myData.iPlayingStatus == PLAYER_STOPPED)  // en pause le temps et la chanson reste constants.
	{
		myData.iCurrentTime = 0;
	}
	cd_message (" myData.iCurrentTime <- %d", __func__, myData.iCurrentTime);
	
}

/* Initialise le backend de EX.
 */
static void cd_exaile_start (void)
{
	// get the current state.
	cd_exaile_getSongInfos ();
	cd_exaile_getCoverPath ();
	cd_musicplayer_update_icon ();
}

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_exaile_handler (void) { //On enregistre notre lecteur
	//cd_debug ("");
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Exaile";
	pHandler->get_data = cd_exaile_get_data;
	pHandler->stop = NULL;
	pHandler->start = cd_exaile_start;  // we could also let the loop get the state on the first iteration.
	pHandler->control = cd_exaile_control;
	pHandler->get_cover = NULL;
	pHandler->cCoverDir = NULL;  /// visiblement il sait gerer les covers, sauf que je l'ai jamais vu en afficher une...
	
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT;
	pHandler->appclass = "exaile.py";
	pHandler->launch = "exaile";
	pHandler->cMprisService = "org.exaile.DBusInterface";
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_BAD;  // API DBus moisie sans aucun signal.
	
	pHandler->cMprisService = "org.exaile.DBusInterface";
	pHandler->path = "/DBusInterfaceObject";
	pHandler->interface = "org.exaile.DBusInterface";
	pHandler->path2 = NULL;
	pHandler->interface2 = NULL;
	
	cd_musicplayer_register_my_handler (pHandler);
}

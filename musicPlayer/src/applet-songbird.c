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
#include "applet-songbird.h"



void cd_songbird_getSongInfos (void)
{
	if( myData.cPreviousRawTitle )
	{
		g_free( myData.cPreviousRawTitle ); myData.cPreviousRawTitle = NULL;
	}
	if( myData.cRawTitle )
	{
		myData.cPreviousRawTitle = g_strdup(myData.cRawTitle);
	}
	
	myData.cAlbum = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_album);
	myData.cArtist = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_artist);
	myData.cTitle = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_title);
	//Artist & Title = RawTitle
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist,myData.cTitle );
	//cd_message("MP : %s - %s - %s", myData.cRawTitle, myData.cArtist, myData.cAlbum);
}


void cd_songbird_free_data (void) { //Permet de libéré la mémoire prise par notre controleur
	musicplayer_dbus_disconnect_from_bus();
}

/* Controle du lecteur */
void cd_songbird_control (MyPlayerControl pControl, char* nothing) { //Permet d'effectuer les actions de bases sur le lecteur
	return;
}


/* Fonction de lecture des infos */
void cd_songbird_read_data (void) {
	if (myData.dbus_enable)
	{
		if (myData.bIsRunning)
		{
			cd_musicplayer_getStatus_string("playing", "paused", "stopped"); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.iPlayingStatus == PLAYER_PLAYING)
			{
				cd_songbird_getSongInfos(); // On récupère toutes les infos de la piste en cours
			}	
		}
		else
		{
			cd_debug("MP : lecteur non ouvert");
			myData.iPlayingStatus = PLAYER_NONE;	
		}
	}
	else
	{
		cd_debug("MP : Impossible d'accéder au bus");
		myData.iPlayingStatus = PLAYER_BROKEN;
	}
	
}

void cd_songbird_load_dbus_commands (void)
{
	myData.DBus_commands.service = "org.mozilla.songbird";
	myData.DBus_commands.path = "/org/mozilla/songbird";
	myData.DBus_commands.interface = "org.mozilla.songbird";
	myData.DBus_commands.previous = "prev_track";
	myData.DBus_commands.get_title = "getTitle";
	myData.DBus_commands.get_artist = "getArtist";
	myData.DBus_commands.get_album = "getAlbum";
	myData.DBus_commands.get_status = "getStatus";
	//cd_debug("MP : Chargement des fonctions DBus effectué");	
}


void cd_musicplayer_register_songbird_handler (void) { //On enregistre notre lecteur
	MusicPlayerHandeler *pSongbird = g_new0 (MusicPlayerHandeler, 1);
	pSongbird->read_data = cd_songbird_read_data;
	pSongbird->free_data = cd_songbird_free_data;
	pSongbird->configure = cd_songbird_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//cd_debug("MP : Valeur de configure : %s", pSongbird->configure);
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pSongbird->control = cd_songbird_control;
	pSongbird->appclass = g_strdup("Songbird"); //Toujours g_strdup sinon l'applet plante au free_handler
	pSongbird->name = g_strdup("Songbird");
	pSongbird->launch = g_strdup("songbird");
	pSongbird->cMprisService = "org.mozilla.songbird";
	pSongbird->iPlayerControls = 0;  // ne gere rien !
	pSongbird->iPlayer = MP_SONGBIRD;
	pSongbird->bSeparateAcquisition = FALSE;
	cd_musicplayer_register_my_handler(pSongbird,"Songbird");
}

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
#include "applet-listen.h"


//Les Fonctions
static void cd_listen_getSongInfos(void)
{
	if( myData.cPreviousRawTitle )
	{
		g_free( myData.cPreviousRawTitle ); myData.cPreviousRawTitle = NULL;
	}
	if( myData.cRawTitle )
	{
		myData.cPreviousRawTitle = g_strdup(myData.cRawTitle);
	}
	myData.cRawTitle=cairo_dock_dbus_get_string (myData.dbus_proxy_player, "current_playing");	
}

/* Controle du lecteur */
static void cd_listen_control (MyPlayerControl pControl, const char *file) { //Permet d'effectuer les actions de bases sur le lecteur
	cd_debug ("");
	
	const gchar *cCommand = NULL;
	/* Conseil de ChangFu pour redetecter le titre à coup sûr */
	g_free (myData.cRawTitle);
	myData.cRawTitle = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "previous";
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = "play_pause";
		break;

		case PLAYER_NEXT :
			cCommand = "next";
		break;

		default :
			return;
		break;
	}
	
	if (cCommand != NULL) {
		cd_debug ("MP : Handler Listen : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}

/* Fonction de lecture des infos */
static void cd_listen_get_data (void)
{
	cd_listen_getSongInfos();  // On recupere toutes les infos de la piste en cours
}


void cd_musicplayer_register_listen_handler (void) { //On enregistre notre lecteur
	MusicPlayerHandler *pHandler = g_new0 (MusicPlayerHandler, 1);
	pHandler->name = "Listen";
	pHandler->get_data = cd_listen_get_data;
	pHandler->stop = NULL;
	pHandler->start = NULL;  // nothing to do, since there is no signal to connect to, and we'll get the stats on the first iteration of the loop.
	pHandler->control = cd_listen_control;
	pHandler->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT;
	pHandler->appclass = "listen.py";
	pHandler->launch = "listen";  /// a verifier ..
	
	pHandler->cMprisService = "org.gnome.Listen";
	pHandler->path = "/org/gnome/listen";
	pHandler->interface = "org.gnome.Listen";
	pHandler->path2 = NULL;
	pHandler->interface2 = NULL;
	
	pHandler->bSeparateAcquisition = FALSE;
	pHandler->iLevel = PLAYER_BAD;
cd_musicplayer_register_my_handler (pHandler);
}

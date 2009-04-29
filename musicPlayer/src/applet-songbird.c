/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Yann SLADEK (for any bug report, please mail me to mav@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)
Rémy Robertson (changfu@cairo-dock.org)

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

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
/* Ici Songbird ne gère aucune commande */
gboolean cd_songbird_ask_control (MyPlayerControl pControl) {
	return FALSE;
}

/* Fonction de connexion au bus de songbird */
void cd_songbird_acquisition (void) {
	cd_musicplayer_check_dbus_connection();	
}


/* Fonction de lecture des infos */
void cd_songbird_read_data (void) {
	if (myData.dbus_enable)
	{
		if (myData.opening)
		{
			cd_musicplayer_getStatus_string("playing", "paused", "stopped"); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				cd_songbird_getSongInfos(); // On récupère toutes les infos de la piste en cours
			}	
		}
		else
		{
			cd_debug("MP : lecteur non ouvert");
			myData.pPlayingStatus = PLAYER_NONE;	
		}
	}
	else
	{
		cd_debug("MP : Impossible d'accéder au bus");
		myData.pPlayingStatus = PLAYER_BROKEN;
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


void cd_musicplayer_register_songbird_handeler (void) { //On enregistre notre lecteur
	MusicPlayerHandeler *pSongbird = g_new0 (MusicPlayerHandeler, 1);
	pSongbird->acquisition = cd_songbird_acquisition;
	pSongbird->read_data = cd_songbird_read_data;
	pSongbird->free_data = cd_songbird_free_data;
	pSongbird->configure = cd_songbird_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//cd_debug("MP : Valeur de configure : %s", pSongbird->configure);
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pSongbird->control = cd_songbird_control;
	pSongbird->ask_control = cd_songbird_ask_control;
	pSongbird->appclass = g_strdup("Songbird"); //Toujours g_strdup sinon l'applet plante au free_handler
	pSongbird->name = g_strdup("Songbird");
	pSongbird->iPlayer = MP_SONGBIRD;
	pSongbird->bSeparateAcquisition = FALSE;
	cd_musicplayer_register_my_handeler(pSongbird,"Songbird");
}

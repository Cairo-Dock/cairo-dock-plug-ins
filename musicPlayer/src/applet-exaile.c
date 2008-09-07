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
#include "applet-exaile.h"

CD_APPLET_INCLUDE_MY_VARS


//Les Fonctions


void _exaile_getTime (void)
{
	cd_debug ("");
	guchar* uValue=NULL;
	gchar* temps=NULL; 
	gchar* length=NULL;
	gint minutes, secondes;
	
	/* Récupération du temps total! */
	length = cd_musicplayer_getLength_string();
	if (length != NULL) { //Sinon ca plante!
		cd_debug ("Length : %s", length);
		temps = strtok (length, ":");
		minutes = atoi(temps);
		temps = strtok (NULL, "\0");
		secondes = atoi(temps);
		myData.iSongLength = secondes + 60 * minutes;
	} //Plante quand le player est stopper, d'ailleurs l'applet ne m'affiche aucun changement de status
	//Revois cette partie du code, pour le reste ca fonctionne maintenant.
	else {
		myData.iSongLength = 0;
	}
	
	/* On récupère le pourcentage de la position actuelle */
	uValue = cd_musicplayer_getCurPos_string();
	
	/* Calcul de la position actuelle */
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	if (uValue != NULL) //Sinon ca plante bien évidement...
		myData.iCurrentTime = (myData.iSongLength * (int) uValue) / 100;
	else 
		myData.iCurrentTime = 0;
	
	/* Décalage dû à l'utilisation du pourcentage par exaile : marchotte mais sans plus */
	if (myData.iPreviousCurrentTime == myData.iCurrentTime && myData.pPlayingStatus == PLAYER_PLAYING)
		myData.iCurrentTime = myData.iCurrentTime +1;
		
	//cd_debug("MP : Position actuelle : %i", myData.iCurrentTime);
}



void cd_exaile_free_data (void) //Permet de libéré la mémoire prise par notre controleur
{
	musicplayer_dbus_disconnect_from_bus();
}

/* Controle du lecteur */
void cd_exaile_control (MyPlayerControl pControl, char* nothing) //Permet d'effectuer les actions de bases sur le lecteur
{ 
	cd_debug ("");
	
	gchar *cCommand = NULL;
	/* Conseil de ChangFu pour redetecter le titre à coup sûr */
	g_free (myData.cRawTitle);
	myData.cRawTitle = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = myData.DBus_commands.previous;
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = myData.DBus_commands.play;
		break;

		case PLAYER_NEXT :
			cCommand = myData.DBus_commands.next;
		break;
		
		/*case PLAYER_ENQUEUE :
			// A faire
		break;*/
		
		default :
			return;
		break; //Ca ne bloque pas l'execution du code plus bas
	}
	
	if (cCommand != NULL) {
		cd_debug ("MP : Handeler Exaile : will use '%s'", cCommand);
		cd_musicplayer_dbus_command (cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_exaile_ask_control (MyPlayerControl pControl) 
{
	cd_debug ("");
	switch (pControl) {
		case PLAYER_PREVIOUS :
			return TRUE;
		break;
		
		case PLAYER_PLAY_PAUSE :
			return TRUE;		
		break;

		case PLAYER_NEXT :
			return TRUE;
		break;
		
		default :
			return FALSE;
		break;
	}
	
	return FALSE;
}

/* Fonction de connexion au bus de Exaile */
void cd_exaile_acquisition (void) 
{
	cd_musicplayer_check_dbus_connection();	
}


/* Fonction de lecture des infos */
void cd_exaile_read_data (void) 
{
	if (myData.opening)
	{
		if (myData.dbus_enable)
		{
			cd_musicplayer_getStatus_string(); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				cd_musicplayer_getSongInfos(); // On récupère toutes les infos de la piste en cours
				_exaile_getTime();
				cd_musicplayer_getCoverPath();
			}	
		}
		else
		{
			//cd_debug("MP : Impossible d'accéder au bus");
			myData.pPlayingStatus = PLAYER_BROKEN;
		}
	}
	else
	{
		//cd_debug("MP : lecteur non ouvert");
		myData.pPlayingStatus = PLAYER_NONE;

	}
	
}

void cd_exaile_load_dbus_commands (void)
{
	cd_debug ("");
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
}




void cd_musicplayer_register_exaile_handeler (void) { //On enregistre notre lecteur
	cd_debug ("");
	MusicPlayerHandeler *pExaile = g_new0 (MusicPlayerHandeler, 1);
	pExaile->acquisition = cd_exaile_acquisition;
	pExaile->read_data = cd_exaile_read_data;
	pExaile->free_data = cd_exaile_free_data;
	pExaile->configure = cd_exaile_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pExaile->control = cd_exaile_control;
	pExaile->ask_control = cd_exaile_ask_control;
	pExaile->appclass = g_strdup("exaile.py"); //Toujours g_strdup sinon l'applet plante au free_handler
	pExaile->name = g_strdup("Exaile");
	cd_musicplayer_register_my_handeler(pExaile, "Exaile");
}

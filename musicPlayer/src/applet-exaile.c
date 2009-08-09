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


//Les Fonctions
void cd_exaile_getSongInfos(void)
{
	gint uValue;	
	gchar* length=NULL;
	
	/* Récupération du temps total */
	length = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.duration);
	//cd_debug ("MP : Length : %s", length);
	if ((length != NULL))
	{
		gchar* temps=NULL; 
		gint minutes, secondes;
		temps = strtok (length, ":");
		minutes = atoi(temps);
		temps = strtok (NULL, ":");
		secondes = atoi(temps);
		myData.iSongLength = secondes + 60 * minutes;
		g_free(length); 
	}
	else myData.iSongLength = -1;
	
	/* On récupère le pourcentage de la position actuelle */
	uValue = (gint) cairo_dock_dbus_get_uchar (myData.dbus_proxy_player, myData.DBus_commands.current_position);

	/* Décalage dû à l'utilisation du pourcentage par exaile */	
	if (myData.iPreviousuValue == uValue && myData.pPlayingStatus == PLAYER_PLAYING && myData.iCurrentTime < ((myData.iSongLength * (uValue + 1)) / 100))
		myData.iCurrentTime = myData.iCurrentTime +1;
	else if (myData.iPreviousuValue != uValue)
		myData.iCurrentTime = (myData.iSongLength * uValue) / 100;
	
	/*Pour permettre de comparer le pourcentage au top suivant et squizzer le decalage*/
	myData.iPreviousuValue = uValue;
	/*Recuperation des infos de la piste*/
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
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
	cd_debug("MP : \n\t Artist - Title : %s \n\t Album : %s", myData.cRawTitle, myData.cAlbum);
}


void cd_exaile_getCoverPath (void)
{
	if (myData.cCoverPath != NULL) 
	{
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
	
	myData.cCoverPath = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_cover_path);
	if (myData.cCoverPath != NULL)
		cd_message("MP : Couverture -> %s", myData.cCoverPath);
	else
		cd_message("MP : Pas de couverture dispo");
	
	
}

/* Permet de libérer la mémoire prise par notre controleur */
void cd_exaile_free_data (void) {
	cd_debug("MP : Deconnexion de DBus");
	musicplayer_dbus_disconnect_from_bus();
	
}

/* Controle du lecteur */
void cd_exaile_control (MyPlayerControl pControl, char* nothing) { //Permet d'effectuer les actions de bases sur le lecteur
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

		default :
			return;
		break;
	}
	
	if (cCommand != NULL) {
		cd_debug ("MP : Handeler Exaile : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}


/* Fonction de connexion au bus de Exaile */
void cd_exaile_acquisition (void) {
	cd_musicplayer_check_dbus_connection();	
}


/* Fonction de lecture des infos */
void cd_exaile_read_data (void) {
	//cd_debug("MP : on va aller lire les donnees");
	if (myData.bIsRunning)
	{
		if (myData.dbus_enable)
		{
			cd_musicplayer_getStatus_string("paused", "playing", "stopped"); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				cd_exaile_getSongInfos(); // On récupère toutes les infos de la piste en cours
				cd_exaile_getCoverPath();
			}
			else if (myData.pPlayingStatus == PLAYER_PAUSED)
			{
				
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
	//cd_debug ("");
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
	//cd_debug ("");
	MusicPlayerHandeler *pExaile = g_new0 (MusicPlayerHandeler, 1);
	pExaile->acquisition = cd_exaile_acquisition;
	pExaile->read_data = cd_exaile_read_data;
	pExaile->free_data = cd_exaile_free_data;
	pExaile->configure = cd_exaile_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pExaile->control = cd_exaile_control;
	pExaile->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT;
	pExaile->appclass = "exaile.py";
	pExaile->name = "Exaile";
	pExaile->launch = "exaile";
	pExaile->iPlayer = MP_EXAILE;
	pExaile->bSeparateAcquisition = FALSE;
	cd_musicplayer_register_my_handeler (pExaile, "Exaile");
}

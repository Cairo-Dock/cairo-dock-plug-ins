/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Yann SLADEK (for any bug report, please mail me to mav@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

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
void cd_exaile_free_data (void) //Permet de libéré la mémoire prise par notre controleur
{
	cd_debug("MP : coucou ca passe pour free data");
}

void cd_exaile_control (MyPlayerControl pControl, char* nothing) //Permet d'effectuer les actions de bases sur le lecteur
{ 
	GError *erreur = NULL;
	
	if (pControl != PLAYER_JUMPBOX && pControl != PLAYER_SHUFFLE && pControl != PLAYER_REPEAT && pControl != PLAYER_ENQUEUE) 
	{
		g_free (myData.cRawTitle);
		myData.cRawTitle = NULL; //Reset the title to detect it for sure ;)
	}
	gchar *cCommand = NULL;
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = myData.DBus_commands.previous;
		break;
		case PLAYER_PLAY_PAUSE :
			cCommand = myData.DBus_commands.play;
		break;
		case PLAYER_STOP :
			
		break;
		case PLAYER_NEXT :
			cCommand = myData.DBus_commands.next;
		break;
		
		case PLAYER_ENQUEUE :
			// A faire
		break;
	}
	
	cd_debug ("Handeler Exaile : will use '%s'", cCommand);
	cd_musicplayer_dbus_command(cCommand);
}

/*Permet de renseigner l'applet des fonctions supportées par le lecteur
Un bon exemple est Banshee ou Exaile qui n'ont pas de jumpbox ni de commande stop
Ici on répond TRUE car xmms supporte toutes les options, pour banshee il faudra faire quelque if*/
gboolean cd_exaile_ask_control (MyPlayerControl pControl) 
{
	return TRUE;
}


void cd_exaile_acquisition (void) 
{
	cd_musicplayer_check_dbus_connection();	
}

//Fonction de lecture des infos.
void cd_exaile_read_data (void) 
{
	if (myData.dbus_enable)
	{
		if (myData.opening)
		{
			cd_message("MP : Bus ouvert et lecteur accessible pour Exaile");
			cd_musicplayer_getStatus_string(); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				if (cd_musicplayer_check_for_changes()) //On vérifie si les données ont changé
				{
					cd_musicplayer_getSongInfos(); // On récupère toutes les infos de la piste en cours
					//cd_musicplayer_change_desklet_data();
				}
				else 
				{
					cd_musicplayer_getSongInfos(); // On récupère toutes les infos de la piste en cours	
				}
			}
			else // La lecture est stoppé, on met l'icone appropriée
			{
				myData.pPlayingStatus = PLAYER_STOPPED; 
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
		myData.pPlayingStatus = PLAYER_BROKEN;
	}
	
}

void cd_exaile_dbus (void)
{
	cd_debug("MP : Chargement des fonctions DBus effectué");
	cd_musicplayer_load_dbus_commands();
}

void cd_musicplayer_register_exaile_handeler (void) { //On enregistre notre lecteur
	MusicPlayerHandeler *pExaile = g_new0 (MusicPlayerHandeler, 1);
	pExaile->acquisition = cd_exaile_acquisition;
	pExaile->read_data = cd_exaile_read_data;
	pExaile->free_data = cd_exaile_free_data;
	pExaile->configure = cd_exaile_dbus; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pExaile->control = cd_exaile_control;
	pExaile->ask_control = cd_exaile_ask_control;
	pExaile->appclass = g_strdup("exaile.py"); //Toujours g_strdup sinon l'applet plante au free_handler
	pExaile->name = g_strdup("Exaile");
	cd_musicplayer_register_my_handeler(pExaile,"Exaile");
	/* Ca passe avec ces valeurs
	pExaile->name = g_strdup("Exaile");
	cd_musicplayer_register_my_handeler(pExaile,"Exaile.py");
	*/
}

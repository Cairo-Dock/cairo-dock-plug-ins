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
#include "applet-listen.h"

CD_APPLET_INCLUDE_MY_VARS


//Les Fonctions

static void cd_listen_getSongInfos(void)
{
	gchar* playing_uri;
	myData.cPreviousRawTitle = myData.cRawTitle;
	myData.cRawTitle=cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_title);	
}




/* Permet de libérer la mémoire prise par notre controleur */
void cd_listen_free_data (void)
{
	cd_debug("MP : Deconnexion de DBus");
	musicplayer_dbus_disconnect_from_bus();
}



/* Controle du lecteur */
void cd_listen_control (MyPlayerControl pControl, char* nothing) //Permet d'effectuer les actions de bases sur le lecteur
{ 
	cd_debug ("");
	
	static gchar *cCommand = NULL;
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
		cd_debug ("MP : Handeler Listen : will use '%s'", cCommand);
		cd_musicplayer_dbus_command (cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_listen_ask_control (MyPlayerControl pControl) 
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

/* Fonction de connexion au bus de listen */
void cd_listen_acquisition (void) 
{
	cd_musicplayer_check_dbus_connection();	
}


/* Fonction de lecture des infos */
void cd_listen_read_data (void) 
{
	if (myData.opening)
	{
		if (myData.dbus_enable)
		{
				cd_listen_getSongInfos(); // On récupère toutes les infos de la piste en cours
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

void cd_listen_load_dbus_commands (void)
{
	cd_debug ("");
	myData.DBus_commands.service = "org.gnome.Listen";
	myData.DBus_commands.path = "/org/gnome/listen";
	myData.DBus_commands.interface = "org.gnome.Listen";
	myData.DBus_commands.play = "play_pause";
	myData.DBus_commands.pause = "play_pause";
	myData.DBus_commands.stop = "";
	myData.DBus_commands.next = "next";
	myData.DBus_commands.previous = "previous";
	myData.DBus_commands.get_title = "current_playing";
	myData.DBus_commands.get_artist = "";
	myData.DBus_commands.get_album = "";
	myData.DBus_commands.get_cover_path = "";
	myData.DBus_commands.get_status = "";
	myData.DBus_commands.duration = "";
	myData.DBus_commands.current_position = "";
}




void cd_musicplayer_register_listen_handeler (void) { //On enregistre notre lecteur
	cd_debug ("");
	MusicPlayerHandeler *plisten = g_new0 (MusicPlayerHandeler, 1);
	plisten->acquisition = cd_listen_acquisition;
	plisten->read_data = cd_listen_read_data;
	plisten->free_data = cd_listen_free_data;
	plisten->configure = cd_listen_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	plisten->control = cd_listen_control;
	plisten->ask_control = cd_listen_ask_control;
	plisten->appclass = g_strdup("listen.py"); //Toujours g_strdup sinon l'applet plante au free_handler
	plisten->name = g_strdup("Listen");
	cd_musicplayer_register_my_handeler(plisten, "Listen");
}


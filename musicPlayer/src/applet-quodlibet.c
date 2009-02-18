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
#include "applet-quodlibet.h"

CD_APPLET_INCLUDE_MY_VARS


//Les Fonctions

void cd_quodlibet_getSongInfos(void)
{
	GHashTable *data_list = NULL;
	gchar *value;
	
	if(dbus_g_proxy_call (myData.dbus_proxy_player, "CurrentSong", NULL,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_STRING),
		&data_list,
		G_TYPE_INVALID))
	{
	myData.iPreviousTrackNumber=myData.iTrackNumber;
	myData.iPreviousCurrentTime=myData.iCurrentTime;
	// Tester si la table de hachage n'est pas vide
		g_free (myData.cArtist);
		value = (const char *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL) myData.cArtist = g_strdup(value);
		else myData.cArtist = NULL;
		//cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
		
		g_free (myData.cAlbum);
		value = (const char *) g_hash_table_lookup(data_list, "album");
		if (value != NULL) myData.cAlbum = g_strdup(value);
		else myData.cAlbum = NULL;
		//cd_message ("\tMP : playing_album <- %s", myData.cAlbum);
		
		g_free (myData.cTitle);
		value = (const char *) g_hash_table_lookup(data_list, "title");
		if (value != NULL) myData.cTitle = g_strdup(value);
		else myData.cTitle = NULL;
		//cd_message ("\tMP : playing_title <- %s", myData.cTitle);
		
		value = (const char *) g_hash_table_lookup(data_list, "tracknumber");
		if (value != NULL) myData.iTrackNumber = g_strdup(value);
		else myData.iTrackNumber = 0;
		//cd_message ("\tMP : playing_track <- %s", myData.iTrackNumber);
		
		value = (const char *) g_hash_table_lookup(data_list, "~#length");
		if (value != NULL) 
			myData.iSongLength = g_strdup(value);
		else myData.iSongLength = 0;
		//cd_message ("\tMP : playing_duration <- %s", myData.iSongLength);
		
		g_value_unset(value);	
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("MP : Can't get song properties");
	}
	
	/* Probleme sur la recup' du temps ecoule, impossible de determiner le format de sortie*/
	//myData.iCurrentTime = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_player, myData.DBus_commands.current_position);
	//cd_message ("\tMP : current_position <- %i", myData.iCurrentTime);
		
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
	
}


/* Permet de libérer la mémoire prise par notre controleur */
void cd_quodlibet_free_data (void)
{
	cd_debug("MP : Deconnexion de DBus");
	musicplayer_dbus_disconnect_from_bus();
}

/* Controle du lecteur */
void cd_quodlibet_control (MyPlayerControl pControl, char* nothing) //Permet d'effectuer les actions de bases sur le lecteur
{ 
	//cd_debug ("");
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
		cd_debug ("MP : Handeler QuodLibet : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_quodlibet_ask_control (MyPlayerControl pControl) 
{
	//cd_debug ("");
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

/* Fonction de connexion au bus de quodlibet */
void cd_quodlibet_acquisition (void) 
{
	cd_musicplayer_check_dbus_connection();	
}


/* Fonction de lecture des infos */
void cd_quodlibet_read_data (void) 
{
	if (myData.opening)
	{
		if (myData.dbus_enable)
		{
			cd_musicplayer_getStatus_integer(); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				cd_quodlibet_getSongInfos(); // On récupère toutes les infos de la piste en cours
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

void cd_quodlibet_load_dbus_commands (void)
{
	//cd_debug ("");
	myData.DBus_commands.service = "net.sacredchao.QuodLibet";
	myData.DBus_commands.path = "/net/sacredchao/QuodLibet";
	myData.DBus_commands.interface = "net.sacredchao.QuodLibet";
	myData.DBus_commands.play = "PlayPause";
	myData.DBus_commands.pause = "PlayPause";
	myData.DBus_commands.stop = "";
	myData.DBus_commands.next = "Next";
	myData.DBus_commands.previous = "Previous";
	myData.DBus_commands.get_title = "CurrentSong";
	myData.DBus_commands.get_artist = "";
	myData.DBus_commands.get_album = "";
	myData.DBus_commands.get_cover_path = "";
	myData.DBus_commands.get_status = "IsPlaying";
	myData.DBus_commands.duration = "";
	myData.DBus_commands.current_position = "GetPosition";
}




void cd_musicplayer_register_quodlibet_handeler (void) { //On enregistre notre lecteur
	//cd_debug ("");
	MusicPlayerHandeler *pquodlibet = g_new0 (MusicPlayerHandeler, 1);
	pquodlibet->acquisition = cd_quodlibet_acquisition;
	pquodlibet->read_data = cd_quodlibet_read_data;
	pquodlibet->free_data = cd_quodlibet_free_data;
	pquodlibet->configure = cd_quodlibet_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pquodlibet->control = cd_quodlibet_control;
	pquodlibet->ask_control = cd_quodlibet_ask_control;
	pquodlibet->appclass = g_strdup("Quodlibet"); //Toujours g_strdup sinon l'applet plante au free_handler
	pquodlibet->name = g_strdup("QuodLibet");
	cd_musicplayer_register_my_handeler(pquodlibet, "QuodLibet");
}


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
#include "applet-banshee.h"


//Les Fonctions - CF: Il y a d'autre personne qui passe dans les .c, pensez a la visibilité de vos codes...
void cd_banshee_getSongInfos (void)
{
	GHashTable *data_list = NULL;
	GValue *value;
	
	if (dbus_g_proxy_call (myData.dbus_proxy_shell, myData.DBus_commands.get_title, NULL, G_TYPE_INVALID,
		dbus_g_type_get_map ("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		  &data_list,
		  G_TYPE_INVALID))
	{
		myData.iPreviousTrackNumber = myData.iTrackNumber;
		
		g_free (myData.cArtist);
		value = (GValue *) g_hash_table_lookup (data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cArtist = g_strdup (g_value_get_string(value));
		else
		  myData.cArtist = NULL;
		//cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
		
		g_free (myData.cAlbum);
		value = (GValue *) g_hash_table_lookup (data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cAlbum = g_strdup (g_value_get_string(value));
		else
		  myData.cAlbum = NULL;
		//cd_message ("\tMP : playing_album <- %s", myData.cAlbum);
		
		g_free (myData.cTitle);
		value = (GValue *) g_hash_table_lookup (data_list, "name");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cTitle = g_strdup (g_value_get_string (value));
		else
		  myData.cTitle = NULL;
		//cd_message ("\tMP : playing_title <- %s", myData.cTitle);	
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_INT(value))
		  myData.iTrackNumber = g_value_get_int(value);
		else
		  myData.iTrackNumber = 0;
		//cd_message ("\tMP : playing_track <- %d", myData.iTrackNumber);
		
		g_value_unset(value);
		g_hash_table_destroy (data_list);
	}
	
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	myData.iSongLength = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_shell, myData.DBus_commands.duration) / 1000;
	myData.iCurrentTime = cairo_dock_dbus_get_uinteger (myData.dbus_proxy_shell, myData.DBus_commands.current_position) / 1000;

	if (myData.cPreviousRawTitle)
	{
		g_free (myData.cPreviousRawTitle);
		myData.cPreviousRawTitle = NULL;
	}
	if (myData.cRawTitle)
	{
		myData.cPreviousRawTitle = g_strdup (myData.cRawTitle);
	}
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}


void cd_banshee_free_data (void) { //Permet de libéré la mémoire prise par notre controleur
	musicplayer_dbus_disconnect_from_bus ();
	musicplayer_dbus_disconnect_from_bus_Shell ();
}


/* Controle du lecteur */
void cd_banshee_control (MyPlayerControl pControl, char* nothing) { //Permet d'effectuer les actions de bases sur le lecteur
	gchar *cCommand = NULL;
	/* Conseil de ChangFu pour redetecter le titre à coup sûr */
	g_free (myData.cRawTitle);
	myData.cRawTitle = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = myData.DBus_commands.previous;
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = myData.DBus_commands.play_pause;
		break;

		case PLAYER_NEXT :
			cCommand = myData.DBus_commands.next;
		break;

		default :
			return;
	}
	
	if (pControl == PLAYER_PLAY_PAUSE)
	{
		cd_debug ("MP : Handeler banshee : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_shell,cCommand);
	}
	else
	{
		cd_debug ("MP : Handeler banshee : will use '%s'", cCommand);
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_player, cCommand, 
		  G_TYPE_BOOLEAN, FALSE,
		  G_TYPE_INVALID,
		  G_TYPE_INVALID);
		//cairo_dock_dbus_call(myData.dbus_proxy_player,cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_banshee_ask_control (MyPlayerControl pControl) {
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
	}
}

/* Fonction de connexion au bus de banshee */
void cd_banshee_acquisition (void) {
	cd_musicplayer_check_dbus_connection_with_two_interfaces();
}


/* Fonction de lecture des infos */
void cd_banshee_read_data (void) {
	if (myData.dbus_enable)
	{
		if (myData.opening)
		{
			cd_musicplayer_getStatus_string("paused", "playing", NULL); // On récupère l'état de la lecture (play/pause/stop)
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				cd_banshee_getSongInfos(); // On récupère toutes les infos de la piste en cours
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

void cd_banshee_load_dbus_commands (void)
{
	//cd_debug("MP : On charge les commande pour Banshee");
	myData.DBus_commands.service = "org.bansheeproject.Banshee";
	myData.DBus_commands.path = "/org/bansheeproject/Banshee/PlaybackController";
	myData.DBus_commands.interface = "org.bansheeproject.Banshee.PlaybackController";
	myData.DBus_commands.path2 = "/org/bansheeproject/Banshee/PlayerEngine";
	myData.DBus_commands.interface2 = "org.bansheeproject.Banshee.PlayerEngine";	
	myData.DBus_commands.play = "Play";
	myData.DBus_commands.pause = "Pause";
	myData.DBus_commands.play_pause = "TogglePlaying";
	myData.DBus_commands.next = "Next";
	myData.DBus_commands.previous = "Previous";
	myData.DBus_commands.get_title = "GetCurrentTrack";
	myData.DBus_commands.get_artist = "";
	myData.DBus_commands.get_album = "";
	myData.DBus_commands.get_cover_path = "";
	myData.DBus_commands.get_status = "GetCurrentState";
	myData.DBus_commands.duration = "GetLength";
	myData.DBus_commands.current_position = "GetPosition";
	//cd_debug("MP : Chargement des fonctions DBus effectué");
	return;
}


void cd_musicplayer_register_banshee_handeler (void) { //On enregistre notre lecteur
	MusicPlayerHandeler *pBanshee = g_new0 (MusicPlayerHandeler, 1);
	pBanshee->acquisition = cd_banshee_acquisition;
	pBanshee->read_data = cd_banshee_read_data;
	pBanshee->free_data = cd_banshee_free_data;
	pBanshee->configure = cd_banshee_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pBanshee->control = cd_banshee_control;
	pBanshee->ask_control = cd_banshee_ask_control;
	pBanshee->appclass = g_strdup ("Banshee"); //Toujours g_strdup sinon l'applet plante au free_handler
	pBanshee->name = g_strdup ("Banshee");
	pBanshee->iPlayer = MP_BANSHEE;
	pBanshee->bSeparateAcquisition = FALSE;
	cd_musicplayer_register_my_handeler(pBanshee,"Banshee");
}


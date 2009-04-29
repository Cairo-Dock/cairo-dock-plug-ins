
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
#include "applet-amarok2.h"



//Les Fonctions
void cd_amarok2_getSongInfos (void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	
	if(dbus_g_proxy_call (myData.dbus_proxy_player, myData.DBus_commands.get_title, NULL,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
	myData.pPreviousPlayingStatus = myData.pPlayingStatus;
	myData.iPreviousTrackNumber = myData.iTrackNumber;
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	// Tester si la table de hachage n'est pas vide
		g_free (myData.cArtist);
		value = (GValue *) g_hash_table_lookup (data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cArtist = g_strdup (g_value_get_string (value));
		else
		  myData.cArtist = NULL;
		cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
		
		g_free (myData.cAlbum);
		value = (GValue *) g_hash_table_lookup (data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cAlbum = g_strdup (g_value_get_string (value));
		else
		  myData.cAlbum = NULL;
		cd_message ("\tMP : playing_album <- %s", myData.cAlbum);
		
		g_free (myData.cTitle);
		value = (GValue *) g_hash_table_lookup (data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cTitle = g_strdup (g_value_get_string (value));
		else
		  myData.cTitle = NULL;
		cd_message ("\tMP : playing_title <- %s", myData.cTitle);
		
		value = (GValue *) g_hash_table_lookup (data_list, "tracknumber");
		if (value != NULL && G_VALUE_HOLDS_UINT (value))
		  myData.iTrackNumber = g_value_get_uint (value);
		else
		  myData.iTrackNumber = 0;
		cd_message ("\tMP : playing_track <- %d", myData.iTrackNumber);
		
		value = (GValue *) g_hash_table_lookup (data_list, "time");
		if (value != NULL && G_VALUE_HOLDS_INT (value))
		  myData.iSongLength = (g_value_get_int (value)) / 60;
		else
		  myData.iSongLength = 0;
		cd_message ("\tMP : playing_duration <- %ds", myData.iSongLength);
		
		value = (GValue *) g_hash_table_lookup (data_list, "arturl");
		if (value != NULL && G_VALUE_HOLDS_STRING (value))
		  myData.cCoverPath = g_strdup (g_value_get_string (value));
		else
		  myData.cCoverPath = NULL;
		cd_message ("\tMP : playing_cover <- %s", myData.cCoverPath);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("MP : Can't get song properties");
		myData.cCoverPath = NULL;
	}
	
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}

//Fonction executée à chaque changement de musique
void onChangeTrack(DBusGProxy *player_proxy,GHashTable *data_list, gpointer data)
{
  //cd_debug("MP : On a change la ZIK");
  GValue *value;

  myData.pPreviousPlayingStatus = myData.pPlayingStatus;
  myData.iPreviousTrackNumber = myData.iTrackNumber;
  myData.iPreviousCurrentTime = myData.iCurrentTime;
  // Tester si la table de hachage n'est pas vide
  g_free (myData.cArtist);
  value = (GValue *) g_hash_table_lookup(data_list, "artist");
  if (value != NULL && G_VALUE_HOLDS_STRING (value))
    myData.cArtist = g_strdup (g_value_get_string (value));
  else
    myData.cArtist = NULL;
  cd_message ("\tMP : playing_artist <- %s", myData.cArtist);

  g_free (myData.cAlbum);
  value = (GValue *) g_hash_table_lookup (data_list, "album");
  if (value != NULL && G_VALUE_HOLDS_STRING (value))
    myData.cAlbum = g_strdup (g_value_get_string (value));
  else
    myData.cAlbum = NULL;
  cd_message ("\tMP : playing_album <- %s", myData.cAlbum);

  g_free (myData.cTitle);
  value = (GValue *) g_hash_table_lookup(data_list, "title");
  if (value != NULL && G_VALUE_HOLDS_STRING (value))
    myData.cTitle = g_strdup (g_value_get_string (value));
  else
    myData.cTitle = NULL;
  cd_message ("\tMP : playing_title <- %s", myData.cTitle);

  value = (GValue *) g_hash_table_lookup (data_list, "tracknumber");
  if (value != NULL && G_VALUE_HOLDS_UINT (value))
    myData.iTrackNumber = g_value_get_uint (value);
  else
    myData.iTrackNumber = 0;
  cd_message ("\tMP : playing_track <- %d", myData.iTrackNumber);

  value = (GValue *) g_hash_table_lookup (data_list, "time");
  if (value != NULL && G_VALUE_HOLDS_INT (value))
    myData.iSongLength = (g_value_get_int (value)) / 60;
  else
    myData.iSongLength = 0;
  cd_message ("\tMP : playing_duration <- %ds", myData.iSongLength);

  value = (GValue *) g_hash_table_lookup (data_list, "arturl");
  if (value != NULL && G_VALUE_HOLDS_STRING (value))
    myData.cCoverPath = g_strdup (g_value_get_string (value));
  else
    myData.cCoverPath = NULL;
  cd_message ("\tMP : playing_cover <- %s", myData.cCoverPath);

  myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}


//Fonction executée à chaque changement play/pause
void onChangeStatus(DBusGProxy *player_proxy, GValueArray *status, gpointer data)
{
  //cd_debug("MP : On a change le statut");
  cd_amarok2_getStatus();
}


void cd_amarok2_getStatus (void)
{
	GValueArray *s = 0;
	GValue *v;
	int status;
	
	dbus_g_proxy_call (myData.dbus_proxy_player, myData.DBus_commands.get_status, NULL,
	G_TYPE_INVALID,
	dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID),
	&s,
	G_TYPE_INVALID);
	
	v = g_value_array_get_nth(s, 0);
	status = g_value_get_int(v);
	
	cd_debug("MP : Status : %i",status);
	switch (status) 
	{
		case 0:
			myData.pPlayingStatus = PLAYER_PLAYING;
			break;
		case 1:
			myData.pPlayingStatus = PLAYER_PAUSED;
			break;
		case 2: 
			myData.pPlayingStatus = PLAYER_STOPPED;
			break;
		default:
			break;
	}
}

void cd_amarok2_proxy_connection (void)
{
	cd_debug ("MP : Debut des connexions aux proxys");
	dbus_g_proxy_add_signal (myData.dbus_proxy_player, "StatusChange",
			dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID),
			G_TYPE_INVALID);
	dbus_g_proxy_add_signal (myData.dbus_proxy_player, "TrackChange",
			dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
			G_TYPE_INVALID);
		
	dbus_g_proxy_connect_signal (myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK(onChangeStatus), NULL, NULL);
			
	dbus_g_proxy_connect_signal (myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK(onChangeTrack), NULL, NULL);
	
	cd_debug ("MP : Fin des connexions aux proxys");
}


void cd_amarok2_free_data (void) { //Permet de libérer la mémoire prise par notre controleur
	if (myData.dbus_proxy_player != NULL) {
		dbus_g_proxy_disconnect_signal (myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK (onChangeStatus), NULL);
		cd_debug ("MP : Signal statusChange deconnecte");
		
		dbus_g_proxy_disconnect_signal (myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK (onChangeTrack), NULL);
		cd_debug ("MP : Signal TrackChange deconnecte");
	}
	
	musicplayer_dbus_disconnect_from_bus ();
	
	cd_debug("MP : Deconnexion du bus effectuee");
}

/* Controle du lecteur */
void cd_amarok2_control (MyPlayerControl pControl, char* nothing) { //Permet d'effectuer les actions de bases sur le lecteur 
	cd_debug ("");
	
	gchar *cCommand = NULL;
		
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
		
		case PLAYER_STOP :
			cCommand = myData.DBus_commands.stop;
		break;
		
		/*case PLAYER_ENQUEUE :
			// A faire
		break;*/
		
		default :
			return;
		break;
	}
	
	if (cCommand != NULL)  {
		cd_debug ("MP : Handeler amarok2 : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_amarok2_ask_control (MyPlayerControl pControl)  {
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
		
		case PLAYER_STOP :
			return TRUE;
		default :
			return FALSE;
		break;
	}
	
	return FALSE;
}

/* Fonction de connexion au bus de amarok2 */
void cd_amarok2_acquisition (void)  {
	//cd_debug("MP : Vérification de la connexion DBus");
	myData.opening = cd_musicplayer_dbus_detection();
	//cd_debug("MP : Opening : %d", myData.opening);
	//cd_debug("MP : DBUS Enable : %d", myData.dbus_enable);
	if ((myData.opening) && (!myData.dbus_enable))
	{
		cd_debug("MP : On se connecte au bus");
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // On se connecte au bus
		if (myData.dbus_enable)
		{
			cd_debug ("MP : On s'est connecte au bus Player");
			cd_amarok2_proxy_connection();
			cd_debug ("MP : Connexions aux proxys OK");
			
			cd_amarok2_getStatus();
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{	
				cd_amarok2_getSongInfos();
				myData.iCurrentTime = (cairo_dock_dbus_get_integer (myData.dbus_proxy_player, myData.DBus_commands.current_position)) / 1000;
			}
		}	
	}
	else if ((myData.opening) && (myData.dbus_enable))
		//cd_debug("MP : Deja connecte au bus");
		;
	else
	{
		myData.dbus_enable = 0;
		//cd_debug("MP : Lecteur non ouvert");
	}
}


/* Fonction de lecture des infos */
void cd_amarok2_read_data (void)  {
	/*
	Rien a lire vu que l'echange de donnees se fait avec les proxys DBUS
	Sauf pour le temps ecoule
	*/
 
	/*cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
	cd_message ("\tMP : playing_title <- %s", myData.cTitle);
	cd_message ("\tMP : song length <- %d", myData.iSongLength);*/
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	myData.iCurrentTime = (cairo_dock_dbus_get_integer (myData.dbus_proxy_player, myData.DBus_commands.current_position)) / 1000;
	//cd_debug ("\tMP : elapsed time <- %d", myData.iCurrentTime);
}

void cd_amarok2_load_dbus_commands (void) {
	cd_debug ("");
	myData.DBus_commands.service = "org.kde.amarok";
	myData.DBus_commands.path = "/Player";
	myData.DBus_commands.interface = "org.freedesktop.MediaPlayer";
	myData.DBus_commands.play = "Play";
	myData.DBus_commands.pause = "Pause";
	myData.DBus_commands.stop = "Stop";
	myData.DBus_commands.next = "Next";
	myData.DBus_commands.previous = "Prev";
	myData.DBus_commands.get_title = "GetMetadata";
	myData.DBus_commands.get_status = "GetStatus";
	myData.DBus_commands.current_position = "PositionGet";
}

void cd_musicplayer_register_amarok2_handeler (void) { //On enregistre notre lecteur
	cd_debug ("");
	MusicPlayerHandeler *pAmarok2 = g_new0 (MusicPlayerHandeler, 1);
	pAmarok2->acquisition = cd_amarok2_acquisition;
	pAmarok2->read_data = cd_amarok2_read_data;
	pAmarok2->free_data = cd_amarok2_free_data;
	pAmarok2->configure = cd_amarok2_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pAmarok2->control = cd_amarok2_control;
	pAmarok2->ask_control = cd_amarok2_ask_control;
	pAmarok2->appclass = g_strdup ("amarok"); //Toujours g_strdup sinon l'applet plante au free_handler
	pAmarok2->name = g_strdup ("Amarok 2");
	pAmarok2->iPlayer = MP_AMAROK2;
	pAmarok2->bSeparateAcquisition = FALSE;
	cd_musicplayer_register_my_handeler (pAmarok2, "Amarok 2");
}

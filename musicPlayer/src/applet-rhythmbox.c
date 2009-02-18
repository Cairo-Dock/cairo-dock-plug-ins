
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
#include "applet-rhythmbox.h"

CD_APPLET_INCLUDE_MY_VARS


//Les Fonctions

void cd_rhythmbox_getSongInfos (void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	
	myData.cPlayingUri = cairo_dock_dbus_get_string(myData.dbus_proxy_player,"getPlayingUri");
	
	if(dbus_g_proxy_call (myData.dbus_proxy_shell, "getSongProperties", NULL,
		G_TYPE_STRING, myData.cPlayingUri,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
	myData.pPreviousPlayingStatus=myData.pPlayingStatus;
	myData.iPreviousTrackNumber=myData.iTrackNumber;
	myData.iPreviousCurrentTime=myData.iCurrentTime;
	// Tester si la table de hachage n'est pas vide
		g_free (myData.cArtist);
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cArtist = g_strdup (g_value_get_string(value));
		else myData.cArtist = NULL;
		cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
		
		g_free (myData.cAlbum);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cAlbum = g_strdup (g_value_get_string(value));
		else myData.cAlbum = NULL;
		cd_message ("\tMP : playing_album <- %s", myData.cAlbum);
		
		g_free (myData.cTitle);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cTitle = g_strdup (g_value_get_string(value));
		else myData.cTitle = NULL;
		cd_message ("\tMP : playing_title <- %s", myData.cTitle);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iTrackNumber = g_value_get_uint(value);
		else myData.iTrackNumber = 0;
		cd_message ("\tMP : playing_track <- %d", myData.iTrackNumber);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iSongLength = g_value_get_uint(value);
		else myData.iSongLength = 0;
		cd_message ("\tMP : playing_duration <- %ds", myData.iSongLength);
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
		if (value != NULL && G_VALUE_HOLDS_STRING(value))
		{
			const gchar *cString = g_value_get_string(value);
			cd_debug ("RB nous a refile cette adresse : %s", cString);
			if (cString != NULL)
			{
				if (strncmp (cString, "file://", 7) == 0)
				{
					GError *erreur = NULL;
					myData.cCoverPath = g_filename_from_uri (cString, NULL, &erreur);
					if (erreur != NULL)
					{
						cd_warning ("Attention : %s", erreur->message);
						g_error_free (erreur);
					}
				}
				else if (*cString == '/')
				{
					myData.cCoverPath = g_strdup (cString);
				}
			}
		}
		if (myData.cCoverPath == NULL)
		{
			gchar *cSongPath = g_filename_from_uri (myData.cPlayingUri, NULL, NULL);  // on teste d'abord dans le repertoire de la chanson.
			if (cSongPath != NULL)  // c'est un fichier local.
			{
				gchar *cSongDir = g_path_get_dirname (cSongPath);
				g_free (cSongPath);
				
				myData.cCoverPath = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, myData.cArtist, myData.cAlbum);
				cd_debug ("test de %s", myData.cCoverPath);
				if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
				{
					g_free (myData.cCoverPath);
					myData.cCoverPath = g_strdup_printf ("%s/cover.jpg", cSongDir);
					cd_debug ("  test de %s", myData.cCoverPath);
					if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
					{
						g_free (myData.cCoverPath);
						myData.cCoverPath = NULL;
					}
				}
				g_free (cSongDir);
			}
			
			if (myData.cCoverPath == NULL)  // on regarde maintenant dans le cache de RB.
			{
				myData.cCoverPath = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv ("HOME"), myData.cArtist, myData.cAlbum);  /// gerer le repertoire ~/.cache/rhythmbox/covers ...
			}
		}
		g_print ("  MP : playing_cover <- %s", myData.cCoverPath);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("  can't get song properties");
		myData.cCoverPath = NULL;
	}
	
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
	
}


void cd_rhythmbox_proxy_connection (void)
{
	cd_debug("MP : Debut des connexions aux proxys");
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "playingChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "playingUriChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "elapsedChanged",
			G_TYPE_UINT,
			G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.dbus_proxy_player, "rb:CovertArt-uri",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL, NULL);
			
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL, NULL);
		
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL, NULL);
		
	dbus_g_proxy_connect_signal(myData.dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCovertArtChanged), NULL, NULL);	
	cd_debug("MP : Fin des connexions aux proxys");
}


void cd_rhythmbox_free_data (void) //Permet de libérer la mémoire prise par notre controleur
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL);
		cd_debug ("playingChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL);
		cd_debug ("playingUriChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL);
		cd_debug ("elapsedChanged deconnecte");
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCovertArtChanged), NULL);
		cd_debug ("onCovertArtChanged deconnecte");
	}
	
	musicplayer_dbus_disconnect_from_bus();
	musicplayer_dbus_disconnect_from_bus_Shell();
	
	cd_debug("MP : Deconnexion du bus effectuee");
}

/* Controle du lecteur */
void cd_rhythmbox_control (MyPlayerControl pControl, char* nothing) //Permet d'effectuer les actions de bases sur le lecteur
{ 
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
		
		/*case PLAYER_ENQUEUE :
			// A faire
		break;*/
		
		default :
			return;
		break;
	}
	
	if (pControl == PLAYER_PLAY_PAUSE) // Cas special pour RB qui necessite un argument pour le PlayPause
	{
		gboolean toggle;
		cd_debug ("MP : Handeler rhythmbox : will use Play Pause", cCommand);
		if (myData.pPlayingStatus == PLAYER_PLAYING) toggle=FALSE;
		else toggle=TRUE;
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_player, "playPause", 
		G_TYPE_BOOLEAN, toggle,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	}
	else if (cCommand != NULL) 
	{
		cd_debug ("MP : Handeler rhythmbox : will use '%s'", cCommand);
		cairo_dock_dbus_call(myData.dbus_proxy_player, cCommand);
	}
}

/* Permet de renseigner l'applet des fonctions supportées par le lecteur */
gboolean cd_rhythmbox_ask_control (MyPlayerControl pControl) 
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

/* Fonction de connexion au bus de rhythmbox */
void cd_rhythmbox_acquisition (void) 
{
	cd_debug("MP : Vérification de la connexion DBus");
	myData.opening = cd_musicplayer_dbus_detection();
	/*cd_debug("MP : Opening : %d", myData.opening);
	cd_debug("MP : DBUS Enable : %d", myData.dbus_enable);*/
	if ((myData.opening) && (!myData.dbus_enable))// && (!myData.dbus_enable_shell))
	{
		cd_debug("MP : On se connecte au bus");
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // On se connecte au bus
		if (myData.dbus_enable)
			//cd_debug("MP : On s'est connecte au bus Player");
			
		myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell (); // On se connecte au bus
		if (myData.dbus_enable_shell)
			//cd_debug("MP : On s'est connecte au bus Shell");
		
		if ((myData.dbus_enable) && (myData.dbus_enable_shell))
			cd_rhythmbox_proxy_connection();
		
		cd_debug("MP : Connexions aux bus OK");
		
		if (cairo_dock_dbus_get_boolean(myData.dbus_proxy_player,"getPlaying"))
			myData.pPlayingStatus = PLAYER_PLAYING;
		else
			myData.pPlayingStatus = PLAYER_PAUSED;
		cd_rhythmbox_getSongInfos;
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
void cd_rhythmbox_read_data (void) 
{
	/*
	Rien a lire vu que l'echange de donnees se fait avec les proxys DBUS
	*/ 
	/*cd_message ("\tMP : playing_artist <- %s", myData.cArtist);
	cd_message ("\tMP : playing_title <- %s", myData.cTitle);
	cd_message ("\tMP : elapsed time <- %d", myData.iCurrentTime);
	cd_message ("\tMP : song length <- %d", myData.iSongLength);*/
}

void cd_rhythmbox_load_dbus_commands (void)
{
	cd_debug ("");
	myData.DBus_commands.service = "org.gnome.Rhythmbox";
	myData.DBus_commands.path = "/org/gnome/Rhythmbox/Player";
	myData.DBus_commands.path2 = "/org/gnome/Rhythmbox/Shell";
	myData.DBus_commands.interface = "org.gnome.Rhythmbox.Player";
	myData.DBus_commands.interface2 = "org.gnome.Rhythmbox.Shell";
	myData.DBus_commands.play = "playPause";
	myData.DBus_commands.pause = "playPause";
	myData.DBus_commands.stop = "";
	myData.DBus_commands.next = "next";
	myData.DBus_commands.previous = "previous";
}




void cd_musicplayer_register_rhythmbox_handeler (void) { //On enregistre notre lecteur
	cd_debug ("");
	MusicPlayerHandeler *prhythmbox = g_new0 (MusicPlayerHandeler, 1);
	prhythmbox->acquisition = cd_rhythmbox_acquisition;
	prhythmbox->read_data = cd_rhythmbox_read_data;
	prhythmbox->free_data = cd_rhythmbox_free_data;
	prhythmbox->configure = cd_rhythmbox_load_dbus_commands; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	prhythmbox->control = cd_rhythmbox_control;
	prhythmbox->ask_control = cd_rhythmbox_ask_control;
	prhythmbox->appclass = g_strdup("rhythmbox"); //Toujours g_strdup sinon l'applet plante au free_handler
	prhythmbox->name = g_strdup("Rhythmbox");
	cd_musicplayer_register_my_handeler(prhythmbox, "rhythmbox");
}




//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	cd_message ("MP : %s (%s)",__func__,uri);
	
	g_free (myData.cPlayingUri);
	if(uri != NULL && *uri != '\0')
	{
		myData.cPlayingUri = g_strdup (uri);
		myData.opening = TRUE;
		cd_rhythmbox_getSongInfos();
	}
	else
	{
		myData.cPlayingUri = NULL;
		//myData.cover_exist = FALSE;
		g_free (myData.cArtist);
		myData.cArtist = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
		myData.iSongLength = 0;
		myData.iTrackNumber = 0;
		
		myData.opening = cd_musicplayer_dbus_detection();
	}
}

//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void onChangePlaying(DBusGProxy *player_proxy, gboolean playing, gpointer data)
{
	if (playing)
		myData.pPlayingStatus = PLAYER_PLAYING;
	else
		myData.pPlayingStatus = PLAYER_PAUSED;
}

//*********************************************************************************
// rhythmbox_elapsedChanged() : Fonction executée à chaque changement de temps joué
//*********************************************************************************
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data)
{
	myData.iCurrentTime=elapsed;
}


void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data)
{
	/*cd_message ("%s (%s)",__func__,cImageURI);*/
	/*g_free (myData.cCoverPath);
	myData.cCoverPath = g_strdup (cImageURI);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.playing_cover);
	CD_APPLET_REDRAW_MY_ICON;
	myData.cover_exist = TRUE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}*/
}

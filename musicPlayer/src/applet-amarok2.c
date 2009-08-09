
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
#include "applet-cover.h"
#include "applet-draw.h"


//////////////////////////////////////
// Les Fonctions propres a Amarok2. //
//////////////////////////////////////

/* Recupere le temps ecoule
 */
static void _amarok2_getElapsedTime (void)
{
	cd_message ("");
	myData.iCurrentTime = cairo_dock_dbus_get_integer (myData.dbus_proxy_player, myData.DBus_commands.current_position) / 1000;
}

/* Recupere le statut dans un tableau
 */
static void _amarok2_get_status (GValueArray *s)
{
	g_return_if_fail (s != NULL);
	GValue *v = g_value_array_get_nth (s, 0);
	int status = g_value_get_int (v);
	
	cd_debug ("MP : Status : %i", status);
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
			cd_warning ("unknown status from Amarok2");
			break;
	}
}

/* Recupere les infos dans une hash table
 */
static void _amarok2_get_infos (GHashTable *data_list)
{
	g_return_if_fail (data_list != NULL);
	
	myData.pPreviousPlayingStatus = myData.pPlayingStatus;
	myData.iPreviousTrackNumber = myData.iTrackNumber;
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	
	GValue *value;
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
	
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // on memorise la precedente couverture.
	myData.cCoverPath = NULL;
	myData.bCoverNeedsTest = FALSE;
	
	const gchar *cString;
	value = (GValue *) g_hash_table_lookup (data_list, "arturl");
	if (value != NULL && G_VALUE_HOLDS_STRING (value))
		cString = g_value_get_string (value);
	else
		cString = NULL;
	cd_musicplayer_get_cover_path (cString, FALSE);  // FALSE <=> on cherche en cache aussi.
	g_print ("MP :  cCoverPath <- %s\n", myData.cCoverPath);
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
void cd_amarok2_getSongInfos (void)
{	
	GHashTable *data_list = NULL;
	GValue *value;
	
	if (dbus_g_proxy_call (myData.dbus_proxy_player, "GetMetadata", NULL,
		G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
		&data_list,
		G_TYPE_INVALID))
	{
		_amarok2_get_infos (data_list);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("MP : Can't get song properties");
		myData.cCoverPath = NULL;
	}
	
	g_free (myData.cRawTitle);
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
	
	if (cairo_dock_strings_differ (myData.cPreviousCoverPath, myData.cCoverPath))  // la couverture a change, son existence est incertaine. Sinon son existence ne change pas.
		myData.cover_exist = FALSE;
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executée à chaque changement de musique
 */
void onChangeTrack(DBusGProxy *player_proxy,GHashTable *data_list, gpointer data)
{
	//cd_debug("MP : On a change la ZIK");
	_amarok2_get_infos (data_list);
	g_free (myData.cRawTitle);
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}


/* Fonction executée à chaque changement play/pause/stop
 */
void onChangeStatus(DBusGProxy *player_proxy, GValueArray *status, gpointer data)
{
	//cd_debug("MP : On a change de statut");
	//cd_amarok2_getStatus();
	_amarok2_get_status (status);
}

/* Recupere le statut du lecteur (play/pause/stop)
 */
void cd_amarok2_getStatus (void)
{
	GValueArray *s = 0;
	
	dbus_g_proxy_call (myData.dbus_proxy_player, myData.DBus_commands.get_status, NULL,
	G_TYPE_INVALID,
	dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID),
	&s,
	G_TYPE_INVALID);
	
	_amarok2_get_status (s) ;
}


////////////////////////////
// Definition du backend. //
////////////////////////////

/* Fonction de connexion au bus de Amarok2.
 */
gboolean cd_amarok2_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // cree le proxy.
		
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
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par notre controleur
 */
void cd_amarok2_free_data (void)  //Permet de libérer la mémoire prise par notre controleur
{
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal (myData.dbus_proxy_player, "StatusChange",
			G_CALLBACK (onChangeStatus), NULL);
		
		dbus_g_proxy_disconnect_signal (myData.dbus_proxy_player, "TrackChange",
			G_CALLBACK (onChangeTrack), NULL);
	}
	
	musicplayer_dbus_disconnect_from_bus ();
}

/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur)
 */
void cd_amarok2_control (MyPlayerControl pControl, const char *file)
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
		
		case PLAYER_STOP :
			cCommand = myData.DBus_commands.stop;
		break;
		
		/*case PLAYER_ENQUEUE :
			/// A faire...
		break;*/
		
		default :
			return;
		break;
	}
	
	if (cCommand != NULL)
	{
		cd_debug ("MP : Handler amarok2 : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}



/* Fonction de lecture periodique des infos (l'echange de donnees se fait avec les proxys DBus, sauf pour le temps ecoule)
 */
void cd_amarok2_read_data (void)
{
	if (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT)
	{
		_amarok2_getElapsedTime ();
	}
}


/* Initialise le backend de Amarok2.
 */
void cd_amarok2_configure (void)
{
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
	
	myData.dbus_enable = cd_amarok2_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de Amarok2.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de RB sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			cd_amarok2_getStatus ();
			cd_amarok2_getSongInfos ();
			cd_musicplayer_update_icon (FALSE);
		}
		else  // player eteint.
		{
			cd_musicplayer_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		cd_musicplayer_set_surface (PLAYER_BROKEN);
	}
}


/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_amarok2_handeler (void) {
	cd_debug ("");
	MusicPlayerHandeler *pAmarok2 = g_new0 (MusicPlayerHandeler, 1);
	pAmarok2->read_data = cd_amarok2_read_data;
	pAmarok2->free_data = cd_amarok2_free_data;
	pAmarok2->configure = cd_amarok2_configure;  // renseigne les proprietes DBus et se connecte au bus.
	pAmarok2->control = cd_amarok2_control;
	pAmarok2->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP;
	pAmarok2->appclass = "amarok";  /// a verifier...
	pAmarok2->launch = "amarok";  /// a verifier...
	pAmarok2->name = "Amarok 2";
	pAmarok2->iPlayer = MP_AMAROK2;
	pAmarok2->bSeparateAcquisition = FALSE;  // inutile de threader, c'est rapide.
	pAmarok2->iLevel = PLAYER_GOOD;
	
	cd_musicplayer_register_my_handeler (pAmarok2, "Amarok 2");
}

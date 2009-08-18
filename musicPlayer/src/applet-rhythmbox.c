
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
#include "applet-rhythmbox.h"

/////////////////////////////////
// Les Fonctions propres a RB. //
/////////////////////////////////

/* Teste si Rhythmbox joue de la musique ou non
 */
static void _rhythmbox_getPlaying (void)
{
	cd_message ("");
	if (cairo_dock_dbus_get_boolean (myData.dbus_proxy_player,"getPlaying"))
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
}

/* Retourne l'adresse de la musique jouée
 */
static void _rhythmbox_getPlayingUri(void)
{
	cd_message ("");
	g_free (myData.cPlayingUri);
	myData.cPlayingUri = cairo_dock_dbus_get_string (myData.dbus_proxy_player, "getPlayingUri");
}

/* Recupere les infos de la chanson courante, y compris le chemin de la couverture (la telecharge au besoin).
 */
static void cd_rhythmbox_getSongInfos (gboolean bGetAll)
{
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
		
	if(dbus_g_proxy_call (myData.dbus_proxy_shell, "getSongProperties", NULL,
		G_TYPE_STRING, myData.cPlayingUri,
		G_TYPE_INVALID,
		MP_DBUS_TYPE_SONG_METADATA,
		&data_list,
		G_TYPE_INVALID))
	{
		if (bGetAll)
		{
			g_free (myData.cArtist);
			value = (GValue *) g_hash_table_lookup(data_list, "artist");
			if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cArtist = g_strdup (g_value_get_string(value));
			else myData.cArtist = NULL;
			cd_message ("  cArtist <- %s", myData.cArtist);
			
			g_free (myData.cAlbum);
			value = (GValue *) g_hash_table_lookup(data_list, "album");
			if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cAlbum = g_strdup (g_value_get_string(value));
			else myData.cAlbum = NULL;
			cd_message ("  cAlbum <- %s", myData.cAlbum);
			
			g_free (myData.cTitle);
			value = (GValue *) g_hash_table_lookup(data_list, "title");
			if (value != NULL && G_VALUE_HOLDS_STRING(value)) myData.cTitle = g_strdup (g_value_get_string(value));
			else myData.cTitle = NULL;
			cd_message ("  cTitle <- %s", myData.cTitle);
			
			value = (GValue *) g_hash_table_lookup(data_list, "track-number");
			if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iTrackNumber = g_value_get_uint(value);
			else myData.iTrackNumber = 0;
			cd_message ("  iTrackNumber <- %d", myData.iTrackNumber);
			
			value = (GValue *) g_hash_table_lookup(data_list, "duration");
			if (value != NULL && G_VALUE_HOLDS_UINT(value)) myData.iSongLength = g_value_get_uint(value);
			else myData.iSongLength = 0;
			cd_message ("  iSongLength <- %ds", myData.iSongLength);
			myData.bCoverNeedsTest = FALSE;
		}
		
		const gchar *cString = NULL;
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		if (value != NULL && G_VALUE_HOLDS_STRING(value))  // RB nous donne une adresse, eventuellement distante.
			cString = g_value_get_string(value);
		cd_musicplayer_get_cover_path (cString, ! bGetAll);  // la 2eme fois on ne recupere que la couverture et donc on cherche aussi en cache.
		g_print ("MP :  cCoverPath <- %s\n", myData.cCoverPath);
		
		g_hash_table_destroy (data_list);
	}
	else
	{
		cd_warning ("  can't get song properties");
		g_free (myData.cPlayingUri);
		myData.cPlayingUri = NULL;
		g_free (myData.cTitle);
		myData.cTitle = NULL;
		g_free (myData.cAlbum);
		myData.cAlbum = NULL;
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
}


/////////////////////////////////////
// Les callbacks des signaux DBus. //
/////////////////////////////////////

/* Fonction executée à chaque changement de musique.
 */
static void onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	cd_message ("MP : %s (%s)",__func__,uri);
	
	g_free (myData.cPlayingUri);
	if(uri != NULL && *uri != '\0')
	{
		myData.cPlayingUri = g_strdup (uri);
		myData.bIsRunning = TRUE;
		cd_rhythmbox_getSongInfos (TRUE);  // TRUE <=> get all
	}
	else
	{
		myData.cPlayingUri = NULL;
		myData.cover_exist = FALSE;
		
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
		
		cd_musicplayer_dbus_detect_player ();
	}
	cd_musicplayer_update_icon (TRUE);
}

/* Fonction executée à chaque changement play/pause
 */
static void onChangePlaying(DBusGProxy *player_proxy, gboolean playing, gpointer data)
{
	if (playing)
		myData.iPlayingStatus = PLAYER_PLAYING;
	else
		myData.iPlayingStatus = PLAYER_PAUSED;
	if(! myData.cover_exist && myData.cPlayingUri != NULL)
	{
		cd_message ("  cPlayingUri : %s", myData.cPlayingUri);
		if(myData.iPlayingStatus == PLAYER_PLAYING)
		{
			cd_musicplayer_set_surface (PLAYER_PLAYING);
		}
		else
		{
			cd_musicplayer_set_surface (PLAYER_PAUSED);
		}
	}
	else
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
}


/* Fonction executée à chaque changement de temps joué
 */
static void onElapsedChanged (DBusGProxy *player_proxy,int elapsed, gpointer data)
{
	myData.iCurrentTime = elapsed;
	if(elapsed > 0)
	{
		//g_print ("%s () : %ds\n", __func__, elapsed);
		if(myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT)  // avec un '-' devant.
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed - myData.iSongLength);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else if(myConfig.iQuickInfoType == MY_APPLET_PERCENTAGE)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int) (100.*elapsed/myData.iSongLength));
			CD_APPLET_REDRAW_MY_ICON;
		}
	}
}


static void onCoverArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data)  // je n'ai jamais vu ce signal appelle...
{
	g_print ("\n%s (%s)\n\n",__func__,cImageURI);
	g_free (myData.cCoverPath);
	myData.cCoverPath = g_strdup (cImageURI);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCoverPath);
	CD_APPLET_REDRAW_MY_ICON;
	myData.cover_exist = TRUE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
}


////////////////////////////
// Definition du backend. //
////////////////////////////

/* Fonction de connexion au bus de rhythmbox.
 */
gboolean cd_rhythmbox_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // cree le proxy.
		
		myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell ();  // cree le proxy pour la 2eme interface car RB en a 2.
		
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
			G_CALLBACK(onCoverArtChanged), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

/* Permet de libérer la mémoire prise par notre controleur
 */
void cd_rhythmbox_free_data (void) {
	if (myData.dbus_proxy_player != NULL)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "playingChanged",
			G_CALLBACK(onChangePlaying), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(onChangeSong), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(onElapsedChanged), NULL);
		
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(onCoverArtChanged), NULL);
	}
	
	musicplayer_dbus_disconnect_from_bus();
	musicplayer_dbus_disconnect_from_bus_Shell();
}


/* Controle du lecteur (permet d'effectuer les actions de bases sur le lecteur)
 */
void cd_rhythmbox_control (MyPlayerControl pControl, const char* song)
{
	cd_debug ("");
	gchar *cCommand = NULL;
		
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = myData.DBus_commands.previous;  // ou bien rhythmbox-client --previous
		break;
		
		case PLAYER_PLAY_PAUSE :
			cCommand = myData.DBus_commands.play;  // ou bien rhythmbox-client --pause/--play
		break;

		case PLAYER_NEXT :
			cCommand = myData.DBus_commands.next;  // ou bien rhythmbox-client --next
		break;
		
		case PLAYER_ENQUEUE :
			cCommand = g_strdup_printf ("rhythmbox-client --enqueue %s", song);
			g_spawn_command_line_async (cCommand, NULL);
			g_free (cCommand);
			cCommand = NULL;
		break;
		
		default :
			return;
		break;
	}
	
	if (pControl == PLAYER_PLAY_PAUSE) // Cas special pour RB qui necessite un argument pour le PlayPause
	{
		gboolean bStartPlaying = (myData.iPlayingStatus != PLAYER_PLAYING);
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_player, cCommand, 
			G_TYPE_BOOLEAN, bStartPlaying,
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	else if (cCommand != NULL) 
	{
		cd_debug ("MP : Handler rhythmbox : will use '%s'", cCommand);
		cairo_dock_dbus_call (myData.dbus_proxy_player, cCommand);
	}
}

void cd_rhythmbox_get_cover_path (void)
{
	cd_rhythmbox_getSongInfos (FALSE);  // FALSE <=> on ne recupere que la couverture.
}

/* Initialise le backend de RB.
 */
void cd_rhythmbox_configure (void)
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
	
	myData.dbus_enable = cd_rhythmbox_dbus_connect_to_bus ();  // se connecte au bus et aux signaux de RB.
	if (myData.dbus_enable)
	{
		cd_musicplayer_dbus_detect_player ();  // on teste la presence de RB sur le bus <=> s'il est ouvert ou pas.
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			g_print ("MP : RB is running");
			_rhythmbox_getPlaying();
			_rhythmbox_getPlayingUri();
			cd_rhythmbox_getSongInfos (TRUE);  // TRUE <=> get all
			cd_musicplayer_update_icon (TRUE);
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
void cd_musicplayer_register_rhythmbox_handler (void)
{
	cd_debug ("");
	MusicPlayerHandeler *pRhythmbox = g_new0 (MusicPlayerHandeler, 1);
	pRhythmbox->read_data = NULL;  // rien a faire vu que l'echange de donnees se fait entierement avec les proxys DBus.
	pRhythmbox->free_data = cd_rhythmbox_free_data;
	pRhythmbox->configure = cd_rhythmbox_configure;  // renseigne les proprietes DBus et se connecte au bus.
	pRhythmbox->control = cd_rhythmbox_control;
	pRhythmbox->get_cover = cd_rhythmbox_get_cover_path;
	
	pRhythmbox->appclass = "rhythmbox";
	pRhythmbox->name = "Rhythmbox";
	pRhythmbox->launch = "rhythmbox";
	pRhythmbox->cCoverDir = g_strdup_printf ("%s/.cache/rhythmbox/covers", g_getenv ("HOME"));
	pRhythmbox->iPlayer = MP_RHYTHMBOX;
	pRhythmbox->bSeparateAcquisition = FALSE;
	pRhythmbox->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_ENQUEUE;
	pRhythmbox->iLevel = PLAYER_EXCELLENT;
	
	cd_musicplayer_register_my_handler(pRhythmbox, "rhythmbox");
}

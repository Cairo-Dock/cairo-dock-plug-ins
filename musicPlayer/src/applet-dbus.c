/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-dbus.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"

CD_APPLET_INCLUDE_MY_VARS

gboolean cd_musicplayer_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		dbus_proxy_player = cairo_dock_create_new_session_proxy (
			myData.DBus_commands.service,
			myData.DBus_commands.path,
			myData.DBus_commands.interface);
		return TRUE;
	}
	return FALSE;
}

gboolean musicplayer_dbus_connect_to_bus_Shell (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		dbus_proxy_shell = cairo_dock_create_new_session_proxy (
			myData.DBus_commands.service,
			myData.DBus_commands.path2,
			myData.DBus_commands.interface2);
		return TRUE;
	}
	return FALSE;
}

void musicplayer_dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_player != NULL)
	{
		g_object_unref (dbus_proxy_player);
		dbus_proxy_player = NULL;
	}
}

void musicplayer_dbus_disconnect_from_bus_Shell (void)
{
	cd_message ("");
	if (dbus_proxy_shell != NULL)
	{
		g_object_unref (dbus_proxy_shell);
		dbus_proxy_shell = NULL;
	}
}

void cd_musicplayer_dbus_detection(void)
{
	//cd_debug("MP : Valeur de service : %s",myData.DBus_commands.service);
	myData.opening = cairo_dock_dbus_detect_application (myData.DBus_commands.service);
}


void cd_musicplayer_check_dbus_connection (void)
{
	cd_debug("MP : on initialise DBus");
	myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus ();
	if (myData.dbus_enable) // Si le bus est accessible
	{
		cd_musicplayer_dbus_detection(); //On vérifie si le lecteur est ouvert
		if (myData.opening) // Si il est ouvert, on se connecte au bus
		{
			cd_debug("MP : le lecteur est ouvert");
		}
		else // Si le lecteur n'est pas ouvert
		{
			//cd_musicplayer_set_surface (PLAYER_NONE);
			cd_debug("MP : le lecteur n'est pas ouvert");
			//return TRUE;
		}
	}
	
	else // Si le bus n'est pas accessible
	{
		//cd_musicplayer_set_surface (PLAYER_BROKEN);
		//return TRUE;
	}
	
	//return TRUE;
}

//*********************************************************************************
// musicplayer_dbus_command() : Envoie une commande à musicplayer
//*********************************************************************************
void cd_musicplayer_dbus_command(const char *command)
{
	cd_message("MP : On demande %s", command);
	cairo_dock_dbus_call(dbus_proxy_player, command);
}

//*********************************************************************************
// musicplayer_getValue() : Retourne une chaine de caractère selon une méthode
//*********************************************************************************

gchar* cd_musicplayer_dbus_getValue (const char *method)
{
	gchar *value;
	value = cairo_dock_dbus_get_string (dbus_proxy_player, method);
	return value;
	
}


//*********************************************************************************
// musicplayer_getStatus_*() : Test si musicplayer joue de la musique ou non
//*********************************************************************************
void cd_musicplayer_getStatus_string (void)
{
		gchar *status;
		status = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_status);
		if (! g_ascii_strcasecmp(status, "playing"))
		{
			myData.pPlayingStatus = PLAYER_PLAYING;
			cd_message ("MP : En cours de lecture");	
		}
		
		else if (! g_ascii_strcasecmp(status, "paused"))
		{
			myData.pPlayingStatus = PLAYER_PAUSED;
			cd_message ("MP : Pause");	
		}
		
		else if (! g_ascii_strcasecmp(status, "stopped"))
		{
			myData.pPlayingStatus = PLAYER_STOPPED;
			cd_message ("MP : Stop");	
		}
		
		g_free(status);
	}

/*
void musicplayer_getStatus_integer (void)
{
	GError *error = 0;
	int status;

	myData.playing=FALSE;
	myData.paused=FALSE;
	myData.stopped=FALSE;
	
	dbus_g_proxy_call (dbus_proxy_player, myData.DBus_commands.get_status, &error,
			G_TYPE_INVALID,
			G_TYPE_INT, &status,
			G_TYPE_INVALID); // A rajouter dans cairo-dock-dbus.c --> cairo_dock_dbus_get_integer()
	
	myData.status=status;
	
	if (status == 0) myData.paused=TRUE;
	else if (status == 1) myData.playing=TRUE;
	else 
	{
		myData.status=-1;
		myData.stopped=TRUE;
	}		
	//myData.status_uinteger = cairo_dock_dbus_get_uinteger (dbus_proxy_player, "GetPlayingStatus");
}
*/


//*********************************************************************************
// musicplayer_getSongInfos() : Retourne les infos de la musique jouée
//*********************************************************************************

void cd_musicplayer_getSongInfos(void)
{
	/*GError *erreur = NULL;
	guchar* bValue = NULL;
	dbus_g_proxy_call (dbus_proxy_player, "current_position", &erreur,
		G_TYPE_INVALID,
		G_TYPE_UCHAR, &bValue,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("musicplayer : Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	//return bValue;*/
	
	//cd_message ("musicplayer : On récupère les infos de la musique jouée");
	
	g_free (myData.cTitle);
	myData.cTitle = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_title);
	myData.cRawTitle = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_title);
	
	g_free (myData.cAlbum);
	myData.cAlbum = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_album);

	g_free (myData.cArtist);
	myData.cArtist = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_artist);

	//g_free (myData.total_length);
	//myData.total_length = cairo_dock_dbus_get_string (dbus_proxy_player, "get_length");
	
	//g_free (myData.percentage);
	//myData.percentage = cairo_dock_dbus_get_uinteger (dbus_proxy_player, "current_position");
		
	cd_message("MP : %s - %s - %s", myData.cTitle, myData.cArtist, myData.cAlbum);
	//cd_message("musicplayer : %i", (int)bValue);

}




//*********************************************************************************
// musicplayer_getCoverPath() : Retourne l'adresse de la pochette
//*********************************************************************************

void cd_musicplayer_getCoverPath (void)
{
	g_free (myData.cCoverPath);
	myData.cCoverPath = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_cover_path);
	if (myData.cCoverPath != NULL)
	{
		//myData.cover_exist=1;
	}
	cd_message("MP : Couverture -> %s", myData.cCoverPath);
}

//*********************************************************************************
// musicplayer_check_for_changes() : Vérifie s'il y a eu des changements de données
//*********************************************************************************

gboolean cd_musicplayer_check_for_changes (void)
{
	gchar *title, *album, *artist, *cover_path;
	gboolean has_changed;
	

	title = cd_musicplayer_dbus_getValue(myData.DBus_commands.get_title);
	artist = cd_musicplayer_dbus_getValue(myData.DBus_commands.get_artist);
	album = cd_musicplayer_dbus_getValue(myData.DBus_commands.get_album);
	cover_path = cd_musicplayer_dbus_getValue(myData.DBus_commands.get_cover_path);
	
	if (! g_ascii_strcasecmp(myData.cTitle, title)) //Si les chaines sont identiques
	{
		if (! g_ascii_strcasecmp(myData.cArtist, artist))
		{
			if (! g_ascii_strcasecmp(myData.cAlbum, album))
			{
				if (! g_ascii_strcasecmp(myData.cCoverPath, cover_path))
					{
						myData.cPreviousRawTitle=myData.cRawTitle;
						has_changed=FALSE; // Tout est identique
					}
			}
		}
	}
	else
	{	
		cd_message("MP : La chanson a change");
		myData.cPreviousRawTitle=myData.cRawTitle;
		has_changed=TRUE;
	}
	
	g_free(title);
	g_free(artist);
	g_free(album);
	g_free(cover_path);
	return has_changed;
}





void cd_musicplayer_load_dbus_commands (void)
{
	cd_debug("MP : valeur dans dbus : %s", myConfig.cMusicPlayer);
	if (! strcmp(myConfig.cMusicPlayer,"Exaile"))
	{
		cd_debug("MP : On charge les commande pour Exaile");
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
		myData.DBus_commands.toggle = "toggle_visibility";
		return TRUE;
	}
		
	/*case MY_BANSHEE :
		myData.DBus_commands.service = "org.gnome.Banshee";
		myData.DBus_commands.path = "/org/gnome/Banshee/Player";
		myData.DBus_commands.interface = "org.gnome.Banshee.Core";
		myData.DBus_commands.play = "Play";
		myData.DBus_commands.pause = "Pause";
		//myData.DBus_commands.stop = "stop";
		myData.DBus_commands.next = "Next";
		myData.DBus_commands.previous = "Previous";
		myData.DBus_commands.get_title = "GetPlayingTitle";
		myData.DBus_commands.get_artist = "GetPlayingArtist";
		myData.DBus_commands.get_album = "GetPlayingAlbum";
		myData.DBus_commands.get_cover_path = "GetPlayingCoverUri";
		myData.DBus_commands.get_status = "GetPlayingStatus";
		myData.DBus_commands.toggle = "PresentWindow";
		break;
		
	case MY_QUOD_LIBET :
		myData.DBus_commands.service = "net.sacredchao.QuodLibet";
		myData.DBus_commands.path = "/net/sacredchao/QuodLibet";
		myData.DBus_commands.interface = "net.sacredchao.QuodLibet";
		myData.DBus_commands.play = "Play";
		myData.DBus_commands.pause = "Pause";
		//myData.DBus_commands.stop = "stop";
		/*myData.DBus_commands.next = "Next";
		myData.DBus_commands.previous = "Previous";
		myData.DBus_commands.get_title = "GetPlayingTitle";
		myData.DBus_commands.get_artist = "GetPlayingArtist";
		myData.DBus_commands.get_album = "GetPlayingAlbum";
		myData.DBus_commands.get_cover_path = "GetPlayingCoverUri";
		myData.DBus_commands.get_status = "GetPlayingStatus";
		myData.DBus_commands.toggle = "PresentWindow";
		break;
		
	case MY_LISTEN :
		myData.DBus_commands.service = "org.gnome.Listen";
		myData.DBus_commands.path = "/org/gnome/listen";
		myData.DBus_commands.interface = "org.gnome.Listen";
		myData.DBus_commands.play = "play_pause";
		myData.DBus_commands.pause = "play_pause";
		//myData.DBus_commands.stop = "stop";
		myData.DBus_commands.next = "next";
		myData.DBus_commands.previous = "previous";
		myData.DBus_commands.get_full_data = "current_playing";
		myData.DBus_commands.toggle = "hello";
		break;
		
	case MY_RHYTHMBOX :
		myData.DBus_commands.service = "org.gnome.Rhythmbox";
		myData.DBus_commands.path = "/org/gnome/Rhythmbox/Player";
		myData.DBus_commands.interface = "org.gnome.Rhythmbox.Player";
		myData.DBus_commands.path2 = "/org/gnome/Rhythmbox/Shell";
		myData.DBus_commands.interface2 = "org.gnome.Rhythmbox.Shell";
		myData.DBus_commands.play = "play_pause";
		myData.DBus_commands.pause = "play_pause";
		//myData.DBus_commands.stop = "stop";
		myData.DBus_commands.next = "next";
		myData.DBus_commands.previous = "previous";
		myData.DBus_commands.get_full_data = "current_playing";
		myData.DBus_commands.toggle = "hello";
		break;
		*/
	
	return FALSE;
	
}

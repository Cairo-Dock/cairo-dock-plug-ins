/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"
#include "string.h"
#include <dbus/dbus-glib.h>
#include <glib/gi18n.h>

#include "musicplayer-struct.h"
#include "musicplayer-init.h"
#include "musicplayer-dbus.h"
#include "musicplayer-draw.h"

CD_APPLET_INCLUDE_MY_VARS


gboolean musicplayer_dbus_connect_to_bus (void)
{
	cd_message("musicplayer : Attention On se connecte");
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
	cd_message("musicplayer : Attention On se connecte à l'interface Shell");
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

void dbus_detect_musicplayer(void)
{
	myData.opening = cairo_dock_dbus_detect_application (myData.DBus_commands.service);
}


gboolean check_dbus_connection (void)
{
	
	load_dbus_command_for_player();
	myData.dbus_enable = musicplayer_dbus_connect_to_bus ();
	if (myData.dbus_enable) // Si le bus est accessible
	{
		dbus_detect_musicplayer(); //On vérifie si le lecteur est ouvert
		if (myData.opening) // Si il est ouvert, on se connecte au bus
		{
		cd_message("musicplayer : Valeur de opening --> %d, Valeur de enable --> %d", myData.opening, myData.dbus_enable);
		return TRUE;
		}
		else if (!myData.opening) // Si le lecteur n'est pas ouvert n'est pas ouvert
		{
			musicplayer_set_surface (PLAYER_NONE);
			cd_message ("musicplayer : le lecteur n'est pas ouvert");
			return TRUE;
		}
	}
	
	else if (!myData.dbus_enable) // Si le bus n'est pas accessible
	{
		musicplayer_set_surface (PLAYER_BROKEN);
		cd_message("musicplayer : Bus non accessible");
		return TRUE;
	}
	
	return TRUE;
}

//*********************************************************************************
// musicplayer_dbus_command() : Envoie une commande à musicplayer
//*********************************************************************************
void musicplayer_dbus_command(const char *command)
{
	cd_message("musicplayer : On demande %s", command);
	cairo_dock_dbus_call(dbus_proxy_player, command);
}

//*********************************************************************************
// musicplayer_getValue() : Retourne une chaine de caractère selon une méthode
//*********************************************************************************

gchar* musicplayer_dbus_getValue (const char *method)
{
	gchar *value;
	value = cairo_dock_dbus_get_string (dbus_proxy_player, method);
	return value;
	
}


//*********************************************************************************
// musicplayer_getStatus_*() : Test si musicplayer joue de la musique ou non
//*********************************************************************************
void musicplayer_getStatus_string (void)
{
		gchar *status;
		myData.playing=FALSE;
		myData.paused=FALSE;
		myData.stopped=FALSE;
		status = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_status);
		cd_message("musicplayer : valeur de status : %s", status);
		if (! g_ascii_strcasecmp(status, "playing"))
		{
			myData.status=1;
			cd_message ("musicplayer : En cours de lecture");	
			myData.playing=TRUE;
		}
		
		else if (! g_ascii_strcasecmp(status, "paused"))
		{
			myData.status=0;
			cd_message ("musicplayer : Pause");	
			myData.paused=TRUE;
		}
		
		else if (! g_ascii_strcasecmp(status, "stopped"))
		{
			myData.status=0;
			cd_message ("musicplayer : Stop");	
			myData.stopped=TRUE;
		}
		
		g_free(status);
}


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
	cd_message("musicplayer : valeur de status --> %i", status);
}



//*********************************************************************************
// musicplayer_getSongInfos() : Retourne les infos de la musique jouée
//*********************************************************************************

void musicplayer_getSongInfos(void)
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
	
	cd_message ("musicplayer : On récupère les infos de la musique jouée");
	
	g_free (myData.playing_title);
	myData.playing_title = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_title);

	g_free (myData.playing_album);
	myData.playing_album = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_album);

	g_free (myData.playing_artist);
	myData.playing_artist = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_artist);

	//g_free (myData.total_length);
	//myData.total_length = cairo_dock_dbus_get_string (dbus_proxy_player, "get_length");
	
	//g_free (myData.percentage);
	//myData.percentage = cairo_dock_dbus_get_uinteger (dbus_proxy_player, "current_position");
		
	cd_message("musicplayer : %s - %s - %s", myData.playing_title, myData.playing_artist, myData.playing_album);
	//cd_message("musicplayer : %i", (int)bValue);

}




//*********************************************************************************
// musicplayer_getCoverPath() : Retourne l'adresse de la pochette
//*********************************************************************************

void musicplayer_getCoverPath (void)
{
	cd_message ("musicplayer : On récupère les infos sur la couverture");
	g_free (myData.playing_cover);
	myData.playing_cover = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_cover_path);
	if (myData.playing_cover != NULL)
	{
		myData.cover_exist=1;
	}
	cd_message("musicplayer : Couverture -> %s", myData.playing_cover);
}

//*********************************************************************************
// musicplayer_check_for_changes() : Vérifie s'il y a eu des changements de données
//*********************************************************************************

void musicplayer_check_for_changes (void)
{
	gchar *title, *album, *artist, *cover_path;
	

	title = musicplayer_dbus_getValue(myData.DBus_commands.get_title);
	artist = musicplayer_dbus_getValue(myData.DBus_commands.get_artist);
	album = musicplayer_dbus_getValue(myData.DBus_commands.get_album);
	cover_path = musicplayer_dbus_getValue(myData.DBus_commands.get_cover_path);
	
	if (! g_ascii_strcasecmp(myData.playing_title, title)) //Si les chaines sont identiques
	{
		if (! g_ascii_strcasecmp(myData.playing_artist, artist))
		{
			if (! g_ascii_strcasecmp(myData.playing_album, album))
			{
				if (! g_ascii_strcasecmp(myData.playing_cover, cover_path))
					{
						cd_message("musicplayer : ca n'a pas changé");
						myData.data_have_changed = FALSE;
					}
			}
		}
	}
	else
	{	
		cd_message("musicplayer : La chanson a change");
		myData.data_have_changed = TRUE;
	}
	
	g_free(title);
	g_free(artist);
	g_free(album);
	g_free(cover_path);
}






void load_dbus_command_for_player (void)
{
	switch (myConfig.iPlayer) 
	{
		
	case MY_EXAILE :
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
		break;
		
	case MY_BANSHEE :
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
		break;*/
		
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
		
	
	default :
	
		break;
		
	 }
}










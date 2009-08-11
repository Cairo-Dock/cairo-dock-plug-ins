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


gboolean cd_musicplayer_dbus_connect_to_bus (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_proxy_player = cairo_dock_create_new_session_proxy (
			myData.DBus_commands.service,
			myData.DBus_commands.path,
			myData.DBus_commands.interface);
		return (myData.dbus_proxy_player != NULL);
	}
	return FALSE;
}

gboolean musicplayer_dbus_connect_to_bus_Shell (void)
{
	if (cairo_dock_bdus_is_enabled ())
	{
		myData.dbus_proxy_shell = cairo_dock_create_new_session_proxy (
			myData.DBus_commands.service,
			myData.DBus_commands.path2,
			myData.DBus_commands.interface2);
		return (myData.dbus_proxy_shell != NULL);
	}
	return FALSE;
}

void musicplayer_dbus_disconnect_from_bus (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		g_object_unref (myData.dbus_proxy_player);
		myData.dbus_proxy_player = NULL;
	}
}

void musicplayer_dbus_disconnect_from_bus_Shell (void)
{
	if (myData.dbus_proxy_shell != NULL)
	{
		g_object_unref (myData.dbus_proxy_shell);
		myData.dbus_proxy_shell = NULL;
	}
}

void cd_musicplayer_dbus_detect_player (void)
{
	myData.bIsRunning = cairo_dock_dbus_detect_application (myData.DBus_commands.service);
}

/// deprecated...
gboolean cd_musicplayer_dbus_detection(void)
{
	gboolean isConnectedToBus = cairo_dock_dbus_detect_application (myData.DBus_commands.service);
	gboolean isProxyCorrect;
	
	if (myData.dbus_enable)
		isProxyCorrect = isConnectedToBus?DBUS_IS_G_PROXY(myData.dbus_proxy_player):FALSE;	
	else 
		isProxyCorrect = TRUE; //C'est la premiere connexion, le proxy n'est pas encore actif

	
	if( !isProxyCorrect ) 
		myData.dbus_proxy_player = NULL;
	
	return isConnectedToBus && isProxyCorrect;
}


//*********************************************************************************
// cd_musicplayer_check_dbus_connection() : Verifie l'etat de la connexion DBus
//*********************************************************************************
void cd_musicplayer_check_dbus_connection (void)
{
	cd_debug("MP : Vérification de la connexion DBus");
	if (myData.DBus_commands.service != NULL)
	{
		myData.bIsRunning = cd_musicplayer_dbus_detection();
		if ((myData.bIsRunning) && (!myData.dbus_enable)) // On vérifie si notre lecteur est ouvert et si on n'est pas déjà connecté au bus
		{
			cd_message("MP : On se connecte au bus pour la première fois");
			myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // Alors on se connecte au bus
			if (myData.dbus_enable)
				cd_message("MP : Connexion au bus effectuee");
		}
		else if ((myData.dbus_enable) && (myData.bIsRunning)) // Sinon on est deja connecte au bus, on lit juste les donnees
			;//cd_debug("MP : On est déjà connecté au bus, on va juste lire les donnees");
		else // Sinon le lecteur n'est pas ouvert
		{
			myData.dbus_enable = 0;
			//cd_debug("MP : lecteur non ouvert");	
		}
	}
}


//*********************************************************************************
// cd_musicplayer_check_dbus_connection() : Verifie l'etat de la connexion DBus sur le Player et sur le Shell
//*********************************************************************************
void cd_musicplayer_check_dbus_connection_with_two_interfaces (void)
{
	cd_debug("MP : Vérification de la connexion DBus");
	if (myData.DBus_commands.service != NULL)
	{
		myData.bIsRunning = cd_musicplayer_dbus_detection();
		if ((myData.bIsRunning) && (!myData.dbus_enable) && (!myData.dbus_enable_shell)) // On vérifie si notre lecteur est ouvert et si on n'est pas déjà connecté au bus
		{
			cd_message("MP : On se connecte au bus pour la première fois");
			myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // Alors on se connecte au bus
			myData.dbus_enable_shell = musicplayer_dbus_connect_to_bus_Shell ();
			if ((myData.dbus_enable) && (myData.dbus_enable_shell))
				cd_message("MP : Connexions aux bus effectuees");
		}
		else if ((myData.dbus_enable) && (myData.bIsRunning) && (myData.dbus_enable_shell)) // Sinon on est deja connecte au bus, on lit juste les donnees
			;//cd_debug("MP : On est déjà connecté au bus, on va juste lire les donnees");
		else // Sinon le lecteur n'est pas ouvert
		{
			myData.dbus_enable = 0;
			//cd_debug("MP : lecteur non ouvert");	
		}
	}
}



//*********************************************************************************
// musicplayer_getStatus_*() : Test si musicplayer joue de la musique ou non
//*********************************************************************************
void cd_musicplayer_getStatus_string (const char *status_paused, const char *status_playing, const char* status_stopped )
{
		gchar *status=NULL;
		status = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_status);
		myData.pPreviousPlayingStatus = myData.iPlayingStatus;
		if ((! g_ascii_strcasecmp(status, status_playing)) || (!g_ascii_strcasecmp(status, "1")))
		{
			//cd_debug("MP : le lecteur est en statut PLAY");
			myData.iPlayingStatus = PLAYER_PLAYING;
		}
		else if (! g_ascii_strcasecmp(status, status_paused))
		{
			//cd_debug("MP : le lecteur est en statut PAUSED");
			myData.iPlayingStatus = PLAYER_PAUSED;
		}
		else if ((status_stopped) &&(! g_ascii_strcasecmp(status, status_stopped)))
		{
			//cd_debug("MP : le lecteur est en statut STOPPED");
			myData.iPlayingStatus = PLAYER_STOPPED;
		}
		if (status != NULL)
		{
			g_free(status);
			status=NULL;
		}
}


void cd_musicplayer_getStatus_integer (int status_paused, int status_playing)
{
	int status;
	
	status=cairo_dock_dbus_get_integer(myData.dbus_proxy_player, myData.DBus_commands.get_status);
	//cd_debug("MP : Statut du lecteur : %d",status);
	if (status == status_paused) myData.iPlayingStatus = PLAYER_PAUSED;
	else if (status == status_playing) myData.iPlayingStatus = PLAYER_PLAYING;
	else myData.iPlayingStatus = PLAYER_STOPPED;
}



//*********************************************************************************
// musicplayer_getCoverPath() : Retourne l'adresse de la pochette
//*********************************************************************************

void cd_musicplayer_getCoverPath (void)
{
	if (myData.cCoverPath != NULL) {
		g_free (myData.cCoverPath);
		myData.cCoverPath = NULL;
	}
	
	myData.cCoverPath = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_cover_path);
	if (myData.cCoverPath != NULL)
		cd_message("MP : Couverture -> %s", myData.cCoverPath);
	else
		cd_message("MP : Pas de couverture dispo");
}

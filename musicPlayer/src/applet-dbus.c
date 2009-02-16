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
	if (cairo_dock_bdus_is_enabled () && myData.opening)
	{
		myData.dbus_proxy_player = cairo_dock_create_new_session_proxy (
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
		myData.dbus_proxy_shell = cairo_dock_create_new_session_proxy (
			myData.DBus_commands.service,
			myData.DBus_commands.path2,
			myData.DBus_commands.interface2);
		return TRUE;
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

gboolean cd_musicplayer_dbus_detection(void)
{
	return cairo_dock_dbus_detect_application (myData.DBus_commands.service);
}


//*********************************************************************************
// cd_musicplayer_check_dbus_connection() : Verifie l'etat de la connexion DBus
//*********************************************************************************
void cd_musicplayer_check_dbus_connection (void)
{
	cd_debug("MP : Vérification de la connexion DBus");
	myData.opening = cd_musicplayer_dbus_detection();
	if ((myData.opening) && (!myData.dbus_enable)) // On vérifie si notre lecteur est ouvert et si on n'est pas déjà connecté au bus
	{
		cd_debug("MP : On se connecte au bus pour la première fois");
		myData.dbus_enable = cd_musicplayer_dbus_connect_to_bus (); // Alors on se connecte au bus
	}
	else if ((myData.dbus_enable) && (myData.opening)) // Sinon on est deja connecte au bus, on lit juste les donnees
		cd_debug("MP : On est déjà connecté au bus, on va juste lire les donnees");
	else // Sinon le lecteur n'est pas ouvert
	{
		myData.dbus_enable = 0;
		cd_debug("MP : lecteur non ouvert");	
	}
}

//*********************************************************************************
// musicplayer_dbus_command() : Envoie une commande à musicplayer
//*********************************************************************************
void cd_musicplayer_dbus_command(const char *command)
{
	cd_message("MP : On demande %s", command);
	cairo_dock_dbus_call(myData.dbus_proxy_player, command);
}


//*********************************************************************************
// musicplayer_dbus_command_Shell() : Envoie une commande à musicplayer
//*********************************************************************************
void cd_musicplayer_dbus_command_Shell(const char *command)
{
	cd_message("MP : On demande %s", command);
	cairo_dock_dbus_call(myData.dbus_proxy_shell, command);
}

//*********************************************************************************
// musicplayer_getValue() : Retourne une chaine de caractère selon une méthode
//*********************************************************************************
gchar* cd_musicplayer_dbus_getValue (const char *method)
{
	gchar *value=NULL;
	value = cairo_dock_dbus_get_string (myData.dbus_proxy_player, method);
	return value;
	
}


//*********************************************************************************
// musicplayer_getStatus_*() : Test si musicplayer joue de la musique ou non
//*********************************************************************************
void cd_musicplayer_getStatus_string (void)
{
		gchar *status=NULL;
		status = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.get_status);
		//cd_debug ("MP : retour de DBUS sur status : %s", status);
		myData.pPreviousPlayingStatus = myData.pPlayingStatus;
		if ((! g_ascii_strcasecmp(status, "playing")) || (!g_ascii_strcasecmp(status, "1")))
		{
			//cd_debug("MP : le lecteur est en statut PLAYING");
			myData.pPlayingStatus = PLAYER_PLAYING;
		}
		else if (! g_ascii_strcasecmp(status, "paused"))
		{
			//cd_debug("MP : le lecteur est en statut PAUSED");
			myData.pPlayingStatus = PLAYER_PAUSED;
		}
		else if (! g_ascii_strcasecmp(status, "stopped"))
		{
			//cd_debug("MP : le lecteur est en statut STOPPED");
			myData.pPlayingStatus = PLAYER_STOPPED;
		}
		
		g_free(status);
	}


void cd_musicplayer_getStatus_integer (void)
{
	int status;
	
	status=cairo_dock_dbus_get_integer(myData.dbus_proxy_player, myData.DBus_commands.get_status);
	//cd_debug("MP : Statut du lecteur : %d",status);
	if (status == 0) myData.pPlayingStatus = PLAYER_PAUSED;
	else if (status == 1) myData.pPlayingStatus = PLAYER_PLAYING;
	else myData.pPlayingStatus = PLAYER_STOPPED;
}



int cd_musicplayer_getCurPos_integer (void) 
{
	int CurPos = NULL;
	CurPos = cairo_dock_dbus_get_integer (myData.dbus_proxy_player, myData.DBus_commands.current_position);
	return CurPos;
}

guchar* cd_musicplayer_getCurPos_string (void) 
{
	guchar* CurPos = NULL;
	CurPos = cairo_dock_dbus_get_uchar (myData.dbus_proxy_player, myData.DBus_commands.current_position);
	return CurPos;
}

int cd_musicplayer_getLength_integer (void)
{
	int duration=0;
	duration = cairo_dock_dbus_get_integer (myData.dbus_proxy_player, myData.DBus_commands.duration);	
	return duration;
}

gchar* cd_musicplayer_getLength_string (void)
{
	gchar* duration=NULL;
	if ((duration = cairo_dock_dbus_get_string (myData.dbus_proxy_player, myData.DBus_commands.duration)) != NULL)	return duration;
	else return "0:00";
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


gchar* cd_musicplayer_getString_player (const char* Method)
{
	gchar* value;
	value=cairo_dock_dbus_get_string (myData.dbus_proxy_player, Method);
	return value;
}





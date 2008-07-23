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
	}
	// Les autres cas sont gérés dans chaque lecteur
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
	gchar *value=NULL;
	value = cairo_dock_dbus_get_string (dbus_proxy_player, method);
	return value;
	
}


//*********************************************************************************
// musicplayer_getStatus_*() : Test si musicplayer joue de la musique ou non
//*********************************************************************************
void cd_musicplayer_getStatus_string (void)
{
		gchar *status=NULL;
		status = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_status);
		myData.pPreviousPlayingStatus = myData.pPlayingStatus;
		if (! g_ascii_strcasecmp(status, "playing"))
		{
			myData.pPlayingStatus = PLAYER_PLAYING;
		}
		else if (! g_ascii_strcasecmp(status, "paused"))
		{
			myData.pPlayingStatus = PLAYER_PAUSED;
		}
		else if (! g_ascii_strcasecmp(status, "stopped"))
		{
			myData.pPlayingStatus = PLAYER_STOPPED;
		}
		
		g_free(status);
	}


void cd_musicplayer_getStatus_integer (void)
{
	GError *error = 0;
	int status;

	dbus_g_proxy_call (dbus_proxy_player, myData.DBus_commands.get_status, &error,
			G_TYPE_INVALID,
			G_TYPE_INT, &status,
			G_TYPE_INVALID); // A rajouter dans cairo-dock-dbus.c --> cairo_dock_dbus_get_integer()
	
	if (status == 0) myData.pPlayingStatus = PLAYER_PAUSED;
	else if (status == 1) myData.pPlayingStatus = PLAYER_PLAYING;
	else myData.pPlayingStatus = PLAYER_STOPPED;
	
	//cd_message("MP : Status (integer) --> %i", myData.pPlayingStatus);
	g_free(error);
}



//*********************************************************************************
// musicplayer_getSongInfos() : Retourne les infos de la musique jouée
//*********************************************************************************

void cd_musicplayer_getSongInfos(void)
{
	if (myData.cRawTitle != NULL) 
		myData.cPreviousRawTitle = myData.cRawTitle; 
	
	myData.cAlbum = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_album);

	myData.cArtist = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_artist);

	//Artist & Title = RawTitle
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_title));
	
	cd_message("MP : %s - %s - %s", myData.cRawTitle, myData.cArtist, myData.cAlbum);
}


int cd_musicplayer_getCurPos_integer (void) 
{
	int CurPos = NULL;
	CurPos = cairo_dock_dbus_get_integer (dbus_proxy_player, myData.DBus_commands.current_position);
	return CurPos;
}

guchar* cd_musicplayer_getCurPos_string (void) 
{
	guchar* CurPos = NULL;
	CurPos = cairo_dock_dbus_get_uchar (dbus_proxy_player, myData.DBus_commands.current_position);
	return CurPos;
}

int cd_musicplayer_getLength_integer (void)
{
	int duration=0;
	duration = cairo_dock_dbus_get_integer (dbus_proxy_player, myData.DBus_commands.duration);	
	return duration;
}

gchar* cd_musicplayer_getLength_string (void)
{
	gchar* duration=NULL;
	duration = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.duration);
	return duration;
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
	
	myData.cCoverPath = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_cover_path);
	if (myData.cCoverPath != NULL)
	{
		//myData.cover_exist=1;
	}
	cd_message("MP : Couverture -> %s", myData.cCoverPath);
}


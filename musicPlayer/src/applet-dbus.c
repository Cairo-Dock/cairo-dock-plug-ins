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


MusicPlayerHandeler *cd_musicplayer_dbus_find_opened_player (void)
{
	// on recupere la liste des services existants.
	gchar **name_list = cairo_dock_dbus_get_services ();
	if (name_list)
	{
		// on teste chaque lecteur.
		MusicPlayerHandeler *pHandler = NULL;
		int i;
		GList *h;
		for (h = myData.pHandelers; h != NULL; h = h->next)
		{
			pHandler = h->data;
			if (pHandler->cMprisService == NULL)
				continue;
			for (i = 0; name_list[i] != NULL; i ++)
			{
				if (strcmp (name_list[i], pHandler->cMprisService) == 0)  // trouve, on quitte.
				{
					g_print ("found %s\n", pHandler->name);
					break;
				}
			}
			if (name_list[i] != NULL)
				break ;
		}
		g_strfreev (name_list);
		return (h ? pHandler : NULL);
	}
	else
	{
		return NULL;
	}
}


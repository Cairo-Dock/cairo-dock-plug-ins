/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-dbus.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"


gboolean cd_musicplayer_dbus_connect_handler (MusicPlayerHandler *pHandler)
{
	g_return_val_if_fail (pHandler != NULL, FALSE);
	if (pHandler->cMprisService == NULL) return TRUE; // will be handled as MPRIS2
	if (cairo_dock_dbus_is_enabled ())
	{
		if (pHandler->path != NULL)
		{
			myData.dbus_proxy_player = cairo_dock_create_new_session_proxy (
				pHandler->cMprisService,
				pHandler->path,
				pHandler->interface);
		}
		if (pHandler->path2 != NULL)
		{
			myData.dbus_proxy_shell = cairo_dock_create_new_session_proxy (
				pHandler->cMprisService,
				pHandler->path2,
				pHandler->interface2);
		}
		return (myData.dbus_proxy_player != NULL || myData.dbus_proxy_shell != NULL);
	}
	return FALSE;
}


void cd_musicplayer_dbus_disconnect_from_bus (void)
{
	if (myData.dbus_proxy_player != NULL)
	{
		g_object_unref (myData.dbus_proxy_player);
		myData.dbus_proxy_player = NULL;
	}
	if (myData.pDetectPlayerCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pDetectPlayerCall);
		myData.pDetectPlayerCall = NULL;
	}
	if (myData.dbus_proxy_shell != NULL)
	{
		g_object_unref (myData.dbus_proxy_shell);
		myData.dbus_proxy_shell = NULL;
	}
}


MusicPlayerHandler *cd_musicplayer_dbus_find_opened_player (void)
{
	if (myData.pCurrentHandler != NULL && myData.bIsRunning)
		return myData.pCurrentHandler;
	
	// get the list of services.	
	MusicPlayerHandler *pHandler = NULL;
	gchar **name_list = cairo_dock_dbus_get_services ();
	
	if (name_list != NULL)
	{
		// check if an MPRIS2 service is present.
		int i;
		for (i = 0; name_list[i] != NULL; i ++)
		{
			if (strncmp (name_list[i], CD_MPRIS2_SERVICE_BASE, strlen (CD_MPRIS2_SERVICE_BASE)) == 0)  // it's an MPRIS2 player.
			{
				pHandler = cd_musicplayer_get_handler_by_name ("Mpris2");
				g_free ((gchar*)pHandler->cMprisService);
				pHandler->cMprisService = g_strdup (name_list[i]);
				pHandler->appclass = g_strdup (name_list[i] + strlen (CD_MPRIS2_SERVICE_BASE)+1);  // only used to be displayed in the combo-box; we'll get the real data once we connect to it on the bus.
/*				gchar *str = strchr (pHandler->launch, '.');
				if (str)
					*str = '\0'; */
				break;
			}
		}
		
		// if no MPRIS2 service is present, look for a known handler.
		if (pHandler == NULL)
		{
			GList *h;
			MusicPlayerHandler *handler;
			for (i = 0; name_list[i] != NULL; i ++)
			{
				for (h = myData.pHandlers; h != NULL; h = h->next)  // see if a known handler matches.
				{
					handler = h->data;
					if (handler->cMprisService == NULL)
						continue;
					if (strcmp (name_list[i], handler->cMprisService) == 0)
					{
						pHandler = handler;
						break;
					}
				}
			}
		}
		
		g_strfreev (name_list);
	}
	return pHandler;
}

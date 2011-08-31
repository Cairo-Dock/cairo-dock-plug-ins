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
#include "applet-draw.h"
#include "applet-dbus.h"
#include "applet-musicplayer.h"

static void _on_name_owner_changed (const gchar *cName, gboolean bOwned, gpointer data);

/*
detect running:
get dbus services
foreach:
 is mpris2 -> get it
 is in handlers list -> get it

if no current handler:
 click/menu -> detect running player -> build list with the found one selected -> set as default in conf, get and launch handler

if preferred player is defined:
get handler from name
if none: make a generic handler from the service
set as current
watch it; watch mpris2 too
detect both async -> present => owned

owned => 
 if mpris2: set mpris2 handler as current
 launch it
not owned => stop handler

*/

MusicPlayerHandler *cd_musicplayer_get_handler_by_name (const gchar *cName)
{
	g_return_val_if_fail (cName != NULL, NULL);
	GList *h;
	MusicPlayerHandler *handler;
	for (h = myData.pHandlers; h != NULL; h = h->next)
	{
		handler = h->data;
		if (strcmp (handler->name, cName) == 0)
			return handler;
	}
	return NULL;
}

MusicPlayerHandler *cd_musicplayer_get_handler_by_service (const gchar *cService)
{
	g_return_val_if_fail (cService != NULL, NULL);
	GList *h;
	MusicPlayerHandler *handler;
	for (h = myData.pHandlers; h != NULL; h = h->next)
	{
		handler = h->data;
		if (handler->cMprisService && strcmp (handler->cMprisService, cService) == 0)
			return handler;
	}
	return NULL;
}


static void _cd_musicplayer_get_data_async (gpointer data) {
	if (myData.pCurrentHandler->get_data)
		myData.pCurrentHandler->get_data();
}

static gboolean _cd_musicplayer_get_data_and_update (gpointer data) {
	CD_APPLET_ENTER;
	if (myData.pCurrentHandler->get_data)
		myData.pCurrentHandler->get_data();
	return cd_musicplayer_draw_icon (data);  // cette fonction sort.
}

/* Initialise le backend et lance la tache periodique si necessaire.
 */
void cd_musicplayer_launch_handler (void)
{ 
	cd_debug ("%s (%s, %s)", __func__, myData.pCurrentHandler->name, myData.pCurrentHandler->appclass);
	// connect to the player.
	if (myData.dbus_proxy_player != NULL)  // don't start twice.
		return;
	if (! cd_musicplayer_dbus_connect_handler (myData.pCurrentHandler))
		return;
	
	// start the handler (connect to signals, or whatever the handler has to do internally).
	if (myData.pCurrentHandler->start != NULL)
	{
		myData.pCurrentHandler->start();
	}
	
	// start the timer.
	if (myData.pCurrentHandler->get_data && (myData.pCurrentHandler->iLevel == PLAYER_BAD || (myData.pCurrentHandler->iLevel == PLAYER_GOOD && (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT))))  // il y'a de l'acquisition de donnees periodique a faire.
	{
		if (myData.pCurrentHandler->bSeparateAcquisition == TRUE)  // Utilisation du thread pour les actions longues
		{
  			myData.pTask = cairo_dock_new_task (1,
  				(CairoDockGetDataAsyncFunc) _cd_musicplayer_get_data_async,
  				(CairoDockUpdateSyncFunc) cd_musicplayer_draw_icon,
  				NULL);
		}
		else
		{
  			myData.pTask = cairo_dock_new_task (1,
  				NULL,
  				(CairoDockUpdateSyncFunc) _cd_musicplayer_get_data_and_update,
  				NULL);
		}
		cairo_dock_launch_task (myData.pTask);
	}  // else all is done by signals.
	
	myData.bIsRunning = TRUE;
}

/* Relance le backend s'il avait ete arrete (lecteur en pause ou arrete).
 */
void cd_musicplayer_relaunch_handler (void)
{
	if (myData.pCurrentHandler->get_data && (myData.pCurrentHandler->iLevel == PLAYER_BAD || (myData.pCurrentHandler->iLevel == PLAYER_GOOD && (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT))))  // il y'a de l'acquisition de donnees periodique a faire.
	{
		if (!cairo_dock_task_is_active (myData.pTask))
			cairo_dock_launch_task (myData.pTask);
	}
}

/* Arrete le backend en nettoyant la memoire
 */
void cd_musicplayer_stop_current_handler (gboolean bStopWatching)
{
	if (myData.pCurrentHandler == NULL)
		return ;
	cd_debug ("MP : stopping %s", myData.pCurrentHandler->name);
	
	// cancel any detection or watching of the current handler.
	if (myData.pDetectPlayerCall != NULL)
	{
		dbus_g_proxy_cancel_call (cairo_dock_get_main_proxy (), myData.pDetectPlayerCall);
		myData.pDetectPlayerCall = NULL;
	}
	
	if (bStopWatching)
		cairo_dock_stop_watching_dbus_name_owner (myData.pCurrentHandler->cMprisService, (CairoDockDbusNameOwnerChangedFunc)_on_name_owner_changed);
	if (myData.cMpris2Service != NULL)  // can be null if we already got the MPRIS2 handler.
	{
		cairo_dock_stop_watching_dbus_name_owner (myData.cMpris2Service, (CairoDockDbusNameOwnerChangedFunc)_on_name_owner_changed);
		g_free (myData.cMpris2Service);
		myData.cMpris2Service = NULL;
	}
	
	// stop whatever the handler was doing internally.
	if (myData.pCurrentHandler->stop != NULL)
		myData.pCurrentHandler->stop();
	
	// disconnect from the bus, stop all signals/task
	cd_musicplayer_dbus_disconnect_from_bus ();
	
	cairo_dock_free_task (myData.pTask);
	myData.pTask = NULL;
	
	// return to initial state.
	myData.bIsRunning = FALSE;
	myData.iPlayingStatus = PLAYER_NONE;
	myData.iCurrentTime = 0;
	myData.iGetTimeFailed = 0;
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
}


void cd_musicplayer_register_my_handler (MusicPlayerHandler *pHandler)
{
	if (cd_musicplayer_get_handler_by_name (pHandler->name) != NULL)  // un peu paranoiaque...
	{
		cd_warning ("MP : Handler %s already listed", pHandler->name);
		return ;
	}
	
	myData.pHandlers = g_list_prepend (myData.pHandlers, pHandler);
}


/* Detruit un backend
 */
void cd_musicplayer_free_handler (MusicPlayerHandler *pHandler)
{
	if (pHandler == NULL)
		return ;
	
	g_free (pHandler->cCoverDir);
	g_free (pHandler);
}

static void _on_name_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%s, %d)", __func__, cName, bOwned);
	
	// launch or stop the handler.
	if (bOwned)
	{
		if (strncmp (cName, CD_MPRIS2_SERVICE_BASE, strlen (CD_MPRIS2_SERVICE_BASE)) == 0)  // the MPRIS2 service is now on the bus, it has priority.
		{
			cd_debug ("the MPRIS2 service is now on the bus, it has priority");
			if (strcmp (myData.pCurrentHandler->name, "Mpris2") != 0)  // our current handler is not the MPRIS2 one, stop it and use the latter instead.
			{
				cd_debug ("our current handler is not the MPRIS2 one, stop it and use the latter instead");
				gboolean bAlreadyWatching = FALSE;
				if (myData.cMpris2Service && strcmp (myData.cMpris2Service, cName) == 0)  // we're already watching it, don't unwatch (we can't unwatch ourselves in the callback, plus it's a waste of CPU).
				{
					bAlreadyWatching = TRUE;
					g_free (myData.cMpris2Service);
					myData.cMpris2Service = NULL;
				}
				cd_musicplayer_stop_current_handler (TRUE);  // so once we detect the MPRIS2 service on the bus, the other one is dropped forever.
				
				myData.pCurrentHandler = cd_musicplayer_get_handler_by_name ("Mpris2");
				g_free ((gchar*)myData.pCurrentHandler->cMprisService);  // well, in _this_ case the string is not constant.
				myData.pCurrentHandler->cMprisService = g_strdup (cName);
				
				myData.pCurrentHandler->launch = g_strdup (cName+strlen (CD_MPRIS2_SERVICE_BASE"."));
				myData.pCurrentHandler->appclass = g_strdup (cName+strlen (CD_MPRIS2_SERVICE_BASE"."));
				if (myConfig.bStealTaskBarIcon)
					CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandler->appclass);
				
				if (! bAlreadyWatching)
					cairo_dock_watch_dbus_name_owner (myData.pCurrentHandler->cMprisService, (CairoDockDbusNameOwnerChangedFunc) _on_name_owner_changed, NULL);
			}
		}
		else  // it's not the MPRIS2 service, ignore it if we already have the MPRIS2 service (shouldn't happen though).
		{
			if (strcmp (myData.pCurrentHandler->name, "Mpris2") == 0)
			{
				cd_debug ("it's not the MPRIS2 service, ignore it since we already have the MPRIS2 service");
				CD_APPLET_LEAVE ();
			}
		}
		
		cd_musicplayer_launch_handler ();
	}
	else  // else stop the handler.
	{
		cd_debug ("stop the handler");
		cd_musicplayer_stop_current_handler (FALSE);  // FALSE = keep watching it.
		cd_musicplayer_set_surface (PLAYER_NONE);
		if (myConfig.cDefaultTitle != NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
		else
		{
			if (strcmp (myData.pCurrentHandler->name, "Mpris2") == 0)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->launch);
			}
			else
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->name);
			}
		}
	}
	CD_APPLET_LEAVE ();
}

static void _on_detect_handler (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pDetectPlayerCall = NULL;
	cd_debug ("%s is present on the bus: %d\n", myData.pCurrentHandler->cMprisService, bPresent);
	if (bPresent)
	{
		_on_name_owner_changed (myData.pCurrentHandler->cMprisService, bPresent, data);
	}
	CD_APPLET_LEAVE ();
}

static void _on_detect_mpris2 (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pDetectPlayerCall = NULL;
	cd_debug ("MPRIS2 is present on the bus: %d\n", bPresent);
	if (bPresent)
		_on_name_owner_changed (myData.cMpris2Service, bPresent, data);
	else if (myData.pCurrentHandler->cMprisService != NULL)  // couldn't detect it, but it has another service, so try this one.
		myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.pCurrentHandler->cMprisService, (CairoDockOnAppliPresentOnDbus) _on_detect_handler, NULL);
	CD_APPLET_LEAVE ();
}

/*if prefered player is defined:
get handler from name
if none: make a generic handler from the service
set as current
watch it; watch mpris2 too
detect async -> present => owned*/
void cd_musicplayer_set_current_handler (const gchar *cName)
{
	cd_debug ("%s (%s)", __func__, cName);
	cd_musicplayer_stop_current_handler (TRUE);
	
	if (cName == NULL)
	{
		myData.pCurrentHandler = NULL;
		cd_musicplayer_set_surface (PLAYER_NONE);
		if (myConfig.cDefaultTitle == NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		return;
	}
	
	// find a handler which has this name
	myData.pCurrentHandler = cd_musicplayer_get_handler_by_name (cName);
	
	// make the corresponding MPRIS2 name in any case, as it is the standard way.
	gchar *cMpris2Service;
	if (strncmp (cName, CD_MPRIS2_SERVICE_BASE, strlen (CD_MPRIS2_SERVICE_BASE)) == 0)  // it's already an MPRIS2 service name
	{
		cMpris2Service = g_strdup (cName);
	}
	else  // make the default MPRIS2 service name.
	{
		cMpris2Service = g_strdup_printf (CD_MPRIS2_SERVICE_BASE".%s", cName);
	}
	
	// if no such handler, make a generic MPRIS2 handler.
	if (myData.pCurrentHandler == NULL)  // no such handler
	{
		myData.pCurrentHandler = cd_musicplayer_get_handler_by_name ("Mpris2");
		
		if (strncmp (cName, CD_MPRIS2_SERVICE_BASE".", strlen (CD_MPRIS2_SERVICE_BASE".")) == 0)
		{
			myData.pCurrentHandler->launch = g_strdup (cName+strlen (CD_MPRIS2_SERVICE_BASE"."));
			myData.pCurrentHandler->appclass = g_strdup (cName+strlen (CD_MPRIS2_SERVICE_BASE"."));
		}
		else
		{
			myData.pCurrentHandler->launch = g_strdup (cName);
			myData.pCurrentHandler->appclass = g_strdup (cName);
		}
		myData.pCurrentHandler->cMprisService = cMpris2Service;
	}
	else
	{
		myData.cMpris2Service = cMpris2Service;
	}
	
	// watch it on the bus.
	if (myData.cMpris2Service != NULL)
		cairo_dock_watch_dbus_name_owner (myData.cMpris2Service, (CairoDockDbusNameOwnerChangedFunc) _on_name_owner_changed, NULL);
	if (myData.pCurrentHandler->cMprisService != NULL)  // watch both, but MPRIS2 will have the priority.
		cairo_dock_watch_dbus_name_owner (myData.pCurrentHandler->cMprisService, (CairoDockDbusNameOwnerChangedFunc) _on_name_owner_changed, NULL);
	
	// detect its current presence on the bus.
	if (myData.cMpris2Service != NULL)
		myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.cMpris2Service, (CairoDockOnAppliPresentOnDbus) _on_detect_mpris2, NULL);  // mpris2 first, and then the other one.	
	else if (myData.pCurrentHandler->cMprisService != NULL)
		myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.pCurrentHandler->cMprisService, (CairoDockOnAppliPresentOnDbus) _on_detect_handler, NULL);
	cd_musicplayer_set_surface (PLAYER_NONE);  // meanwhile, consider it's not running.
	if (myConfig.cDefaultTitle == NULL)
	{
		if (strcmp (myData.pCurrentHandler->name, "Mpris2") == 0)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->launch);
			g_print ("SET NAME %s\n", myData.pCurrentHandler->launch);
		}
		else
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->name);
			g_print ("SET NAME %s\n", myData.pCurrentHandler->name);
		}
	}
	
	// manage its taskbar icon.
	if (myConfig.bStealTaskBarIcon)
		CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandler->appclass);
}

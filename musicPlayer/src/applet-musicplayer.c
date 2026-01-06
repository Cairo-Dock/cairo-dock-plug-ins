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

static inline void _fill_handler_properties (const gchar *cDesktopFileName, gchar *cAppClass)
{
	g_free ((gchar*)myData.pCurrentHandler->appclass);
	myData.pCurrentHandler->appclass = cAppClass;
	gldi_object_unref (GLDI_OBJECT (myData.pCurrentHandler->pAppInfo));

	myData.pCurrentHandler->pAppInfo = cairo_dock_get_class_app_info (myData.pCurrentHandler->appclass);
	if (myData.pCurrentHandler->pAppInfo) gldi_object_ref (GLDI_OBJECT (myData.pCurrentHandler->pAppInfo));
	
	// myData.pCurrentHandler->launch = g_strdup (cAppClass); // only used for display
	g_free ((gchar*)myData.pCurrentHandler->cDisplayedName);
	myData.pCurrentHandler->cDisplayedName = g_strdup (cairo_dock_get_class_name (myData.pCurrentHandler->appclass));
}

static inline void _get_right_class_and_desktop_file (const gchar *cName, gchar **cDesktopFileName, gchar **cAppClass)
{
	if (myConfig.cLastKnownDesktopFile)
	{
		*cDesktopFileName = myConfig.cLastKnownDesktopFile;
		*cAppClass = cairo_dock_register_class (*cDesktopFileName); // no need to be freed here
	}
	if (*cAppClass == NULL && cName) // myConfig.cLastKnownDesktopFile is NULL when transitionning from an old version of the applet where we didn't use the "Desktop Entry" property yet -> use some heuristic as a fallback.
	{
		*cAppClass = cairo_dock_register_class (cName); // no need to be freed here
		if (*cAppClass == NULL &&
			(*cDesktopFileName = strrchr (cName, '.')) != NULL) // if cName = org.mpris.MediaPlayer2.amarok => amarok
			*cAppClass = cairo_dock_register_class (*cDesktopFileName+1); // no need to be freed here
		else
			*cDesktopFileName = (gchar *)cName;
	}
	cd_debug ("%s (%s - %s) => (%s - %s)", __func__, myConfig.cLastKnownDesktopFile, cName, *cDesktopFileName, *cAppClass);
}

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


static void _cd_musicplayer_get_data_async (gpointer data) {
	if (myData.pCurrentHandler->get_data)
		myData.pCurrentHandler->get_data();
}

static gboolean _cd_musicplayer_update_from_data (gpointer data)
{
	g_return_val_if_fail (myData.pCurrentHandler->iLevel != PLAYER_EXCELLENT, FALSE);
	//cd_debug ("MP - %s (%d : %d -> %d)", __func__, myData.iPlayingStatus, myData.iPreviousCurrentTime, myData.iCurrentTime);
	CD_APPLET_ENTER;
	gboolean bNeedRedraw = FALSE;
	
	// All players: update the time.
	if (myData.iCurrentTime != myData.iPreviousCurrentTime)
	{
		myData.iPreviousCurrentTime = myData.iCurrentTime;
		if (myData.iPlayingStatus == PLAYER_PLAYING || myData.iPlayingStatus == PLAYER_PAUSED)
		{
			if (myData.iCurrentTime >= 0)  // peut etre -1 si le lecteur a demarre mais ne fournit pas encore de temps.
			{
				if (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED)
				{
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime);
				}
				else if (myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT)
				{
					CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (myData.iCurrentTime - myData.iSongLength);
				}
			}
			else
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		bNeedRedraw = TRUE;
	}
	
	// Bad players: update the icon if the state or song has changed (no signals for this).
	if (myData.pCurrentHandler->iLevel == PLAYER_BAD)
	{
		if (myData.iPlayingStatus != myData.pPreviousPlayingStatus)  // state has changed.
		{
			cd_debug ("MP : PlayingStatus : %d -> %d", myData.pPreviousPlayingStatus, myData.iPlayingStatus);
			myData.pPreviousPlayingStatus = myData.iPlayingStatus;
			
			cd_musicplayer_update_icon ();
			bNeedRedraw = FALSE;
		}
		else if (cairo_dock_strings_differ (myData.cPreviousRawTitle, myData.cRawTitle))  // song has changed.
		{
			g_free (myData.cPreviousRawTitle);
			myData.cPreviousRawTitle = g_strdup (myData.cRawTitle);
			cd_musicplayer_update_icon ();
			bNeedRedraw = FALSE;
		}
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	CD_APPLET_LEAVE (myData.pCurrentHandler->iLevel == PLAYER_BAD || (myData.pCurrentHandler->iLevel == PLAYER_GOOD && myData.iPlayingStatus == PLAYER_PLAYING));
}

static gboolean _cd_musicplayer_get_data_and_update (gpointer data) {
	CD_APPLET_ENTER;
	if (myData.pCurrentHandler->get_data)
		myData.pCurrentHandler->get_data();
	return _cd_musicplayer_update_from_data (data);  // cette fonction sort.
}

/* Initialise le backend et lance la tache periodique si necessaire.
 */
void cd_musicplayer_launch_handler (void)
{ 
	cd_debug ("%s (%s, %s)", __func__, myData.pCurrentHandler->name, myData.pCurrentHandler->appclass);
	if (myData.pProxyMain != NULL)  // don't start twice.
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
  			myData.pTask = gldi_task_new (1,
  				(GldiGetDataAsyncFunc) _cd_musicplayer_get_data_async,
  				(GldiUpdateSyncFunc) _cd_musicplayer_update_from_data,
  				NULL);
		}
		else
		{
  			myData.pTask = gldi_task_new (1,
  				NULL,
  				(GldiUpdateSyncFunc) _cd_musicplayer_get_data_and_update,
  				NULL);
		}
		gldi_task_launch (myData.pTask);
	}  // else all is done by signals.
	
	myData.bIsRunning = TRUE;
}

/* Relance le backend s'il avait ete arrete (lecteur en pause ou arrete).
 */
void cd_musicplayer_relaunch_handler (void)
{
	if (myData.pCurrentHandler->get_data && (myData.pCurrentHandler->iLevel == PLAYER_BAD || (myData.pCurrentHandler->iLevel == PLAYER_GOOD && (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT))))  // il y'a de l'acquisition de donnees periodique a faire.
	{
		if (!gldi_task_is_active (myData.pTask))
			gldi_task_launch (myData.pTask);
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
	if (myData.pGetPropsCall)
	{
		dbus_g_proxy_cancel_call (cairo_dock_get_main_proxy (), myData.pGetPropsCall);
		myData.pGetPropsCall = NULL;
	}
	
	if (bStopWatching)
	{
		cairo_dock_stop_watching_dbus_name_owner (myData.pCurrentHandler->cMprisService, (CairoDockDbusNameOwnerChangedFunc)_on_name_owner_changed);
		if (myData.cMpris2Service != NULL)  // can be null if we already got the MPRIS2 handler.
		{
			cairo_dock_stop_watching_dbus_name_owner (myData.cMpris2Service, (CairoDockDbusNameOwnerChangedFunc)_on_name_owner_changed);
			g_free (myData.cMpris2Service);
			myData.cMpris2Service = NULL;
		}
	}
	
	// stop whatever the handler was doing internally.
	if (myData.pCurrentHandler->stop != NULL)
		myData.pCurrentHandler->stop();
	
	// disconnect from the bus, stop all signals/task
	cd_musicplayer_dbus_disconnect_from_bus ();
	
	gldi_task_free (myData.pTask);
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
void cd_musicplayer_free_handler (gpointer data)
{
	if (data == NULL)
		return ;
	
	MusicPlayerHandler *pHandler = (MusicPlayerHandler*)data;
	g_free (pHandler->cCoverDir);
	g_free (pHandler->appclass);
	gldi_object_unref (GLDI_OBJECT (pHandler->pAppInfo));
	g_free (pHandler);
}

static void _on_got_desktop_entry (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pGetPropsCall = NULL;
	
	GValue v = G_VALUE_INIT;
	GError *error = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&error,
		G_TYPE_VALUE,
		&v,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("%s", error->message);
		g_error_free (error);
	}
	if (bSuccess && G_VALUE_HOLDS_STRING (&v))
	{
		const gchar *cDesktopFileName = g_value_get_string (&v);
		cd_debug (" got desktop-entry '%s' (was '%s') from the service '%s'", cDesktopFileName, myConfig.cLastKnownDesktopFile, myData.pCurrentHandler->cMprisService);
		
		if (cDesktopFileName != NULL)
		{
			// convert to lowercase, as that is expected by cairo_dock_register_class ()
			gchar *cDesktopFileLower = g_ascii_strdown (cDesktopFileName, -1);
			if (myConfig.cLastKnownDesktopFile == NULL || strcmp (cDesktopFileLower, myConfig.cLastKnownDesktopFile) != 0)  // the property has changed from the previous time.
			{
				gchar *cAppClass = cairo_dock_register_class (cDesktopFileLower); // no need to be freed here
				cd_debug ("  desktop-entry has changed, update => Class: %s", cAppClass);
				if (cAppClass != NULL) // maybe the application has given a wrong cAppClass... Amarok? :)
				{
					// store the desktop filename, since we can't have it until the service is up, the next time the applet is started, which means we wouldn't be able to launch the player.
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
						G_TYPE_STRING, "Configuration", "desktop-entry", cDesktopFileLower,
						G_TYPE_INVALID);
					g_free (myConfig.cLastKnownDesktopFile);
					myConfig.cLastKnownDesktopFile = cDesktopFileLower;
					
					// register the desktop file, and get the common properties of this class.
					_fill_handler_properties (cDesktopFileLower, cAppClass);
					
					if (myData.pCurrentHandler->appclass != NULL)
					{
						cairo_dock_set_data_from_class (myData.pCurrentHandler->appclass, myIcon);
					}
					
					if (myConfig.bStealTaskBarIcon)
						CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandler->appclass);
				}
				else
				{
					cd_warning ("Wrong .desktop file name: %s", cDesktopFileLower);
					g_free (cDesktopFileLower);
				}
			}
		}
		g_value_unset (&v);
	}
	CD_APPLET_LEAVE ();
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
			// set the MPRIS2 handler as the current one if not already the case.
			if (strcmp (myData.pCurrentHandler->name, "Mpris2") != 0)  // the current handler is not the MPRIS2 one, stop it and use the latter instead.
			{
				cd_debug ("our current handler is not the MPRIS2 one, stop it and use the latter instead");
				// stop the old handler
				if (myData.cMpris2Service != cName)
					g_free (myData.cMpris2Service);
				myData.cMpris2Service = NULL;  // we're already watching it, don't re-watch (we can't unwatch ourselves in the callback, plus it's a waste of CPU).
				
				cd_musicplayer_stop_current_handler (TRUE);  // so once we detect the MPRIS2 service on the bus, the other one is dropped forever.
				
				// set the MPRIS2 handler as the current one
				myData.pCurrentHandler = cd_musicplayer_get_handler_by_name ("Mpris2");  // no need to watch it, it was already done (that's why we are here !)
				
				// fill its properties
				gchar *cAppClass = NULL, *cDesktopFileName = NULL;
				_get_right_class_and_desktop_file (cName, &cDesktopFileName, &cAppClass);
				if (cAppClass) // better to not use any class than a wrong class
					_fill_handler_properties (cDesktopFileName, cAppClass);

				g_free ((gchar*)myData.pCurrentHandler->cMprisService);
				myData.pCurrentHandler->cMprisService = g_strdup (cName);
			}
			// get the desktop properties of the player.
			// we do it now that the service is on the bus, because we can't guess them for sure from the MPRIS service name only (ex.: Rhythmbox has "org.mpris.MediaPlayer2.rhythmbox3" but its class is "rhythmbox").
			DBusGProxy *pProxyProp = cairo_dock_create_new_session_proxy (
				myData.pCurrentHandler->cMprisService,
				CD_MPRIS2_OBJ,
				DBUS_INTERFACE_PROPERTIES);
			if (myData.pGetPropsCall)
				dbus_g_proxy_cancel_call (cairo_dock_get_main_proxy (), myData.pGetPropsCall);
			myData.pGetPropsCall = dbus_g_proxy_begin_call (pProxyProp, "Get",
				(DBusGProxyCallNotify)_on_got_desktop_entry,
				myApplet,
				(GDestroyNotify) NULL,
				G_TYPE_STRING, CD_MPRIS2_MAIN_IFACE,
				G_TYPE_STRING, "DesktopEntry",
				G_TYPE_INVALID);
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
		cd_debug ("stop the handler {%s, %s}", myData.pCurrentHandler->name, myData.pCurrentHandler->appclass);
		cd_musicplayer_stop_current_handler (FALSE);  // FALSE = keep watching it.
		cd_musicplayer_apply_status_surface (PLAYER_NONE);
		if (myConfig.cDefaultTitle != NULL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
		}
		else
		{
			if (strcmp (myData.pCurrentHandler->name, "Mpris2") == 0)
			{
				gchar *cDefaultName = cd_musicplayer_get_string_with_first_char_to_upper (myData.pCurrentHandler->appclass);
				CD_APPLET_SET_NAME_FOR_MY_ICON (cDefaultName);
				g_free (cDefaultName);
			}
			else
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->name);
			}
		cd_debug ("stopped {%s, %s}", myData.pCurrentHandler->name, myData.pCurrentHandler->appclass);
		}
	}
	CD_APPLET_LEAVE ();
}

static void _on_detect_handler (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pDetectPlayerCall = NULL;
	cd_debug ("%s presence on the bus: %d", myData.pCurrentHandler->cMprisService, bPresent);
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
	cd_debug ("MPRIS2 presence on the bus: %d", bPresent);
	if (bPresent)
		_on_name_owner_changed (myData.cMpris2Service, bPresent, data);
	else if (myData.pCurrentHandler->cMprisService != NULL)  // couldn't detect it, but it has another service, so try this one.
		myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.pCurrentHandler->cMprisService, (CairoDockOnAppliPresentOnDbus) _on_detect_handler, NULL);
	CD_APPLET_LEAVE ();
}

/**
Set the current handler from its name.
It can be either the name of an old handler, or the name of an MPRIS2 service.
 - if an old handler matches the name, use it, and detect it on the bus; also detect an MPRIS2 with the same name
 - if no handler matches, then it's an MPRIS2 service; set the MPRIS2 handler as the current one, and detect it on the bus.
*/
void cd_musicplayer_set_current_handler (const gchar *cName)
{
	cd_debug ("%s (%s)", __func__, cName);
	// stop completely any previous handler
	cd_musicplayer_stop_current_handler (TRUE);
	
	// if no handler is defined, go to a neutral state.
	if (cName == NULL)
	{
		myData.pCurrentHandler = NULL;
		cd_musicplayer_apply_status_surface (PLAYER_NONE);
		if (myConfig.cDefaultTitle == NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		return;
	}
	
	// find a handler from the given name
	myData.pCurrentHandler = cd_musicplayer_get_handler_by_name (cName);
	
	if (myData.pCurrentHandler != NULL)  // an old handler exist with this name, use it but also look for the associated MPRIS2 service.
	{
		myData.cMpris2Service = myData.pCurrentHandler->cMpris2Service ? g_strdup (myData.pCurrentHandler->cMpris2Service) : g_strdup_printf (CD_MPRIS2_SERVICE_BASE".%s", cName);
		cd_debug ("We check this MPRIS2 service: %s", myData.cMpris2Service);
		
		cairo_dock_watch_dbus_name_owner (myData.cMpris2Service, (CairoDockDbusNameOwnerChangedFunc) _on_name_owner_changed, NULL);
		
		myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.cMpris2Service, (CairoDockOnAppliPresentOnDbus) _on_detect_mpris2, NULL);  // mpris2 first, and then the other one.
	}
	else  // no such handler, make an MPRIS2 service with this name.
	{
		// get the MPRIS2 handler
		myData.pCurrentHandler = cd_musicplayer_get_handler_by_name ("Mpris2");
		
		gchar *cAppClass = NULL, *cDesktopFileName = NULL;
		_get_right_class_and_desktop_file (cName, &cDesktopFileName, &cAppClass);
		if (cAppClass) // better to not use any class than a wrong class
			_fill_handler_properties (cDesktopFileName, cAppClass);
		
		myData.pCurrentHandler->cMprisService = g_strdup_printf (CD_MPRIS2_SERVICE_BASE".%s", cName);
		myData.cMpris2Service = NULL;
	}
	
	// watch it on the bus.
	if (myData.pCurrentHandler->cMprisService != NULL)  // paranoia
	{
		cairo_dock_watch_dbus_name_owner (myData.pCurrentHandler->cMprisService, (CairoDockDbusNameOwnerChangedFunc) _on_name_owner_changed, NULL);
	
		// detect its presence on the bus.
		if (myData.pDetectPlayerCall == NULL)  // if we're already detecting MPRIS2, we'll detect this handler after we got the answer.
			myData.pDetectPlayerCall = cairo_dock_dbus_detect_application_async (myData.pCurrentHandler->cMprisService, (CairoDockOnAppliPresentOnDbus) _on_detect_handler, NULL);
	}
	
	// set the current icon and label.
	if (myData.pCurrentHandler->appclass != NULL)
	{
		cairo_dock_set_data_from_class (myData.pCurrentHandler->appclass, myIcon);
	}
	
	cd_musicplayer_apply_status_surface (PLAYER_NONE);  // until we detect any service, consider it's not running.
	if (myConfig.cDefaultTitle == NULL)
	{
		/**if (myIcon->cName != NULL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myIcon->cName);
		}
		else */if (strcmp (myData.pCurrentHandler->name, "Mpris2") != 0)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->name);
		}
		else
		{
			gchar *cDefaultName = cd_musicplayer_get_string_with_first_char_to_upper (myData.pCurrentHandler->appclass);
			CD_APPLET_SET_NAME_FOR_MY_ICON (cDefaultName);
			g_free (cDefaultName);
		}
	}
	
	// manage its taskbar icon.
	if (myData.pCurrentHandler->appclass != NULL)
	{
		cairo_dock_set_data_from_class (myData.pCurrentHandler->appclass, myIcon);
	}
	
	if (myConfig.bStealTaskBarIcon)
		CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandler->appclass);
}

gchar *cd_musicplayer_get_string_with_first_char_to_upper (const gchar *cName)
{
	return (cName == NULL) ? NULL : g_strdup_printf ("%c%s", g_ascii_toupper (*cName), cName + 1);
}

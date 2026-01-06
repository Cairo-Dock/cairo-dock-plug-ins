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
	
	if (bStopWatching && myData.uNameWatch)
	{
		g_bus_unwatch_name (myData.uNameWatch);
		myData.uNameWatch = 0;
	}
	
	// stop whatever the handler was doing internally.
	if (myData.pCurrentHandler->stop != NULL)
		myData.pCurrentHandler->stop();
	
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

static void _on_name_appeared (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cName,
	G_GNUC_UNUSED const gchar *cOwner, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	cd_musicplayer_launch_handler ();
	
	CD_APPLET_LEAVE ();
}
	
static void _on_name_vanished (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cName,
	G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	cd_debug ("stop the handler {%s, %s}", myData.pCurrentHandler->name, myData.pCurrentHandler->appclass);
	cd_musicplayer_stop_current_handler (FALSE);  // FALSE = keep watching it.
	cd_musicplayer_apply_status_surface (PLAYER_NONE);
	if (myConfig.cDefaultTitle != NULL)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
	}
	else
	{
		if (myData.pCurrentHandler->cDisplayedName) CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->cDisplayedName);
		else CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cMpris2Service + strlen (CD_MPRIS2_SERVICE_BASE) + 1);
	}
	
	CD_APPLET_LEAVE ();
}

static gboolean s_bNeedConfigUpdate = FALSE;

static void _update_handler_appinfo (MusicPlayerHandler *pHandler, const gchar *cDesktopFileName)
{
	pHandler->appclass = cairo_dock_register_class (cDesktopFileName);
	if (pHandler->appclass)
	{
		cairo_dock_set_data_from_class (pHandler->appclass, myIcon);
		pHandler->cDisplayedName = g_strdup (cairo_dock_get_class_name (pHandler->appclass));
		pHandler->pAppInfo = cairo_dock_get_class_app_info (pHandler->appclass);
		if (pHandler->pAppInfo) gldi_object_ref (GLDI_OBJECT (pHandler->pAppInfo));
		else cd_warning ("Cannot get app info for this class: '%s', will not be able to launch the music player", pHandler->appclass);
	}
	else cd_warning ("Cannot find this app: '%s', will not be able to launch the music player", cDesktopFileName);
}

void cd_musicplayer_set_current_handler (const gchar *cMpris2Service, const gchar *cAppName, const gchar *cDesktopFileName,
	gboolean bUpdateConfig, gboolean bUpdateIcon)
{
	// stop completely any previous handler and free related data
	cd_musicplayer_stop_current_handler (TRUE);
	
	if (myData.pCurrentHandler)
	{
		g_free (myData.pCurrentHandler->cDisplayedName);
		myData.pCurrentHandler->cDisplayedName = NULL;
		g_free (myData.pCurrentHandler->appclass);
		if (myData.pCurrentHandler->pAppInfo) gldi_object_unref (GLDI_OBJECT (myData.pCurrentHandler->pAppInfo));
		myData.pCurrentHandler->appclass = NULL;
		myData.pCurrentHandler->pAppInfo = NULL;
	}
	
	if (myData.uNameWatch)
	{
		g_bus_unwatch_name (myData.uNameWatch);
		myData.uNameWatch = 0;
	}
	
	g_free (myData.cMpris2Service);
	
	// stop the association with any appli
	CD_APPLET_MANAGE_APPLICATION (NULL);
	
	s_bNeedConfigUpdate = FALSE;
	if (bUpdateConfig)
	{
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING, "Configuration", "current-player", cAppName ? cAppName : "",
			// reset the desktop filename if not given, we'll get the new one from the "DesktopEntry" property of the new player
			G_TYPE_STRING, "Configuration", "desktop-entry", cDesktopFileName ? cDesktopFileName : "",
			// add the MPRIS2 name as it will be needed when loading the next time
			G_TYPE_STRING, "Configuration", "mpris2-name", cMpris2Service ? cMpris2Service : "",
			G_TYPE_INVALID);
		
		g_free (myConfig.cLastKnownDesktopFile);
		myConfig.cLastKnownDesktopFile = cDesktopFileName ? g_strdup (cDesktopFileName) : NULL;
		g_free (myConfig.cMusicPlayer);
		myConfig.cMusicPlayer = cAppName ? g_strdup (cAppName) : NULL;
		if (cMpris2Service && !cDesktopFileName) s_bNeedConfigUpdate = TRUE;
	}
	
	// if no handler is defined, go to a neutral state.
	if (!cMpris2Service)
	{
		myData.cMpris2Service = NULL;
		myData.pCurrentHandler = NULL;
		if (bUpdateIcon)
		{
			cd_musicplayer_apply_status_surface (PLAYER_NONE);
			if (myConfig.cDefaultTitle == NULL)
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		}
		return;
	}
	
	// we only support Mpris2 for now
	myData.cMpris2Service = g_strdup (cMpris2Service);
	myData.pCurrentHandler = cd_musicplayer_get_handler_by_name ("Mpris2");
	MusicPlayerHandler *pHandler = myData.pCurrentHandler;
	
	if (cDesktopFileName)
	{
		_update_handler_appinfo (pHandler, cDesktopFileName);
		if (pHandler->cDisplayedName && !cAppName && bUpdateConfig)
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING,
				"Configuration", "current-player", pHandler->cDisplayedName, G_TYPE_INVALID);
	}
	
	if (cAppName && !pHandler->cDisplayedName)
		pHandler->cDisplayedName = g_strdup (cAppName);

	// watch it on the bus.
	myData.uNameWatch = g_bus_watch_name (G_BUS_TYPE_SESSION, cMpris2Service,
		G_BUS_NAME_WATCHER_FLAGS_NONE, _on_name_appeared, _on_name_vanished, NULL, NULL);
	
	// set the current icon and label.
	if (myData.pCurrentHandler->appclass != NULL)
	{
		cairo_dock_set_data_from_class (myData.pCurrentHandler->appclass, myIcon);
	}
	
	if (bUpdateIcon)
	{
		cd_musicplayer_apply_status_surface (PLAYER_NONE);  // until we detect any service, consider it's not running.
		if (myConfig.cDefaultTitle == NULL)
		{
			if (pHandler->cDisplayedName) CD_APPLET_SET_NAME_FOR_MY_ICON (pHandler->cDisplayedName);
			else CD_APPLET_SET_NAME_FOR_MY_ICON (cMpris2Service + strlen (CD_MPRIS2_SERVICE_BASE) + 1);
		}
	}
	
	// manage its taskbar icon.
	if (pHandler->appclass && myConfig.bStealTaskBarIcon)
		CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandler->appclass);
}

void cd_musicplayer_on_got_desktop_entry (const gchar *cDesktopFileName)
{
	MusicPlayerHandler *pHandler = myData.pCurrentHandler;
	if (!pHandler->pAppInfo) // we did not have a desktop file name originally
	{
		g_free (pHandler->appclass);
		g_free (pHandler->cDisplayedName);
		_update_handler_appinfo (pHandler, cDesktopFileName);
		
		if (s_bNeedConfigUpdate)
		{
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_STRING, "Configuration", "current-player", pHandler->cDisplayedName,
				G_TYPE_STRING, "Configuration", "desktop-entry", cDesktopFileName,
				G_TYPE_INVALID);
			g_free (myConfig.cLastKnownDesktopFile);
			myConfig.cLastKnownDesktopFile = g_strdup (cDesktopFileName);
			s_bNeedConfigUpdate = FALSE;
			g_free (myConfig.cMusicPlayer);
			myConfig.cMusicPlayer = g_strdup (pHandler->cDisplayedName);
		}
	}
}


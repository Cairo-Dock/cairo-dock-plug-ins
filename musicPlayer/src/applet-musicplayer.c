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
	}
	
	// if no handler is defined, go to a neutral state.
	if (!(cMpris2Service && cDesktopFileName))
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


typedef struct _CDKnownMusicPlayer
{
	const gchar *id; // desktop file ID (case insensitive)
	const gchar *alt_id; // alternative (currently we know of at most two .desktop file IDs for each player)
	const gchar *mpris2; // MPRIS2 DBus name
	const gchar *name; // display name + name in config file
} CDKnownMusicPlayer;

static const CDKnownMusicPlayer s_players[] =
{
	{"org.kde.amarok", NULL, "org.mpris.MediaPlayer2.amarok", "Amarok"},
	{"audacious", "audacious2", "org.mpris.MediaPlayer2.audacious", "Audacious"},
	{"org.clementine_player.clementine", NULL, "org.mpris.MediaPlayer2.clementine", "Clementine"},
	{"exaile", NULL, "org.mpris.MediaPlayer2.exaile", "Exaile"},
	{"gmusicbrowser", NULL, "org.mpris.MediaPlayer2.gmusicbrowser", "GMusicBrowser"},
	{"org.guayadeque.guayadeque", "guayadeque", "org.mpris.MediaPlayer2.guayadeque", "Guayadeque"},
	{"qmmp-1", "qmmp", "org.mpris.MediaPlayer2.qmmp", "Qmmp"},
	{"io.github.quodlibet.quodlibet", "quodlibet", "org.mpris.MediaPlayer2.quodlibet", "QuodLibet"},
	{"org.gnome.rhythmbox3", "rhythmbox3", "org.mpris.MediaPlayer2.rhythmbox", "Rhythmbox"},
	{NULL, NULL, NULL, NULL}
};


void cd_musicplayer_info_free (CDMPInfo *pInfo)
{
	if (pInfo)
	{
		g_free (pInfo->cName);
		g_free (pInfo->cDesktopFile);
		g_free (pInfo->cMpris2Name);
		g_free (pInfo);
	}
}

typedef struct _CDMPInfoData {
	GCancellable *pCancel;
	CDMPInfoCB cb;
	gpointer user_data;
	GList *res;
	unsigned int to_process;
} CDMPInfoData;


static int _info_cmp (gconstpointer x, gconstpointer y)
{
	const CDMPInfo *pInfo = (const CDMPInfo*)x;
	const char *cMPRIS2 = (const char*)y;
	if (!pInfo->cMpris2Name || !cMPRIS2) return -1;
	return strcmp (pInfo->cMpris2Name, cMPRIS2);
}

static gboolean _get_players_finish (gpointer ptr)
{
	CD_APPLET_ENTER;
	
	CDMPInfoData *pData = (CDMPInfoData*)ptr;
	
	if (!pData->pCancel || !g_cancellable_is_cancelled (pData->pCancel))
	{
		GList *to_add = NULL;
		int i;
		for (i = 0; s_players[i].id; i++)
		{
			GList *x = g_list_find_custom (pData->res, s_players[i].mpris2, _info_cmp);
			if (x) continue; // already found
			
			gboolean bAlt = FALSE;
			gchar *tmp = cairo_dock_register_class (s_players[i].id);
			if (!tmp && s_players[i].alt_id)
			{
				bAlt = TRUE;
				tmp = cairo_dock_register_class (s_players[i].alt_id);
			}
			if (tmp)
			{
				CDMPInfo *pInfo = g_new0 (CDMPInfo, 1);
				pInfo->cDesktopFile = g_strdup (bAlt ? s_players[i].alt_id : s_players[i].id);
				pInfo->cMpris2Name = g_strdup (s_players[i].mpris2);
				pInfo->cName = g_strdup (s_players[i].name);
				to_add = g_list_prepend (to_add, pInfo);
				g_free (tmp);
			}
		}
		GList *res = g_list_concat (to_add, pData->res);
		pData->cb (TRUE, res);
	}
	
	// already canceled, need to free partial results
	g_list_free_full (pData->res, (GDestroyNotify)cd_musicplayer_info_free);
	
	if (pData->pCancel) g_object_unref (pData->pCancel);
	g_free (pData);
	
	CD_APPLET_LEAVE (G_SOURCE_REMOVE);
}

typedef struct _CDMPQueryData {
	CDMPInfoData *pData;
	CDMPInfo *pInfo;
} CDMPQueryData;

static void _on_got_info (GObject *pObj, GAsyncResult *pRes, gpointer data, gboolean bName)
{
	CD_APPLET_ENTER;
	
	CDMPQueryData *pQuery = (CDMPQueryData*)data;
	CDMPInfoData *pData = pQuery->pData;
	
	gboolean bFinished = FALSE;
	gboolean bError = FALSE;
	if (! pQuery->pInfo->bIsRunning)
	{
		bFinished = TRUE;
		bError = TRUE; // previous error
	}
	else if ( (bName && pQuery->pInfo->cDesktopFile) ||
		(!bName && pQuery->pInfo->cName)) bFinished = TRUE; // other data is ready
	
	GError *err = NULL;
	GDBusConnection *pConn = G_DBUS_CONNECTION (pObj);
	GVariant *res = g_dbus_connection_call_finish (pConn, pRes, &err);
	if (err)
	{
		// note: error can happen if player exited in the meantime
		pQuery->pInfo->bIsRunning = FALSE; // signal an error
		bError = TRUE;
	}
	else
	{
		if (!bError)
		{
			GVariant *tmp1 = g_variant_get_child_value (res, 0);
			GVariant *tmp2 = g_variant_get_variant (tmp1);
			if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("s")))
			{
				if (bName) g_variant_get (tmp2, "s", &pQuery->pInfo->cName);
				else
				{
					gsize len;
					const gchar *v2 = g_variant_get_string (tmp2, &len);
					pQuery->pInfo->cDesktopFile = g_ascii_strdown (v2, len);
				}
			}
			else
			{
				pQuery->pInfo->bIsRunning = FALSE; // signal an error
				bError = TRUE;
				cd_warning ("Unexpected property type: %s", g_variant_get_type_string (tmp2));
			}
			
			g_variant_unref (tmp2);
			g_variant_unref (tmp1);
		}
		
		g_variant_unref (res);
	}
	
	if (bFinished)
	{
		if (!bError)
			pData->res = g_list_prepend (pData->res, pQuery->pInfo);
		else cd_musicplayer_info_free (pQuery->pInfo);
		g_free (pQuery);
		
		pData->to_process--;
		if (!pData->to_process)
			// processed all running players, check the rest and call back
			g_idle_add (_get_players_finish, pData);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_got_name (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	_on_got_info (pObj, pRes, data, TRUE);
}

static void _on_got_desktop (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	_on_got_info (pObj, pRes, data, FALSE);
}

static void _on_got_players_running (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	
	CDMPInfoData *pData = (CDMPInfoData*)ptr;
	GError *err = NULL;
	GDBusConnection *pConn = G_DBUS_CONNECTION (pObj);
	GVariant *res = g_dbus_connection_call_finish (pConn, pRes, &err);
	if (err)
	{
		if (!g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting the list of active DBus services: %s", err->message);
		
		// we need to free our data (note: in this case, pData->res == NULL)
		g_error_free (err);
		if (pData->pCancel) g_object_unref (pData->pCancel); // ref was taken before this call
		pData->cb (FALSE, NULL);
		g_free (pData);
		CD_APPLET_LEAVE ();
	}
	else
	{
		// type of res is (as), checked by GLib
		GVariantIter *it = NULL;
		const gchar *cName;
		const size_t len1 = strlen (CD_MPRIS2_SERVICE_BASE);
		g_variant_get (res, "(as)", &it);
		while (g_variant_iter_loop (it, "&s", &cName))
			if (strncmp (cName, CD_MPRIS2_SERVICE_BASE, len1) == 0)  // it's an MPRIS2 player.
			{
				if (cName[len1] != '.') continue; // check just in case
				// leave out firefox, it is for controlling whatever media is currently playing
				// (TODO: identify other browsers as well)
				if (strncmp (cName + len1 + 1, "firefox", 7) == 0) continue;
				
				pData->to_process++;
				
				// need to get name and desktop file as properties
				CDMPQueryData *pQuery = g_new0 (CDMPQueryData, 1);
				pQuery->pData = pData;
				pQuery->pInfo = g_new0 (CDMPInfo, 1);
				pQuery->pInfo->cMpris2Name = g_strdup (cName);
				pQuery->pInfo->bIsRunning = TRUE;
				
				g_dbus_connection_call (pConn, cName, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties",
					"Get", g_variant_new ("(ss)", CD_MPRIS2_MAIN_IFACE, "Identity"), G_VARIANT_TYPE ("(v)"),
					G_DBUS_CALL_FLAGS_NONE, 500, pData->pCancel, _on_got_name, pQuery);
				g_dbus_connection_call (pConn, cName, CD_MPRIS2_OBJ, "org.freedesktop.DBus.Properties",
					"Get", g_variant_new ("(ss)", CD_MPRIS2_MAIN_IFACE, "DesktopEntry"), G_VARIANT_TYPE ("(v)"),
					G_DBUS_CALL_FLAGS_NONE, 500, pData->pCancel, _on_got_desktop, pQuery);
			}
		g_variant_iter_free (it);
		g_variant_unref (res);
	}
	
	if (!pData->to_process)
		// no running players, just check the rest and call back
		g_idle_add (_get_players_finish, pData);
	
	CD_APPLET_LEAVE ();
}

void cd_musicplayer_get_known_players (CDMPInfoCB cb, GCancellable *pCancel)
{
	g_return_if_fail (cb != NULL);
	
	GDBusConnection *pConn = cairo_dock_dbus_get_session_bus ();
	if (!pConn)
	{
		cd_warning ("DBus not available, cannot find music players");
		cb (FALSE, NULL);
		return;
	}
	
	CDMPInfoData *pData = g_new0 (CDMPInfoData, 1);
	pData->pCancel = pCancel;
	pData->cb = cb;
	if (pCancel) g_object_ref (pCancel);
	
	g_dbus_connection_call (pConn, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
		"ListNames", NULL, G_VARIANT_TYPE ("(as)"), G_DBUS_CALL_FLAGS_NONE, -1,
		pCancel, _on_got_players_running, pData);
}


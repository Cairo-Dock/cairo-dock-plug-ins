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
#include <sys/types.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-item.h"
#include "applet-draw.h"
#include "applet-host.h"
#include "applet-host-ias.h"  // for fallback
#include "applet-host-kde.h"

// KDE watcher
#define CD_STATUS_NOTIFIER_WATCHER_ADDR "org.kde.StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_OBJ "/StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_IFACE "org.kde.StatusNotifierWatcher"

static DBusGProxyCall *s_pDetectWatcherCall = NULL;


  ///////////////
 /// Signals ///
///////////////

static void on_new_item (DBusGProxy *proxy_watcher, const gchar *cNotifierItemId, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%s)", __func__, cNotifierItemId);
	
	gchar *cService = NULL, *cObjectPath = NULL;
	gchar *str = strchr (cNotifierItemId, '/');
	if (str != NULL)  // service + path
	{
		cService = g_strndup (cNotifierItemId, str - cNotifierItemId);
		cObjectPath = str;
	}
	else  // we handle this case too, by supposing the path is the default /StatusNotifierItem
	{
		cService = g_strdup (cNotifierItemId);
		cObjectPath = NULL;
	}
	
	cd_satus_notifier_add_new_item (cService, cObjectPath, -1);  // indicator-application's positions start from 0, so it will never be -1.
	
	g_free (cService);
	CD_APPLET_LEAVE ();
}

static void on_removed_item (DBusGProxy *proxy_watcher, const gchar *cNotifierItemId, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%s)", __func__, cNotifierItemId);
	
	gchar *str = strchr (cNotifierItemId, '/');
	if (str != NULL)  // service + path, remove the path, we only need the service.
		*str = '\0';
	
	cd_satus_notifier_remove_item (cNotifierItemId, -1);
	
	CD_APPLET_LEAVE ();
}


  /////////////////
 /// Get Items ///
/////////////////

static void _on_get_applications_from_watcher (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	
	GError *erreur = NULL;
	GValue *v = g_new0 (GValue, 1);
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_VALUE, v,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug ("=== couldn't get applications from the watcher (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bSuccess = FALSE;
	}
	
	if (bSuccess)
	{
		if (!G_VALUE_HOLDS_BOXED (v))
			CD_APPLET_LEAVE ();
		gchar **pApplications = g_value_get_boxed (v);
		if (pApplications == NULL)
			CD_APPLET_LEAVE ();
		
		guint i;
		gchar *cService = NULL, *cObjectPath = NULL;
		CDStatusNotifierItem *pItem;
		for (i = 0; pApplications[i] != NULL; i ++)
		{
			cd_message (" + '%s'", pApplications[i]);  // service + path
			if (*pApplications[i] == '\0')
				continue;
			
			gchar *str = strchr (pApplications[i], '/');
			if (str != NULL)  // service + path
			{
				cService = g_strndup (pApplications[i], str - pApplications[i]);
				cObjectPath = str;
			}
			else  // we handle this case too, by supposing the path is the default /StatusNotifierItem
			{
				cService = g_strdup (pApplications[i]);
				cObjectPath = NULL;
			}
			pItem = cd_satus_notifier_create_item (cService, cObjectPath);
			g_free (cService);
			if (! pItem)
				continue;
			cd_debug ("===  => + %s", pItem->cTitle?pItem->cTitle:pItem->cLabel);
		}
		
		g_free (v);
		
		if (myConfig.bCompactMode)
		{
			cd_satus_notifier_reload_compact_mode ();
		}
		else
		{
			cd_satus_notifier_load_icons_from_items ();
		}
	}
	else   // un watcher asocial comme celui d'Ubuntu, on essaye avec l'"indicator-application".
	{
		cd_debug ("=== this watcher is not so friendly, let's try the 'application indicator'");
		myData.bBrokenWatcher = TRUE;
		if (myData.bIASWatched)
			cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}


  //////////////////
 /// Connection ///
//////////////////

static void _on_register_host (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug ("couldn't register to the Notification Watcher (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bSuccess = FALSE;
	}
	
	if (bSuccess)  // we are friend now, let's ask him the current items.
	{
		cd_debug ("=== found a friendly watcher, now ask for the items...");
		// get the current items
		myData.pProxyWatcherProps = cairo_dock_create_new_session_proxy (
			CD_STATUS_NOTIFIER_WATCHER_ADDR,
			CD_STATUS_NOTIFIER_WATCHER_OBJ,
			DBUS_INTERFACE_PROPERTIES);
		dbus_g_proxy_begin_call (myData.pProxyWatcherProps,
			"Get",
			(DBusGProxyCallNotify)_on_get_applications_from_watcher,
			myApplet,
			(GDestroyNotify) NULL,
			G_TYPE_STRING, CD_STATUS_NOTIFIER_WATCHER_IFACE,
			G_TYPE_STRING, "RegisteredStatusNotifierItems",
			G_TYPE_INVALID);
		
		// connect to the signals to keep the list of items up-to-date.
		dbus_g_proxy_add_signal(myData.pProxyWatcher, "StatusNotifierItemRegistered",
			G_TYPE_STRING, G_TYPE_INVALID);  // ServiceRegistered
		dbus_g_proxy_connect_signal(myData.pProxyWatcher, "StatusNotifierItemRegistered",
			G_CALLBACK(on_new_item), myApplet, NULL);
		
		dbus_g_proxy_add_signal(myData.pProxyWatcher, "StatusNotifierItemUnregistered",
			G_TYPE_STRING, G_TYPE_INVALID);  // ServiceUnregistered
		dbus_g_proxy_connect_signal(myData.pProxyWatcher, "StatusNotifierItemUnregistered",
			G_CALLBACK(on_removed_item), myApplet, NULL);
	}
	else  // an asocial watcher like the Ubuntu's one, let's try with the IAS if availeble.
	{
		cd_debug ("=== no friendy watcher, let's try the 'application indicator'");
		myData.bBrokenWatcher = TRUE;
		if (myData.bIASWatched)
			cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}
static void _on_watcher_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	cd_debug ("=== Watcher is on the bus (%d)", bOwned);
	CD_APPLET_ENTER;
	
	if (bOwned)
	{
		myData.bNoWatcher = FALSE;
		// set up a proxy to the Watcher
		myData.pProxyWatcher = cairo_dock_create_new_session_proxy (
			CD_STATUS_NOTIFIER_WATCHER_ADDR,
			CD_STATUS_NOTIFIER_WATCHER_OBJ,
			CD_STATUS_NOTIFIER_WATCHER_IFACE);  // whenever it appears on the bus, we'll get it.
		
		// and register to it.
		cd_debug ("=== register to the it");
		dbus_g_proxy_begin_call (myData.pProxyWatcher, "RegisterStatusNotifierHost",  // RegisterNotificationHost
			(DBusGProxyCallNotify)_on_register_host,
			myApplet,
			(GDestroyNotify) NULL,
			G_TYPE_STRING, myData.cHostName,
			G_TYPE_INVALID);
		if (myConfig.bCompactMode)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (NULL);  // remove the broken image if it was set beforehand, to get the default icon in the items GUI.
	}
	else  // no more watcher on the bus.
	{
		g_object_unref (myData.pProxyWatcher);
		myData.pProxyWatcher = NULL;
		
		g_object_unref (myData.pProxyWatcherProps);
		myData.pProxyWatcherProps = NULL;
		
		g_list_foreach (myData.pItems, (GFunc) cd_free_item, NULL);
		g_list_free (myData.pItems);
		myData.pItems = NULL;
		
		g_hash_table_remove_all (myData.pThemePaths);
		
		// empty the list of items and redraw.
		if (! myConfig.bCompactMode)
		{
			CD_APPLET_DELETE_MY_ICONS_LIST;
		}
		else
		{
			// draw an 'failed' image to not have an empty icon.
			CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/icon-broken.svg");
		}
		myData.bBrokenWatcher = FALSE;
		
		myData.bNoWatcher = TRUE;
		cd_satus_notifier_launch_our_watcher ();
	}
	CD_APPLET_LEAVE ();
}
static void _on_detect_watcher (gboolean bPresent, gpointer data)
{
	cd_debug ("=== Watcher is present: %d", bPresent);
	CD_APPLET_ENTER;
	s_pDetectWatcherCall = NULL;
	// if present, set up proxy.
	if (bPresent)
	{
		_on_watcher_owner_changed (CD_STATUS_NOTIFIER_WATCHER_ADDR, TRUE, NULL);
	}
	else
	{
		myData.bNoWatcher = TRUE;
		cd_satus_notifier_launch_our_watcher ();
		if (myConfig.bCompactMode)  // in compact mode, draw a 'failed' image to not have an empty icon.
			CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/icon-broken.svg");
	}
	
	// watch whenever the Watcher goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_watcher_owner_changed,
		NULL);
	CD_APPLET_LEAVE ();
}

void cd_satus_notifier_detect_watcher (void)
{
	s_pDetectWatcherCall = cairo_dock_dbus_detect_application_async (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_watcher,
		NULL);
}

void cd_satus_notifier_unregister_from_watcher (void)
{
	if (myData.pProxyWatcher != NULL)
	{
		g_object_unref (myData.pProxyWatcher);
		g_object_unref (myData.pProxyWatcherProps);
	}
	
	if (s_pDetectWatcherCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectWatcherCall);
		s_pDetectWatcherCall = NULL;
	}
	cairo_dock_stop_watching_dbus_name_owner (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_watcher_owner_changed);
}

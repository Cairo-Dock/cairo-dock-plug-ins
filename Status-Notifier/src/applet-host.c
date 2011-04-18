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

// our address basename
#define CD_STATUS_NOTIFIER_HOST_ADDR "org.kde.StatusNotifierHost"

// KDE watcher (old names)
//~ #define CD_STATUS_NOTIFIER_WATCHER_ADDR2 "org.kde.NotificationItemWatcher"
//~ #define CD_STATUS_NOTIFIER_WATCHER_OBJ2 "/NotificationItemWatcher"
//~ #define CD_STATUS_NOTIFIER_WATCHER_IFACE2 "org.kde.NotificationItemWatcher"

// KDE watcher
#define CD_STATUS_NOTIFIER_WATCHER_ADDR "org.kde.StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_OBJ "/StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_IFACE "org.kde.StatusNotifierWatcher"

// Ubuntu sort-of-high-level-Watcher (new or old address)
#if (INDICATOR_OLD_NAMES == 0)  // Natty
#define CD_INDICATOR_APPLICATION_ADDR "com.canonical.indicator.application"
#define CD_INDICATOR_APPLICATION_OBJ "/com/canonical/indicator/application/service"
#define CD_INDICATOR_APPLICATION_IFACE "com.canonical.indicator.application.service"
#else
#define CD_INDICATOR_APPLICATION_ADDR "org.ayatana.indicator.application"
#define CD_INDICATOR_APPLICATION_OBJ "/org/ayatana/indicator/application/service"
#define CD_INDICATOR_APPLICATION_IFACE "org.ayatana.indicator.application.service"
#endif

// Ubuntu Indicator Service
#define  CD_INDICATOR_SERVICE_INTERFACE "org.ayatana.indicator.service"
#define  CD_INDICATOR_SERVICE_OBJECT "/org/ayatana/indicator/service"

static DBusGProxyCall *s_pDetectWatcherCall = NULL;
static DBusGProxyCall *s_pDetectIASCall = NULL;

#if (INDICATOR_OLD_NAMES != 0)  // Maverick
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("=== %s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING) (
		gpointer     data1,
		gchar      *arg_1,
		gint        arg_2,
		gchar      *arg_3,
		gchar      *arg_4,
		gchar      *arg_5,
		gchar      *arg_6,
		gchar      *arg_7,
		gpointer     data2);
	register GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 8);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		(char*) g_value_get_string (param_values + 1),
		g_value_get_int (param_values + 2),
		(char*) g_value_get_string (param_values + 3),
		(char*) g_value_get_string (param_values + 4),
		(char*) g_value_get_string (param_values + 5),
		(char*) g_value_get_string (param_values + 6),
		(char*) g_value_get_string (param_values + 7),
		data2);
}
#else  // Natty
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("=== %s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING) (
		gpointer     data1,
		gchar      *arg_1,
		gint        arg_2,
		gchar      *arg_3,
		gchar      *arg_4,
		gchar      *arg_5,
		gchar      *arg_6,
		gchar      *arg_7,
		gchar      *arg_8,
		gpointer     data2);
	register GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 9);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		(char*) g_value_get_string (param_values + 1),
		g_value_get_int (param_values + 2),
		(char*) g_value_get_string (param_values + 3),
		(char*) g_value_get_boxed (param_values + 4),
		(char*) g_value_get_string (param_values + 5),
		(char*) g_value_get_string (param_values + 6),
		(char*) g_value_get_string (param_values + 7),
		(char*) g_value_get_string (param_values + 8),
		data2);
}
#endif

static CDStatusNotifierItem * _cd_satus_notifier_find_item_from_service (const gchar *cService)
{
	g_return_val_if_fail (cService != NULL, NULL);
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->cService && strcmp (pItem->cService, cService) == 0)
			return pItem;
	}
	return NULL;
}

static CDStatusNotifierItem * _cd_satus_notifier_find_item_from_position (int iPosition)
{
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iPosition == iPosition)
			return pItem;
	}
	return NULL;
}


  ///////////////////////////////
 /// Signals add/remove item ///
///////////////////////////////

static inline void _add_new_item (const gchar *cService, const gchar *cObjectPath, int iPosition)
{
	CDStatusNotifierItem *pItem = _cd_satus_notifier_find_item_from_service (cService);
	g_return_if_fail (pItem == NULL);  // on evite d'ajouter 2 fois le meme service.
	
	pItem = cd_satus_notifier_create_item (cService, cObjectPath);
	g_return_if_fail (pItem != NULL);
	
	pItem->iPosition = iPosition;
	if (pItem->cLabel == NULL && pItem->cTitle == NULL)
		pItem->cLabel = g_strdup (pItem->cId);  // cService is often a dbus name like :1.355
	myData.pItems = g_list_prepend (myData.pItems, pItem);
	g_print ("item '%s' appended\n", pItem->cId);
	
	if (pItem->iStatus == CD_STATUS_PASSIVE)  // don't show a passive item.
		return;
	if (myConfig.bCompactMode)
	{
		cd_satus_notifier_reload_compact_mode ();
	}
	else
	{
		Icon *pIcon = cd_satus_notifier_create_icon_for_item (pItem);
		CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
	}
}

static inline void _remove_item (const gchar *cService, int iPosition)
{
	CDStatusNotifierItem *pItem = (cService ? _cd_satus_notifier_find_item_from_service (cService) : _cd_satus_notifier_find_item_from_position (iPosition));
	g_return_if_fail (pItem != NULL);
	
	myData.pItems = g_list_remove (myData.pItems, pItem);
	
	if (pItem->iStatus == CD_STATUS_PASSIVE)  // the item was passive, therefore not visible.
		return;
	if (myConfig.bCompactMode)
	{
		cd_satus_notifier_reload_compact_mode ();
	}
	else
	{
		Icon *pIcon = cd_satus_notifier_get_icon_from_item (pItem);
		CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);
	}
	
	g_print ("=== item %s removed\n", pItem->cTitle?pItem->cTitle:pItem->cLabel);
	cd_free_item (pItem);
}

static void on_new_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("=== %s (%s)\n", __func__, cService);
	
	_add_new_item (cService, NULL, -1);  // on suppose que leur indicator-application ne mettra jamais -1 comme position.
	
	CD_APPLET_LEAVE ();
}

static void on_removed_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("=== %s (%s)\n", __func__, cService);
	
	gchar *str = strchr (cService, '/');
	if (str)
		*str = '\0';
	
	_remove_item (cService, -1);
	
	CD_APPLET_LEAVE ();
}

static void on_new_application (DBusGProxy *proxy_watcher, const gchar *cIconName, gint iPosition, const gchar *cAdress, const gchar *cObjectPath, const gchar *cIconThemePath, const gchar *cLabel, const gchar *cLabelGuide,
#if (INDICATOR_OLD_NAMES == 0)  // Natty
const gchar *cAccessbleDesc,  // WTF is this new param ??
#endif
CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("=== %s (%s, %s, %s, %s, %d)\n", __func__, cAdress, cObjectPath, cIconName, cIconThemePath, iPosition);
	#if (INDICATOR_OLD_NAMES == 0)  // Natty
	g_print ("    %s\n", cAccessbleDesc);
	#endif
	/// position +1 for items placed after this one...
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iPosition >= iPosition)
		{
			pItem->iPosition ++;
			g_print ("===  %s -> %d -> %d\n", pItem->cId, pItem->iPosition-1, pItem->iPosition);
		}
	}
	
	_add_new_item (cAdress, cObjectPath, iPosition);
	
	CD_APPLET_LEAVE ();
}

static void on_removed_application (DBusGProxy *proxy_watcher, gint iPosition, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("=== %s (%d)\n", __func__, iPosition);
	
	_remove_item (NULL, iPosition);
	
	/// position -1 for items placed after this one...
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iPosition >= iPosition)
		{
			pItem->iPosition --;
			g_print ("===  %s -> %d -> %d\n", pItem->cId, pItem->iPosition+1, pItem->iPosition);
		}
	}
	
	CD_APPLET_LEAVE ();
}


  /////////////////
 /// Get Items ///
/////////////////

static void _on_get_applications_from_service (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	g_print ("=== %s ()\n", __func__);
	CD_APPLET_ENTER;
	
	//\______________________ get the applications list from the service.
	GPtrArray *pApplications = NULL;
	GError *erreur = NULL;
	GType g_type_ptrarray = g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_STRING,  // iconname
			G_TYPE_INT,  // position
			G_TYPE_STRING,  // dbusaddress
			DBUS_TYPE_G_OBJECT_PATH,  // dbusobject
			G_TYPE_STRING,  // iconpath
			G_TYPE_STRING,  // label
			G_TYPE_STRING,  // labelguide
			G_TYPE_INVALID));
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		g_type_ptrarray, &pApplications,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		g_print ("=== couldn't get applications in the systray (%s)\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (pApplications == NULL)
		CD_APPLET_LEAVE ();
	
	//\______________________ build each items.
	g_print ("=== got %d aplications\n", pApplications->len);
	guint i, j;
	GValueArray *va;
	GValue *v;
	CDStatusNotifierItem *pItem=NULL;
	//g_print ("=== %d apps in the systray\n", pApplications->len);
	for (i = 0; i < pApplications->len; i ++)
	{
		g_print ("=== %d) %p\n", i, pApplications->pdata[i]);
		va = pApplications->pdata[i];
		if (! va)
			continue;
		
		const gchar *cIconName = NULL;
		gint iPosition = -1;
		const gchar *cAdress = NULL;
		const gchar *cObjectPath = NULL;
		const gchar *cIconThemePath = NULL;
		const gchar *cLabel = NULL;
		const gchar *cLabelGuide = NULL;
		
		v = g_value_array_get_nth (va, 0);
		if (v && G_VALUE_HOLDS_STRING (v))
			cIconName = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 1);
		if (v && G_VALUE_HOLDS_INT (v))
			iPosition = g_value_get_int (v);
		
		v = g_value_array_get_nth (va, 2);
		if (v && G_VALUE_HOLDS_STRING (v))
			cAdress = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 3);
		if (v && G_VALUE_HOLDS_BOXED (v))
			cObjectPath = (gchar*)g_value_get_boxed (v);
		
		v = g_value_array_get_nth (va, 4);
		if (v && G_VALUE_HOLDS_STRING (v))
			cIconThemePath = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 5);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabel = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 6);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabelGuide = g_value_get_string (v);
		
		g_print ("===  + item {%s ; %d ; %s ; %s ; %s ; %s ; %s}\n",
			cIconName,
			iPosition,
			cAdress,
			cObjectPath,
			cIconThemePath,
			cLabel,
			cLabelGuide);
		
		pItem = cd_satus_notifier_create_item (cAdress, cObjectPath);
		if (! pItem)
			continue;
		if (pItem->iPosition == -1)
			pItem->iPosition = iPosition;
		if (pItem->cTitle == NULL && pItem->cLabel == NULL)
			pItem->cLabel = g_strdup (cLabel && *cLabel != '\0' ? cLabel : pItem->cId);
		myData.pItems = g_list_prepend (myData.pItems, pItem);
	}
	
	if (myConfig.bCompactMode)
	{
		cd_satus_notifier_reload_compact_mode ();
	}
	else
	{
		cd_satus_notifier_load_icons_from_items ();
	}
	
	g_ptr_array_free (pApplications, TRUE);
	CD_APPLET_LEAVE ();
}

static void _cd_satus_notifier_get_items_from_ias (void)
{
	if (! myData.bIASWatched)
		return;
	g_print ("=== %s ()\n", __func__);
	
	g_return_if_fail (myData.pProxyIndicatorApplicationService == NULL);
	
	myData.pProxyIndicatorApplicationService = cairo_dock_create_new_session_proxy (
		CD_INDICATOR_APPLICATION_ADDR,
		CD_INDICATOR_APPLICATION_OBJ,
		CD_INDICATOR_APPLICATION_IFACE);
	
	// get the current items
	dbus_g_proxy_begin_call (myData.pProxyIndicatorApplicationService,
		"GetApplications",
		(DBusGProxyCallNotify)_on_get_applications_from_service,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
	
	// connect to the signals to keep the list of items up-to-date.
	#if (INDICATOR_OLD_NAMES != 0)  // Maverick
	dbus_g_object_register_marshaller(_cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING,
			G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	#else  // Natty
	dbus_g_object_register_marshaller(_cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING,
			G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	#endif
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationAdded",
		G_TYPE_STRING,  // iconname
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // dbusaddress
		#if (INDICATOR_OLD_NAMES != 0)  // Maverick
		G_TYPE_STRING,  // dbusobject
		#else  // Natty
		DBUS_TYPE_G_OBJECT_PATH,  // dbusobject
		#endif
		G_TYPE_STRING,  // iconpath
		G_TYPE_STRING,  // label
		G_TYPE_STRING,  // labelguide
		#if (INDICATOR_OLD_NAMES == 0)  // Natty
		G_TYPE_STRING,  // accessibledesc
		#endif
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationAdded",
		G_CALLBACK(on_new_application), myApplet, NULL);
	
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationRemoved",
		G_TYPE_INT,  // position
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationRemoved",
		G_CALLBACK(on_removed_application), myApplet, NULL);
}

static void _on_get_applications_from_watcher (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	g_print ("=== %s ()\n", __func__);
	CD_APPLET_ENTER;
	
	gchar **pApplications = NULL;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_STRV, &pApplications,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		g_print ("=== couldn't get applications from the watcher (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bSuccess = FALSE;
	}
	
	if (bSuccess)
	{
		if (pApplications == NULL)
			CD_APPLET_LEAVE ();
		guint i;
		CDStatusNotifierItem *pItem;
		for (i = 0; pApplications[i] != NULL; i ++)
		{
			pItem = cd_satus_notifier_create_item (pApplications[i], NULL);
			if (! pItem)
				continue;
			g_print ("===  => + %s\n", pItem->cTitle);
			myData.pItems = g_list_prepend (myData.pItems, pItem);
		}
		
		g_strfreev (pApplications);
		
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
		g_print ("=== this watcher is not so friendly, let's try the 'application indicator'\n");
		myData.bBrokenWatcher = TRUE;
		if (myData.bIASWatched)
			_cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}


  /////////////////////////////////
 /// connection to the Watcher ///
/////////////////////////////////

static void _on_register_host (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	g_print ("=== %s ()\n", __func__);
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		g_print ("couldn't register to the Notification Watcher (%s)\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bSuccess = FALSE;
	}
	
	if (bSuccess)  // we are friend now, let's ask him the current items.
	{
		g_print ("=== found a friendly watcher, now ask for the items...\n");
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
		dbus_g_proxy_add_signal(myData.pProxyWatcher, "ServiceRegistered",
			G_TYPE_STRING, G_TYPE_INVALID);  // StatusNotifierItemRegistered
		dbus_g_proxy_connect_signal(myData.pProxyWatcher, "ServiceRegistered",
			G_CALLBACK(on_new_item), myApplet, NULL);
		
		dbus_g_proxy_add_signal(myData.pProxyWatcher, "ServiceUnregistered",
			G_TYPE_STRING, G_TYPE_INVALID);  // StatusNotifierItemUnregistered
		dbus_g_proxy_connect_signal(myData.pProxyWatcher, "ServiceUnregistered",
			G_CALLBACK(on_removed_item), myApplet, NULL);
	}
	else  // an asocial watcher like the Ubuntu's one, let's try with the IAS if availeble.
	{
		g_print ("=== no friendy watcher, let's try the 'application indicator'\n");
		myData.bBrokenWatcher = TRUE;
		if (myData.bIASWatched)
			_cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}
static void _on_watcher_owner_changed (gboolean bOwned, gpointer data)
{
	g_print ("=== Watcher is on the bus (%d)\n", bOwned);
	
	if (bOwned)
	{
		// set up a proxy to the Watcher
		myData.pProxyWatcher = cairo_dock_create_new_session_proxy (
			CD_STATUS_NOTIFIER_WATCHER_ADDR,
			CD_STATUS_NOTIFIER_WATCHER_OBJ,
			CD_STATUS_NOTIFIER_WATCHER_IFACE);  // whenever it appears on the bus, we'll get it.
		
		// and register to it.
		g_print ("=== register to the it\n");
		dbus_g_proxy_begin_call (myData.pProxyWatcher, "RegisterNotificationHost",
			(DBusGProxyCallNotify)_on_register_host,
			myApplet,
			(GDestroyNotify) NULL,
			G_TYPE_STRING, myData.cHostName,
			G_TYPE_INVALID);
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
	}
}
static void _on_detect_watcher (gboolean bPresent, gpointer data)
{
	g_print ("=== Watcher is present: %d\n", bPresent);
	s_pDetectWatcherCall = NULL;
	// if present, set up proxy.
	if (bPresent)
	{
		_on_watcher_owner_changed (TRUE, NULL);
	}
	else if (myConfig.bCompactMode)  // in compact mode, draw an 'failed' image to not have an empty icon.
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/icon-broken.svg");
	}
	
	// watch whenever the Watcher goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_watcher_owner_changed,
		NULL);
}

  /////////////////////////////
 /// connection to the IAS ///
/////////////////////////////

static void _on_start_service (DBusGProxy *proxy, guint status, GError *error, gpointer user_data)
{
	// if service has not started, then we'll assume we don't need it (eg.: KDE)
	if (error != NULL)  // couldn't start the service.
	{
		g_print ("=== Unable to start the indicator service (%s), assuming we don't need it\n", error->message);
		return;
	}
	if (status != DBUS_START_REPLY_SUCCESS && status != DBUS_START_REPLY_ALREADY_RUNNING)  // started but wrong status.
	{
		g_print ("=== Unable to start the indicator service (got status %d), assuming we don't need it\n", status);
		return;
	}
	g_print ("=== Indicator Service has started\n");
}
static void _on_watch_service (DBusGProxy *proxy, DBusGProxyCall *call, gpointer data)
{
	GError *error = NULL;
	guint service_api_version=0, this_service_version=0;
	dbus_g_proxy_end_call (proxy, call, &error,
		G_TYPE_UINT, &service_api_version,
		G_TYPE_UINT, &this_service_version,
		G_TYPE_INVALID);
	g_print ("=== got indicator service (API: %d, service: %d, broken watcher: %d)\n", service_api_version, this_service_version, myData.bBrokenWatcher);
	
	if (service_api_version > 0)  /// shouldn't the 2 versions be equal ?...
	{
		myData.bIASWatched = TRUE;  // now we're friend with the IAS
		
		if (myData.bBrokenWatcher)  // if the watcher is not our friend, let's ask the IAS the current items.
		{
			_cd_satus_notifier_get_items_from_ias ();
		}
	}
}
static void _on_ias_owner_changed (gboolean bOwned, gpointer data)
{
	g_print ("=== Indicator Applications Service is on the bus (%d)\n", bOwned);
	
	if (bOwned)
	{
		// set up a proxy to the Service
		myData.pProxyIndicatorService = cairo_dock_create_new_session_proxy (
			CD_INDICATOR_APPLICATION_ADDR,
			CD_INDICATOR_SERVICE_OBJECT,
			CD_INDICATOR_SERVICE_INTERFACE);
		
		// and watch it.
		g_print ("=== watch it\n");
		dbus_g_proxy_begin_call (myData.pProxyIndicatorService,
			"Watch",
			(DBusGProxyCallNotify)_on_watch_service,
			myApplet,
			(GDestroyNotify) NULL,
			G_TYPE_INVALID);
	}
	else  // no more IAS on the bus.
	{
		g_object_unref (myData.pProxyIndicatorService);
		myData.pProxyIndicatorService = NULL;
		
		g_object_unref (myData.pProxyIndicatorApplicationService);
		myData.pProxyIndicatorApplicationService = NULL;
		
		myData.bIASWatched = FALSE;
	}
}
static void _on_detect_ias (gboolean bPresent, gpointer data)
{
	g_print ("=== Indicator Applications Service is present: %d\n", bPresent);
	s_pDetectIASCall = NULL;
	// if present, set up proxy, else try to start the service.
	if (bPresent)
	{
		_on_ias_owner_changed (TRUE, NULL);
	}
	else  // not present, maybe the service is not started => try starting it.
	{
		g_print ("=== try to start the Indicator Service...\n");
		DBusGProxy *dbus_proxy = cairo_dock_get_main_proxy ();
		org_freedesktop_DBus_start_service_by_name_async (dbus_proxy,
			CD_INDICATOR_APPLICATION_ADDR,
			0,
			_on_start_service,
			myApplet);
	}
	// watch whenever the Service goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_ias_owner_changed,
		NULL);
}

/* watch Watcher + IAS ->
Watcher ON -> make proxy -> register -> ok => Get items -> ok => load items
                                                        -> ko => broken; if IAS watched: Get Applications
                                     -> ko => broken; if IAS watched: Get Applications
IAS ON -> make proxy -> watch -> ok => watched; if broken: Get Applications 
                              -> ko => X
*/

void cd_satus_notifier_launch_service (void)
{
	if (myData.pThemePaths == NULL)
		myData.pThemePaths = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);  // (path dir,ref count).
	
	// Register our service name on the bus.
	pid_t pid = getpid ();
	myData.cHostName = g_strdup_printf (CD_STATUS_NOTIFIER_HOST_ADDR"-%d", pid);
	//g_print ("=== registering name '%s' on the bus ...\n", myData.cHostName);
	cairo_dock_register_service_name (myData.cHostName);
	
	// see if a Watcher and/or an Indicator Application Service (IAS) is on the bus.
	s_pDetectWatcherCall = cairo_dock_dbus_detect_application_async (CD_STATUS_NOTIFIER_WATCHER_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_watcher,
		NULL);
	s_pDetectIASCall = cairo_dock_dbus_detect_application_async (CD_INDICATOR_APPLICATION_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_ias,
		NULL);
}


void cd_satus_notifier_stop_service (void)
{
	if (myData.pProxyWatcher != NULL)
	{
		g_object_unref (myData.pProxyWatcher);
		g_object_unref (myData.pProxyWatcherProps);
	}
	if (myData.pProxyIndicatorApplicationService != NULL)
	{
		g_object_unref (myData.pProxyIndicatorApplicationService);
		g_object_unref (myData.pProxyIndicatorService);
	}
	
	if (s_pDetectWatcherCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectWatcherCall);
	}
	if (s_pDetectIASCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectIASCall);
	}
	
	g_list_foreach (myData.pItems, (GFunc) cd_free_item, NULL);
	g_list_free (myData.pItems);
	
	if (! myConfig.bCompactMode)
		CD_APPLET_DELETE_MY_ICONS_LIST;
	
	g_hash_table_destroy (myData.pThemePaths);
}


  ///////////////////
 /// THEMES PATH ///
///////////////////

void cd_satus_notifier_add_theme_path (const gchar * cThemePath)
{
	g_return_if_fail (cThemePath != NULL);
	int ref = GPOINTER_TO_INT (g_hash_table_lookup (myData.pThemePaths, cThemePath));  // 0 si le theme n'est pas dans la table.
	ref ++;  // on incremente la reference.
	g_hash_table_insert (myData.pThemePaths, g_strdup (cThemePath), GINT_TO_POINTER (ref));  // et on la met a jour dans la table.
	
	if (ref == 1)  // premiere fois qu'on voit ce chemin.
		gtk_icon_theme_append_search_path (gtk_icon_theme_get_default(), cThemePath);  // append car ce sont des icones par defaut.
}

void cd_satus_notifier_remove_theme_path (const gchar * cThemePath)
{
	g_return_if_fail (cThemePath != NULL);
	int ref = GPOINTER_TO_INT (g_hash_table_lookup (myData.pThemePaths, cThemePath));
	if (ref == 0)  // pas dans la table, rien a faire (ne devrait pas arriver).
		return;
	
	if (ref == 1)  // derniere reference.
	{
		g_hash_table_remove (myData.pThemePaths, cThemePath);  // on le supprime de la table.
		
		GtkIconTheme *pIconTheme = gtk_icon_theme_get_default();  // et du theme.
		gchar **paths = NULL;
		gint iNbPaths = 0;
		gtk_icon_theme_get_search_path (pIconTheme, &paths, &iNbPaths);
	
		int i;
		for (i = 0; i < iNbPaths; i++)  // on cherche sa position dans le tableau.
		{
			if (strcmp (paths[i], cThemePath))
				break;
		}
		if (i < iNbPaths)  // trouve
		{
			g_free (paths[i]);
			for (i = i+1; i < iNbPaths; i++)  // on decale tous les suivants vers l'arriere.
			{
				paths[i-1] = paths[i];
			}
			paths[i-1] = NULL;
			gtk_icon_theme_set_search_path (pIconTheme, (const gchar **)paths, iNbPaths - 1);
		}
		
		g_strfreev (paths);
	}
	else  // on decremente la reference.
	{
		ref --;
		g_hash_table_insert (myData.pThemePaths, g_strdup (cThemePath), GINT_TO_POINTER (ref));  // et on la met a jour dans la table.
	}
}


void cd_satus_notifier_load_icons_from_items (void)
{
	GList *pIcons = NULL;
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iStatus != CD_STATUS_PASSIVE)
		{
			Icon *pIcon = cd_satus_notifier_create_icon_for_item (pItem);
			if (pIcon)
				pIcons = g_list_prepend (pIcons, pIcon);
		}
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (pIcons, NULL, "Slide", NULL);
}

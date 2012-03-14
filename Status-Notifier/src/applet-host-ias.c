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
#include "applet-host-ias.h"

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

#define CD_INDICATOR_APPLICATION_ITEM_OBJ "/org/ayatana/NotificationItem"

static DBusGProxyCall *s_pDetectIASCall = NULL;

#if (INDICATOR_OLD_NAMES != 0)  // Maverick
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//cd_debug ("=== %s ()\n", __func__);
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
#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING_STRING (
#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING (
#else
static void _cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING (
#endif
	GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//cd_debug ("=== %s ()\n", __func__);
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	typedef void (*GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING_STRING) (
	#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	typedef void (*GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING) (
	#else
	typedef void (*GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING) (
	#endif
		gpointer     data1,
		gchar      *arg_1,
		gint        arg_2,
		gchar      *arg_3,
		gchar      *arg_4,
		gchar      *arg_5,
		gchar      *arg_6,
		gchar      *arg_7,
		gchar      *arg_8,
		#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
		gchar      *arg_9,
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		gchar      *arg_10,
		#endif
		#endif
		gpointer     data2);
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	register GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING_STRING callback;
	#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	register GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING callback;
	#else
	register GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING callback;
	#endif
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	// return_value est NULL ici, car la callback ne renvoit rien.
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	g_return_if_fail (n_param_values == 11);
	#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	g_return_if_fail (n_param_values == 10);
	#else
	g_return_if_fail (n_param_values == 9);
	#endif

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
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	callback = (GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING_STRING)
	#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	callback = (GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING)
	#else
	callback = (GMarshalFunc_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING)
	#endif
		(marshal_data ? marshal_data : cc->callback);

	callback (data1,
		(char*) g_value_get_string (param_values + 1),
		g_value_get_int (param_values + 2),
		(char*) g_value_get_string (param_values + 3),
		(char*) g_value_get_boxed (param_values + 4),
		(char*) g_value_get_string (param_values + 5),
		(char*) g_value_get_string (param_values + 6),
		(char*) g_value_get_string (param_values + 7),
		(char*) g_value_get_string (param_values + 8),
		#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
		(char*) g_value_get_string (param_values + 9),
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		(char*) g_value_get_string (param_values + 10),
		#endif
		#endif
		data2);
}
#endif

  ///////////////
 /// Signals ///
///////////////

static void on_new_application (DBusGProxy *proxy_watcher, const gchar *cIconName, gint iPosition, const gchar *cAddress, const gchar *cObjectPath, const gchar *cIconThemePath, const gchar *cLabel, const gchar *cLabelGuide,
#if (INDICATOR_OLD_NAMES == 0)  // Natty
const gchar *cAccessbleDesc,  // WTF is this new param ??
#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
const gchar *cHint,  // <irony> is this a hint to work around a clumsy API ? </irony>
#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
const gchar *cTitle,
#endif
#endif
#endif
CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%s, %s, %s, %s, %d)", __func__, cAddress, cObjectPath, cIconName, cIconThemePath, iPosition);
	#if (INDICATOR_OLD_NAMES == 0)  // Natty
	cd_debug ("    %s", cAccessbleDesc);
	#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	cd_debug ("    %s", cHint);
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	cd_debug ("    %s", cTitle);
	#endif
	#endif
	#endif
	
	// position +1 for items placed after this one.
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iPosition >= iPosition)
		{
			pItem->iPosition ++;
			cd_debug ("===  %s -> %d -> %d", pItem->cId, pItem->iPosition-1, pItem->iPosition);
		}
	}
	
	cd_satus_notifier_add_new_item_with_default (cAddress, cObjectPath, iPosition,
		#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
		cIconName ? cIconName : cHint,
		#else
		cIconName,
		#endif
		cIconThemePath,
		#if (INDICATOR_OLD_NAMES == 0)
		cAccessbleDesc && *cAccessbleDesc != '\0' ? cAccessbleDesc :
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		cTitle && *cTitle != '\0' ? cTitle :
		#endif
		#endif
		cLabel
		);
	
	CD_APPLET_LEAVE ();
}

static void on_removed_application (DBusGProxy *proxy_watcher, gint iPosition, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d)", __func__, iPosition);
	
	cd_satus_notifier_remove_item (NULL, iPosition);
	
	// position -1 for items placed after this one.
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iPosition >= iPosition)
		{
			pItem->iPosition --;
			cd_debug ("===  %s -> %d -> %d", pItem->cId, pItem->iPosition+1, pItem->iPosition);
		}
	}
	
	CD_APPLET_LEAVE ();
}


  /////////////////
 /// Get Items ///
/////////////////

static void _on_get_applications_from_service (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	
	//\______________________ get the applications list from the service.
	GPtrArray *pApplications = NULL;
	GError *erreur = NULL;
	GType g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_STRING,  // iconname
			G_TYPE_INT,  // position
			G_TYPE_STRING,  // dbusaddress
			DBUS_TYPE_G_OBJECT_PATH,  // dbusobject
			G_TYPE_STRING,  // iconpath
			G_TYPE_STRING,  // label
			G_TYPE_STRING,  // labelguide
			#if (INDICATOR_OLD_NAMES == 0)  // Natty
			G_TYPE_STRING,  // accessibledesc
			#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
			G_TYPE_STRING,  // hint
			#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
			G_TYPE_STRING,  // title
			#endif
			#endif
			#endif
			G_TYPE_INVALID));
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		g_type_ptrarray, &pApplications,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug ("=== couldn't get applications in the systray (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (pApplications == NULL)
		CD_APPLET_LEAVE ();
	
	//\______________________ build each items.
	cd_debug ("=== got %d applications", pApplications->len);
	guint i, j;
	GValueArray *va;
	GValue *v;
	CDStatusNotifierItem *pItem = NULL;
	//cd_debug ("=== %d apps in the systray", pApplications->len);
	for (i = 0; i < pApplications->len; i ++)
	{
		cd_debug ("=== %d) %p", i, pApplications->pdata[i]);
		va = pApplications->pdata[i];
		if (! va)
			continue;
		
		const gchar *cIconName = NULL;
		gint iPosition = -1;
		const gchar *cAddress = NULL;
		const gchar *cObjectPath = NULL;
		const gchar *cIconThemePath = NULL;
		const gchar *cLabel = NULL;
		const gchar *cLabelGuide = NULL;
		const gchar *cAccessibleDesc = NULL; // natty
		// const gchar *cHint = NULL; // oneiric
		const gchar *cTitle = NULL;  // precise
		
		v = g_value_array_get_nth (va, 0);
		if (v && G_VALUE_HOLDS_STRING (v))
			cIconName = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 1);
		if (v && G_VALUE_HOLDS_INT (v))
			iPosition = g_value_get_int (v);
		
		v = g_value_array_get_nth (va, 2);
		if (v && G_VALUE_HOLDS_STRING (v))
			cAddress = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 3);
		if (v && G_VALUE_HOLDS (v, DBUS_TYPE_G_OBJECT_PATH))
			cObjectPath = (gchar*)g_value_get_boxed (v);

		/*cd_debug ("=== cObjectPath : %s", cObjectPath);
		if (cObjectPath != NULL && strncmp (cObjectPath, CD_INDICATOR_APPLICATION_ITEM_OBJ, strlen (CD_INDICATOR_APPLICATION_ITEM_OBJ)) == 0)
		{
			gchar *str = strrchr (cObjectPath, '/');  // I think this is because this path is actually the menu path, and fortunately it's just under the item object's path.
			if (str)
				*str = '\0';
		}*/ // => we will do that in cd_satus_notifier_create_item because this function is also called when a new application is added => host.c

		v = g_value_array_get_nth (va, 4);
		if (v && G_VALUE_HOLDS_STRING (v))
			cIconThemePath = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 5);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabel = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 6);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabelGuide = g_value_get_string (v);
		
		#if (INDICATOR_OLD_NAMES == 0)
		v = g_value_array_get_nth (va, 7);
		if (v && G_VALUE_HOLDS_STRING (v))
			cAccessibleDesc = g_value_get_string (v);
		
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		v = g_value_array_get_nth (va, 9);
		if (v && G_VALUE_HOLDS_STRING (v))
			cTitle = g_value_get_string (v);
		#endif
		#endif
		
		cd_debug ("===  + item {%s ; %d ; %s ; %s ; %s ; %s ; %s ; %s ; %s}",
			cIconName,
			iPosition,
			cAddress,
			cObjectPath,
			cIconThemePath,
			cLabel,
			cLabelGuide,
			cAccessibleDesc,
			cTitle);

		// position +1 for items placed after this one.
		GList *it;
		for (it = myData.pItems; it != NULL; it = it->next)
		{
			pItem = it->data;
			if (strcmp (cAddress, pItem->cService) == 0)
			{
				cd_warning ("Duplicated item: %s (%s)", cIconName, cAddress);
				return;
			}
		}

		pItem = cd_satus_notifier_create_item (cAddress, cObjectPath);
		if (! pItem)
			continue;
		if (pItem->iPosition == -1)
			pItem->iPosition = iPosition;
		if (pItem->cTitle == NULL && pItem->cLabel == NULL && pItem->cAccessibleDesc == NULL)
			pItem->cLabel = g_strdup (cAccessibleDesc && *cAccessibleDesc != '\0' ? cAccessibleDesc :
			                          cLabel && *cLabel != '\0' ? cLabel :
			                          cTitle && *cTitle != '\0' ? cTitle :
			                          NULL);
			                          // pItem->cId); // maybe better to not display cId, e.g: nm-applet ; dropbox-xxxx ; etc.
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

void cd_satus_notifier_get_items_from_ias (void)
{
	if (! myData.bIASWatched)
		return;
	cd_debug ("=== %s ()", __func__);
	
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
	dbus_g_object_register_marshaller(
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	_cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING_STRING,
	#elif (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	_cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING_STRING,
	#else
	_cd_cclosure_marshal_VOID__STRING_INT_STRING_STRING_STRING_STRING_STRING_STRING,
	#endif
			G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING,
			DBUS_TYPE_G_OBJECT_PATH,  // dbusobject
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
			G_TYPE_STRING,
			#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
			G_TYPE_STRING,
			#endif
			#endif
			G_TYPE_INVALID);
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
		#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
		G_TYPE_STRING, // hint => only with indicator-0.4 (Oneiric)
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		G_TYPE_STRING, // title => only with indicator-0.4.90 (Precise)
		#endif
		#endif
		#endif
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationAdded",
		G_CALLBACK(on_new_application), myApplet, NULL);
	
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationRemoved",
		G_TYPE_INT,  // position
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationRemoved",
		G_CALLBACK(on_removed_application), myApplet, NULL);
	// TODO? ApplicationIconChanged, ApplicationIconThemePathChanged, ApplicationLabelChanged, ApplicationTitleChanged
}

  //////////////////
 /// Connection ///
//////////////////

static void _on_start_service (DBusGProxy *proxy, guint status, GError *error, gpointer data)
{
	// if service has not started, then we'll assume we don't need it (eg.: KDE)
	if (status != DBUS_START_REPLY_SUCCESS && status != DBUS_START_REPLY_ALREADY_RUNNING)  // service is not started.
	{
		if (error != NULL)  // couldn't start the service
			cd_debug ("=== Unable to start the indicator service (%s), assuming we don't need it", error->message);
		else
			cd_debug ("=== Unable to start the indicator service (got status %d), assuming we don't need it", status);
		myData.bNoIAS = TRUE;
		cd_satus_notifier_launch_our_watcher ();
		return;
	}
	cd_debug ("=== Indicator Service has started");
}
static void _on_watch_service (DBusGProxy *proxy, DBusGProxyCall *call, gpointer data)
{
	CD_APPLET_ENTER;
	GError *error = NULL;
	guint service_api_version=0, this_service_version=0;
	dbus_g_proxy_end_call (proxy, call, &error,
		G_TYPE_UINT, &service_api_version,
		G_TYPE_UINT, &this_service_version,
		G_TYPE_INVALID);
	cd_debug ("=== got indicator service (API: %d, service: %d, broken watcher: %d)", service_api_version, this_service_version, myData.bBrokenWatcher);
	
	if (service_api_version > 0)  /// shouldn't the 2 versions be equal ?...
	{
		myData.bIASWatched = TRUE;  // now we're friend with the IAS
		
		if (myData.bBrokenWatcher)  // if the watcher is not our friend, let's ask the IAS the current items.
		{
			cd_satus_notifier_get_items_from_ias ();
		}
	}
	CD_APPLET_LEAVE ();
}
static void _on_ias_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("=== Indicator Applications Service is on the bus (%d)", bOwned);
	
	if (bOwned)
	{
		myData.bNoIAS = FALSE;
		// set up a proxy to the Service
		myData.pProxyIndicatorService = cairo_dock_create_new_session_proxy (
			CD_INDICATOR_APPLICATION_ADDR,
			CD_INDICATOR_SERVICE_OBJECT,
			CD_INDICATOR_SERVICE_INTERFACE);
		
		// and watch it.
		cd_debug ("=== watch it");
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
		
		myData.bNoIAS = TRUE;
		cd_satus_notifier_launch_our_watcher ();
	}
	CD_APPLET_LEAVE ();
}
static void _on_detect_ias (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("=== Indicator Applications Service is present: %d", bPresent);
	s_pDetectIASCall = NULL;
	// if present, set up proxy, else try to start the service.
	if (bPresent)
	{
		_on_ias_owner_changed (CD_INDICATOR_APPLICATION_ADDR, TRUE, NULL);
	}
	else  // not present, maybe the service is not started => try starting it.
	{
		cd_debug ("=== try to start the Indicator Service...");
		DBusGProxy *dbus_proxy = cairo_dock_get_main_proxy ();
		org_freedesktop_DBus_start_service_by_name_async (dbus_proxy,
			CD_INDICATOR_APPLICATION_ADDR,
			0,
			_on_start_service,
			myApplet);
	}
	// watch whenever the Service goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_INDICATOR_APPLICATION_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_ias_owner_changed,
		NULL);
	CD_APPLET_LEAVE ();
}


void cd_satus_notifier_detect_ias (void)
{
	s_pDetectIASCall = cairo_dock_dbus_detect_application_async (CD_INDICATOR_APPLICATION_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_ias,
		NULL);
}


void cd_satus_notifier_unregister_from_ias (void)
{
	if (myData.pProxyIndicatorApplicationService != NULL)
	{
		g_object_unref (myData.pProxyIndicatorApplicationService);
		g_object_unref (myData.pProxyIndicatorService);
	}
	
	if (s_pDetectIASCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectIASCall);
		s_pDetectIASCall = NULL;
	}
	
	cairo_dock_stop_watching_dbus_name_owner (CD_INDICATOR_APPLICATION_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_ias_owner_changed);
}

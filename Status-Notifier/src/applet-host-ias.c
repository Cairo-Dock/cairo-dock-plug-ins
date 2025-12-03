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

#define CD_INDICATOR_APPLICATION_ADDR "org.ayatana.indicator.application"
#define CD_INDICATOR_APPLICATION_OBJ "/org/ayatana/indicator/application/service"
#define CD_INDICATOR_APPLICATION_IFACE "org.ayatana.indicator.application.service"

// Ubuntu Indicator Service
#define  CD_INDICATOR_SERVICE_INTERFACE "org.ayatana.indicator.service"
#define  CD_INDICATOR_SERVICE_OBJECT "/org/ayatana/indicator/service"


static DBusGProxyCall *s_pDetectIASCall = NULL;

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
	//cd_debug ("=== %s ()", __func__);
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


static void _cd_cclosure_marshal_VOID__INT_STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//cd_debug ("=== %s ()", __func__);
	typedef void (*GMarshalFunc_VOID__INT_STRING_STRING) (
		gpointer     data1,
		gint         arg_1,
		gchar       *arg_2,
		gchar       *arg_3,
		gpointer     data2);
	register GMarshalFunc_VOID__INT_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 4);  // return_value est NULL ici, car la callback ne renvoit rien.

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
	callback = (GMarshalFunc_VOID__INT_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_value_get_int (param_values + 1),
		(char*) g_value_get_string (param_values + 2),
		(char*) g_value_get_string (param_values + 3),
		data2);
}

static void _cd_cclosure_marshal_VOID__INT_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//cd_debug ("=== %s ()", __func__);
	typedef void (*GMarshalFunc_VOID__INT_STRING) (
		gpointer     data1,
		gint         arg_1,
		gchar       *arg_2,
		gpointer     data2);
	register GMarshalFunc_VOID__INT_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

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
	callback = (GMarshalFunc_VOID__INT_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_value_get_int (param_values + 1),
		(char*) g_value_get_string (param_values + 2),
		data2);
}

  ///////////////
 /// Signals ///
///////////////

static void on_new_application (DBusGProxy *proxy_watcher, const gchar *cIconName, gint iPosition, const gchar *cAddress, const gchar *cObjectPath, const gchar *cIconThemePath, const gchar *cLabel, const gchar *cLabelGuide,
const gchar *cAccessbleDesc,  // WTF is this new param ??
#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
const gchar *cHint,  // <irony> is this a hint to work around a clumsy API ? </irony>
#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
const gchar *cTitle,
#endif
#endif
GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%s, %s, %s, %s, %d)", __func__, cAddress, cObjectPath, cIconName, cIconThemePath, iPosition);
	cd_debug ("    %s", cAccessbleDesc);
	#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
	cd_debug ("    %s", cHint);
	#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
	cd_debug ("    %s", cTitle);
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
		cAccessbleDesc && *cAccessbleDesc != '\0' ? cAccessbleDesc :
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		cTitle && *cTitle != '\0' ? cTitle :
		#endif
		cLabel
		);
	
	CD_APPLET_LEAVE ();
}

static void on_removed_application (DBusGProxy *proxy_watcher, gint iPosition, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d)", __func__, iPosition);
	
	cd_satus_notifier_remove_item (NULL, NULL, iPosition);
	
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

static void on_application_icon_changed (DBusGProxy *proxy_watcher, gint iPosition, const gchar *cIconName, const gchar *cIconDesc, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d, %s, %s)", __func__, iPosition, cIconName, cIconDesc);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
	g_return_if_fail (pItem != NULL);
	
	if (g_strcmp0 (pItem->cIconName, cIconName) == 0)  // discard useless updates (skype...)
		return;
	g_free (pItem->cIconName);
	pItem->cIconName = g_strdup (cIconName);
	g_free (pItem->cAccessibleDesc);
	pItem->cAccessibleDesc = g_strdup (cIconDesc);
	
	if (pItem->iStatus != CD_STATUS_NEEDS_ATTENTION)
	{
		cd_satus_notifier_update_item_image (pItem);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_application_icon_theme_path_changed (DBusGProxy *proxy_watcher, gint iPosition, const gchar *cIconThemePath, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d, %s)", __func__, iPosition, cIconThemePath);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
	g_return_if_fail (pItem != NULL);
	
	if (g_strcmp0 (cIconThemePath, pItem->cIconThemePath) != 0)
	{
		if (pItem->cIconThemePath != NULL)  // if the item previously provided a theme, remove it first.
			cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
		g_free (pItem->cIconThemePath);
		pItem->cIconThemePath = g_strdup (cIconThemePath);
		
		cd_satus_notifier_add_theme_path (cIconThemePath);  // and add the new one.
		
		if (pItem->cIconName != NULL)
		{
			cd_satus_notifier_update_item_image (pItem);
		}
	}
	CD_APPLET_LEAVE ();
}

static void on_application_label_changed (DBusGProxy *proxy_watcher, gint iPosition, const gchar *cLabel, const gchar *cLabelGuide, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d, %s, %s)", __func__, iPosition, cLabel, cLabelGuide);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
	g_return_if_fail (pItem != NULL);
	
	g_free (pItem->cLabel);
	pItem->cLabel = g_strdup (cLabel);
	g_free (pItem->cLabelGuide);
	pItem->cLabelGuide = g_strdup (cLabelGuide);
	
	CD_APPLET_LEAVE ();
}

static void on_application_title_changed (DBusGProxy *proxy_watcher, gint iPosition, const gchar *cTitle, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%d, %s)", __func__, iPosition, cTitle);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
	g_return_if_fail (pItem != NULL);
	
	if (cTitle != NULL)
	{
		g_free (pItem->cTitle);
		pItem->cTitle = g_strdup (cTitle);
	}
	
	CD_APPLET_LEAVE ();
}

  /////////////////
 /// Get Items ///
/////////////////

static void _on_get_applications_from_service (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
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
			G_TYPE_STRING,  // accessibledesc
			#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
			G_TYPE_STRING,  // hint
			#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
			G_TYPE_STRING,  // title
			#endif
			#endif
			G_TYPE_INVALID));
	dbus_g_proxy_end_call (proxy,
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
	guint i;
	GValueArray *va;
	GValue *v;
	CDStatusNotifierItem *pItem = NULL;
	//cd_debug ("=== %d apps in the systray", pApplications->len);
	for (i = 0; i < pApplications->len; i ++)
	{
		// get the properties of the item
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
		
		v = g_value_array_get_nth (va, 0);  // GValueArray is deprecated from 2.32, yet it's so convenient to map the g_type_ptrarray type ...
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

		v = g_value_array_get_nth (va, 4);
		if (v && G_VALUE_HOLDS_STRING (v))
			cIconThemePath = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 5);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabel = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 6);
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabelGuide = g_value_get_string (v);
		
		v = g_value_array_get_nth (va, 7);
		if (v && G_VALUE_HOLDS_STRING (v))
			cAccessibleDesc = g_value_get_string (v);
		
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		v = g_value_array_get_nth (va, 9);
		if (v && G_VALUE_HOLDS_STRING (v))
			cTitle = g_value_get_string (v);
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
		
		if (!cAddress)
			continue;
		
		// ensure we're not duplicating an existing item (this should never happen if the service is ok, but since it doesn't depend on us, let's be careful).
		GList *it;
		for (it = myData.pItems; it != NULL; it = it->next)
		{
			pItem = it->data;
			if (strcmp (cAddress, pItem->cService) == 0)  // pItem->cService is never NULL
			{
				cd_warning ("Duplicated item: %s (%s)", cIconName, cAddress);
				return;
			}
			if (iPosition != -1 && pItem->iPosition == iPosition)
			{
				cd_warning ("Possible duplicated item: %s/%s/%d , %s/%s/%d)", cIconName, cAddress, iPosition, pItem->cIconName, pItem->cService, pItem->iPosition);
			}
		}
		
		// make a new item based on these properties.
		pItem = g_new0 (CDStatusNotifierItem, 1);
		pItem->cService = g_strdup (cAddress);
		// set default values
		pItem->iPosition = iPosition;
		pItem->cLabel = g_strdup (cAccessibleDesc && *cAccessibleDesc != '\0' ? cAccessibleDesc :
								  cLabel && *cLabel != '\0' ? cLabel :
								  cTitle && *cTitle != '\0' ? cTitle :
								  NULL);  // don't use cId as a fallback, because it often has cryptic names (nm-applet ; dropbox-xxxx). If the appli doesn't provide a title, it's its fault.
		
		cd_satus_notifier_create_item (pItem, cObjectPath);
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
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationAdded",
		G_TYPE_STRING,  // iconname
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // dbusaddress
		DBUS_TYPE_G_OBJECT_PATH,  // dbusobject
		G_TYPE_STRING,  // iconpath
		G_TYPE_STRING,  // label
		G_TYPE_STRING,  // labelguide
		G_TYPE_STRING,  // accessibledesc
		#if (INDICATOR_APPLICATIONADDED_HAS_HINT == 1)
		G_TYPE_STRING, // hint => only with indicator-0.4 (Oneiric)
		#if (INDICATOR_APPLICATIONADDED_HAS_TITLE == 1)
		G_TYPE_STRING, // title => only with indicator-0.4.90 (Precise)
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
	
	// we add the following signals because some program don't support the StatusNotifier API (skype, once again...) but only the IAS one.
	dbus_g_object_register_marshaller(_cd_cclosure_marshal_VOID__INT_STRING_STRING,
			G_TYPE_NONE, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationIconChanged",
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // icon name
		G_TYPE_STRING,  // icon desc (?)
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationIconChanged",
		G_CALLBACK(on_application_icon_changed), myApplet, NULL);
	
	dbus_g_object_register_marshaller(_cd_cclosure_marshal_VOID__INT_STRING,
			G_TYPE_NONE, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationIconThemePathChanged",
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // icon name
		G_TYPE_STRING,  // icon desc (Note: I'm quite sure they will eventually remove it ...)
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationIconThemePathChanged",
		G_CALLBACK(on_application_icon_theme_path_changed), myApplet, NULL);
	
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationLabelChanged",
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // label
		G_TYPE_STRING,  // guide
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationLabelChanged",
		G_CALLBACK(on_application_label_changed), myApplet, NULL);
	
	dbus_g_proxy_add_signal(myData.pProxyIndicatorApplicationService, "ApplicationTitleChanged",
		G_TYPE_INT,  // position
		G_TYPE_STRING,  // title
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyIndicatorApplicationService, "ApplicationTitleChanged",
		G_CALLBACK(on_application_title_changed), myApplet, NULL);
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
	if (error != NULL)
	{
		cd_debug ("Unable to watch the service: %s", error->message);
		g_error_free (error);
		/* Note: Not a big deal, let start getting items: it seems that the
		 * service is no longer available and *no longer needed* on Ubuntu 14.04
		 *  https://bazaar.launchpad.net/~indicator-applet-developers/indicator-application/trunk.14.04/revision/246
		 *  https://bugs.launchpad.net/bugs/1303731
		 * TODO: remove this ugly hack...
		 */
		service_api_version = 1;
	}
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

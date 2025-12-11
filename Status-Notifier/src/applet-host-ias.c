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


  ///////////////
 /// Signals ///
///////////////

// try to add an app from the ApplicationAdded signal or the result of the GetApplications call
// note: the caller is expected to already check the type of v3
static void _ias_add_app (GVariant *v3, gboolean bAdjustPos)
{
	CDStatusNotifierItem *pItem = g_new0 (CDStatusNotifierItem, 1);
			
	const gchar *cObjectPath = NULL;
	
	// note: we use cd_status_notifier_get_string_child_from_variant () so that empty strings are set to NULL instead
	cd_status_notifier_get_string_child_from_variant (v3, 0, &pItem->cIconName);
	g_variant_get_child (v3, 1, "i", &pItem->iPosition);
	cd_status_notifier_get_string_child_from_variant (v3, 2, &pItem->cService);
	
	GVariant *v4 = g_variant_get_child_value (v3, 3);
	cObjectPath = g_variant_get_string (v4, NULL); // note: not copied; empty strings are handled later
	
	cd_status_notifier_get_string_child_from_variant (v3, 4, &pItem->cIconThemePath);
	cd_status_notifier_get_string_child_from_variant (v3, 5, &pItem->cLabel);
	cd_status_notifier_get_string_child_from_variant (v3, 6, &pItem->cLabelGuide);
	cd_status_notifier_get_string_child_from_variant (v3, 7, &pItem->cAccessibleDesc);
	cd_status_notifier_get_string_child_from_variant (v3, 9, &pItem->cTitle);
	
	/* Note: latest version of ayatana-indicator-application has three more "s" parameters for tooltips, see:
	 * https://github.com/AyatanaIndicators/ayatana-indicator-application/pull/26/commits/f61a6ca08c2f61c26630e3c6ec14685178b0ea65
	 * However, there is no good way to get the version, but eventually, we may try to distinguish just based
	 * on the type signature. */
	
	cd_debug ("Adding app, pos: %d, addr: %s, object path: %s", pItem->iPosition, pItem->cService, cObjectPath);
	
	if (bAdjustPos)
	{
		// position +1 for items placed after this one.
		// note: unsure what should happen if adding this item eventually fails
		CDStatusNotifierItem *pItem2;
		GList *it;
		for (it = myData.pItems; it != NULL; it = it->next)
		{
			pItem2 = it->data;
			if (pItem2->iPosition >= pItem->iPosition)
			{
				pItem2->iPosition ++;
				cd_debug ("===  %s -> %d -> %d", pItem2->cId, pItem2->iPosition-1, pItem2->iPosition);
			}
		}
		
	}
	
	cd_satus_notifier_create_item (pItem, cObjectPath);
}


static void _ias_signal (G_GNUC_UNUSED GDBusProxy *pProxy, G_GNUC_UNUSED const gchar *cSender, const gchar *cSignal,
	GVariant* pPar, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	if (!myData.bIASWatched)
	{
		// We should not process signals before getting the full list of applications first
		// (this is necessary to avoid messing up the order of items)
		CD_APPLET_LEAVE ();
	}
	
	if (! strcmp (cSignal, "ApplicationAdded"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(sisossssss)")) || // last released version of ayatana-indicator-application (22.2)
			g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(sisosssssssss)")) // git master, see: https://github.com/AyatanaIndicators/ayatana-indicator-application/pull/26/commits/f61a6ca08c2f61c26630e3c6ec14685178b0ea65
		)
		{
			_ias_add_app (pPar, TRUE); // adjust positions of existing items
		}
		else cd_warning ("Unexpected parameter type for ApplicationAdded signal: %s", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "ApplicationRemoved"))
	{
		// parameter should be one integer
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(i)")))
		{
			int iPosition;
			g_variant_get_child (pPar, 0, "i", &iPosition);
			
			cd_debug ("ApplicationRemoved signal: %d", iPosition);
			
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
		}
		else cd_warning ("Unexpected parameter type for ApplicationRemoved signal: %s", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "ApplicationIconChanged"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE("(iss)")))
		{
			int iPosition;
			g_variant_get_child (pPar, 0, "i", &iPosition);
			
			CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
			if (pItem == NULL)
			{
				cd_warning ("Unknown item");
				CD_APPLET_LEAVE ();
			}
			
			GVariant *v1 = g_variant_get_child_value (pPar, 1);
			const gchar *cIconName = g_variant_get_string (v1, NULL);
			
			if (g_strcmp0 (pItem->cIconName, cIconName) != 0)  // discard useless updates (skype...)
			{
				g_free (pItem->cIconName);
				if (cIconName && *cIconName) pItem->cIconName = g_strdup (cIconName);
				else pItem->cIconName = NULL; // we do not want empty strings
				
				cd_status_notifier_get_string_child_from_variant (pPar, 2, &pItem->cAccessibleDesc);
				
				if (pItem->iStatus != CD_STATUS_NEEDS_ATTENTION)
				{
					cd_satus_notifier_update_item_image (pItem);
				}
			}
			
			g_variant_unref (v1);
		}
		else cd_warning ("Unexpected parameter type for ApplicationIconChanged signal: %s", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "ApplicationIconThemePathChanged"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE("(iss)")))
		{
			int iPosition;
			g_variant_get_child (pPar, 0, "i", &iPosition);
			
			CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
			if (pItem == NULL)
			{
				cd_warning ("Unknown item");
				CD_APPLET_LEAVE ();
			}
			
			GVariant *v1 = g_variant_get_child_value (pPar, 1);
			const gchar *cIconThemePath = g_variant_get_string (v1, NULL);
			
			if (g_strcmp0 (cIconThemePath, pItem->cIconThemePath) != 0)
			{
				if (pItem->cIconThemePath != NULL)  // if the item previously provided a theme, remove it first.
					cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
				g_free (pItem->cIconThemePath);
				if (cIconThemePath && *cIconThemePath)
				{
					pItem->cIconThemePath = g_strdup (cIconThemePath);
					cd_satus_notifier_add_theme_path (cIconThemePath);  // and add the new one.
				}
				else pItem->cIconThemePath = NULL;
				
				if (pItem->cIconName != NULL)
				{
					cd_satus_notifier_update_item_image (pItem);
				}
			}
			g_variant_unref (v1);
		}
		else cd_warning ("Unexpected parameter type for ApplicationIconThemePathChanged signal: %s", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "ApplicationLabelChanged"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE("(iss)")))
		{
			int iPosition;
			g_variant_get_child (pPar, 0, "i", &iPosition);
			
			CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
			if (pItem == NULL)
			{
				cd_warning ("Unknown item");
				CD_APPLET_LEAVE ();
			}
			
			cd_status_notifier_get_string_child_from_variant (pPar, 1, &pItem->cLabel);
			cd_status_notifier_get_string_child_from_variant (pPar, 2, &pItem->cLabelGuide);
			
		}
		else cd_warning ("Unexpected parameter type for ApplicationLabelChanged signal: %s", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "ApplicationTitleChanged"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE("(is)")))
		{
			int iPosition;
			g_variant_get_child (pPar, 0, "i", &iPosition);
			
			CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_position (iPosition);
			if (pItem == NULL)
			{
				cd_warning ("Unknown item");
				CD_APPLET_LEAVE ();
			}
			
			cd_status_notifier_get_string_child_from_variant (pPar, 1, &pItem->cTitle);
		}
		else cd_warning ("Unexpected parameter type for ApplicationTitleChanged signal: %s", g_variant_get_type_string (pPar));
	}
	
	CD_APPLET_LEAVE ();
}

  /////////////////
 /// Get Items ///
/////////////////

static void _on_get_applications_from_service (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	
	GError *error = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &error);
	
	if (error)
	{
		if (! g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("couldn't get the list of applications from IAS (%s)", error->message);
			cd_satus_notifier_unregister_from_ias ();
		}
		g_error_free (error);
		CD_APPLET_LEAVE ();
	}
	
	GVariant *v2 = g_variant_get_child_value (res, 0);
	if (g_variant_is_of_type (v2, G_VARIANT_TYPE ("a(sisossssss)")) || // last released version of ayatana-indicator-application (22.2)
		g_variant_is_of_type (v2, G_VARIANT_TYPE ("a(sisosssssssss)")) // git master, see: https://github.com/AyatanaIndicators/ayatana-indicator-application/pull/26/commits/f61a6ca08c2f61c26630e3c6ec14685178b0ea65
	)
	{
		GVariantIter iter;
		g_variant_iter_init (&iter, v2);
		GVariant *v3;
		while ((v3 = g_variant_iter_next_value (&iter)))
		{
			_ias_add_app (v3, FALSE); // do not adjust positions of existing apps, we should get everything with the correct positions
			g_variant_unref (v3);
		}
	}
	else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (v2));
	
	g_variant_unref (v2);
	g_variant_unref (res);
	
	myData.bIASWatched = TRUE; // we have a valid IAS proxy that we are using
	
	CD_APPLET_LEAVE ();
}

void cd_satus_notifier_get_items_from_ias (void)
{
	if (myData.bIASWatched || ! myData.bHaveIAS)
		return;
	cd_debug ("=== %s ()", __func__);
	
	// we have a service, get the list of applications
	g_dbus_proxy_call (myData.pProxyIndicatorApplicationService,
		"GetApplications",
		NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START,
		-1,
		myData.pCancellableIAS,
		_on_get_applications_from_service,
		NULL);
	
	// connect to the signals to keep the list of items up-to-date.
	g_signal_connect (G_OBJECT(myData.pProxyIndicatorApplicationService), "g-signal", G_CALLBACK (_ias_signal), NULL);
}

  //////////////////
 /// Connection ///
//////////////////

static void _free_item2 (gpointer ptr)
{
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	cd_free_item (pItem);
}

static void _on_ias_owner_changed (G_GNUC_UNUSED GObject *pObj, G_GNUC_UNUSED GParamSpec *par, gpointer data)
{
	gboolean bFirstRun = !!GPOINTER_TO_INT (data); // TRUE if this function is directly called from _ias_proxy_created () below
	
	if (!bFirstRun)
	{
		CD_APPLET_ENTER;
	}
	
	gchar *cNameOwner = g_dbus_proxy_get_name_owner (myData.pProxyIndicatorApplicationService);
	
	if (cNameOwner)
	{
		myData.bHaveIAS = TRUE;
		
		if (myData.bBrokenWatcher)  // if the watcher is not our friend, let's ask the IAS the current items.
			cd_satus_notifier_get_items_from_ias ();
	}
	else  // no more IAS on the bus.
	{
		if (myData.bIASWatched)
		{
			// we already received some items from the IAS proxy, we need to remove them
			myData.bIASWatched = FALSE;
			
			// we need to remove any items that we added
			g_list_free_full (myData.pItems, _free_item2);
			myData.pItems = NULL;
			
			if (! myConfig.bCompactMode)
				CD_APPLET_DELETE_MY_ICONS_LIST;
			
		}
		
		if (!bFirstRun)
		{
			// IAS was present before, but disappeared, we just forget about it and try to use our own watcher instead
			g_object_unref (myData.pProxyIndicatorApplicationService);
			myData.pProxyIndicatorApplicationService = NULL;
			
			cd_satus_notifier_launch_our_watcher ();
		}
		
		myData.bHaveIAS = FALSE;
	}
	
	if (!bFirstRun)
	{
		CD_APPLET_LEAVE ();
	}
}

static void _ias_proxy_created (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	
	GError *error = NULL;
	GDBusProxy* pProxyIAS = g_dbus_proxy_new_for_bus_finish (pRes, &error);
	if (error)
	{
		// Note: G_IO_ERROR_CANCELLED is not really an error, this just means that creating the
		// proxy was cancelled; this can happen if the applet is being unloaded.
		if (! g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error creating IAS DBus proxy: %s", error->message);
		g_error_free (error);
		CD_APPLET_LEAVE ();
	}
	
	// here we have a valid DBus proxy object, but we might not have a watcher service connected to it
	// in any case, we can store it in our applet data struct
	myData.pProxyIndicatorApplicationService = pProxyIAS;
	
	// connect to the name-owner-changed signal in case the service starts later or disappears
	// (it can happen if the service is started in parallel with Cairo-Dock on login)
	g_signal_connect (G_OBJECT(pProxyIAS), "notify::g-name-owner", G_CALLBACK (_on_ias_owner_changed), NULL);
	
	// check if we already have a service
	_on_ias_owner_changed (NULL, NULL, GINT_TO_POINTER (1));
	
	CD_APPLET_LEAVE ();
}

void cd_satus_notifier_detect_ias (void)
{
	// this is called only once when the applet is started
	myData.pCancellableIAS = g_cancellable_new ();
	g_dbus_proxy_new_for_bus (
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
		NULL, // GDBusInterfaceInfo -- TODO: see if there is a standard XML that we can use !!
		CD_INDICATOR_APPLICATION_ADDR,
		CD_INDICATOR_APPLICATION_OBJ,
		CD_INDICATOR_APPLICATION_IFACE,
		myData.pCancellableIAS,
		_ias_proxy_created,
		NULL
	);
}

void cd_satus_notifier_unregister_from_ias (void)
{
	if (myData.pCancellableIAS != NULL)
	{
		g_cancellable_cancel (myData.pCancellableIAS); // cancel any ongoing operation
		g_object_unref (myData.pCancellableIAS);
		myData.pCancellableIAS = NULL;
	}
	if (myData.pProxyIndicatorApplicationService != NULL)
	{
		g_object_unref (myData.pProxyIndicatorApplicationService);
		myData.pProxyIndicatorApplicationService = NULL;
	}

	myData.bIASWatched = FALSE;
}


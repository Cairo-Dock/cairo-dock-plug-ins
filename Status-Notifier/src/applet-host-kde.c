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

  ///////////////
 /// Signals ///
///////////////

static void _watcher_signal (G_GNUC_UNUSED GDBusProxy *pProxy, G_GNUC_UNUSED const gchar *cSender, const gchar *cSignal,
	GVariant* pPar, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	if (! strcmp (cSignal, "StatusNotifierItemRegistered"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(s)")))
		{
			GVariant *v2 = g_variant_get_child_value (pPar, 0);
			const gchar *cNotifierItemId = g_variant_get_string (v2, NULL);
			if (cNotifierItemId && *cNotifierItemId)
			{
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
				
				CDStatusNotifierItem *pItem = g_new0 (CDStatusNotifierItem, 1);
				pItem->cService = cService;
				pItem->iPosition = -1;
				cd_satus_notifier_create_item (pItem, cObjectPath);
			}
			else cd_warning ("Empty item path for StatusNotifierItemRegistered signal");
			g_variant_unref (v2);
		}
		else cd_warning ("Unexpected parameter for StatusNotifierItemRegistered signal (type: %s)", g_variant_get_type_string (pPar));
	}
	else if (! strcmp (cSignal, "StatusNotifierItemUnregistered"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(s)")))
		{
			GVariant *v2 = g_variant_get_child_value (pPar, 0);
			const gchar *cNotifierItemId = g_variant_get_string (v2, NULL);
			if (cNotifierItemId && *cNotifierItemId)
			{
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
				
				cd_satus_notifier_remove_item (cService, cObjectPath, -1);
				
				g_free (cService);
			}
			else cd_warning ("Empty item path for StatusNotifierItemUnregistered signal");
			g_variant_unref (v2);
		}
		else cd_warning ("Unexpected parameter for StatusNotifierItemUnregistered signal (type: %s)", g_variant_get_type_string (pPar));
	}
	
	CD_APPLET_LEAVE ();
}


  /////////////////
 /// Get Items ///
/////////////////

// static void _on_get_applications_from_watcher (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
static void _on_get_applications_from_watcher (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	
	GError *error = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &error);
	
	gboolean bSuccess = TRUE;
	if (error != NULL)
	{
		if (! g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("=== couldn't get applications from the watcher (%s)", error->message);
			bSuccess = FALSE;
		}
		g_error_free (error);
		error = NULL;
	}
	else
	{
		// res should be of type "(v)" where v is "as"
		GVariant *v1 = g_variant_get_child_value (res, 0);
		GVariant *v2 = g_variant_get_variant (v1);
		if (g_variant_is_of_type (v2, G_VARIANT_TYPE ("as")))
		{
			GVariantIter iter;
			g_variant_iter_init (&iter, v2);
			GVariant *v3;
			while ((v3 = g_variant_iter_next_value (&iter)))
			{
				const gchar *cItemPath = g_variant_get_string (v3, NULL);
				if (cItemPath && *cItemPath)
				{
					gchar *cService = NULL;
					const gchar *cObjectPath = NULL;
					
					const gchar *str = strchr (cItemPath, '/');
					if (str != NULL)  // service + path
					{
						cService = g_strndup (cItemPath, str - cItemPath);
						cObjectPath = str;
					}
					else  // we handle this case too, by supposing the path is the default /StatusNotifierItem
					{
						cService = g_strdup (cItemPath);
						cObjectPath = NULL;
					}
					
					CDStatusNotifierItem *pItem = g_new0 (CDStatusNotifierItem, 1);
					pItem->cService = cService;
					pItem->iPosition = -1;
					cd_satus_notifier_create_item (pItem, cObjectPath);
				}
				g_variant_unref (v3);
			}
		}
		else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (v2));
		g_variant_unref (v2);
		g_variant_unref (v1);
		g_variant_unref (res);
	}
	if (!bSuccess)   // un watcher asocial comme celui d'Ubuntu, on essaye avec l'"indicator-application".
	{
		cd_debug ("=== this watcher is not so friendly, let's try the 'application indicator'");
		// disconnect our signals since we don't want to duplicate items
		g_signal_handlers_disconnect_by_func (myData.pProxyWatcher, _watcher_signal, NULL);
		
		myData.bBrokenWatcher = TRUE;
		if (myData.bHaveIAS)
			cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}


  //////////////////
 /// Connection ///
//////////////////

// static void _on_register_host (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
static void _on_register_host (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	// cd_debug ("=== %s ()", __func__);
	CD_APPLET_ENTER;
	GError *error = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &error);
	
	gboolean bFailed = FALSE;
	
	if (error)
	{
		if (! g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("couldn't register to the Notification Watcher (%s)", error->message);
			bFailed = TRUE;
		}
		g_error_free (error);
	}
	else // we are friend now, let's ask him the current items.
	{
		if (res) g_variant_unref (res); // no return value, but just in case we get an empty GVariant
		
		cd_debug ("=== found a friendly watcher, now ask for the items...");
		// get the current items -- we create a temporary proxy for the DBus properties for this
		// Note: we can use the _sync version since this call will not block (and should not fail).
		GDBusProxy *pProxyProps = g_dbus_proxy_new_for_bus_sync (
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
				G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | // there are no properties
				G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS, // and we do not use the PropertiesChanged signal
			NULL, // GDBusInterfaceInfo -- we could use the info from the XML we parse when creating the main proxy
			CD_STATUS_NOTIFIER_WATCHER_ADDR,
			CD_STATUS_NOTIFIER_WATCHER_OBJ,
			DBUS_INTERFACE_PROPERTIES,
			NULL,
			&error
		);
		
		
		if (error)
		{
			cd_warning ("Error creating DBus proxy for getting current items from SNI watcher: %s", error->message);
			bFailed = TRUE;
		}
		else
		{
			g_dbus_proxy_call (pProxyProps, "Get",
				g_variant_new ("(ss)", CD_STATUS_NOTIFIER_WATCHER_IFACE, "RegisteredStatusNotifierItems"),
				G_DBUS_CALL_FLAGS_NO_AUTO_START,
				-1,
				myData.pCancellableWatcher,
				_on_get_applications_from_watcher,
				NULL);
			g_object_unref (pProxyProps); // we will not need this proxy later (it will be kept around for the above call)
			
			// connect to the signals to keep the list of items up-to-date
			g_signal_connect (G_OBJECT(myData.pProxyWatcher), "g-signal", G_CALLBACK (_watcher_signal), NULL);
		}
	}
	if (bFailed)  // an asocial watcher like the Ubuntu's / ayatana one, let's try with the IAS if availeble.
	{
		
		cd_debug ("=== no friendy watcher, let's try the 'application indicator'");
		myData.bBrokenWatcher = TRUE;
		if (myData.bHaveIAS)
			cd_satus_notifier_get_items_from_ias ();
	}
	CD_APPLET_LEAVE ();
}

static void _free_item2 (gpointer ptr)
{
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	cd_free_item (pItem);
}

static void _on_watcher_owner_changed (G_GNUC_UNUSED GObject *pObj, G_GNUC_UNUSED GParamSpec *par, gpointer data)
{
	gboolean bFirstRun = !!GPOINTER_TO_INT (data); // TRUE if this function is directly called from _watcher_proxy_created () below

	if (!bFirstRun)
	{
		CD_APPLET_ENTER;
	}
	
	gchar *cNameOwner = g_dbus_proxy_get_name_owner (myData.pProxyWatcher);
	
	if (cNameOwner)
	{
		g_free (cNameOwner);
		// TODO: does this mean that now the interface is always available?? (since we provided the interface when creating the proxy)
		
		myData.bNoWatcher = FALSE;
		
		// and register to it.
		cd_debug ("=== register to the SNI wathcer");
		g_dbus_proxy_call (myData.pProxyWatcher,
			"RegisterStatusNotifierHost",
			g_variant_new ("(s)", myData.cHostName),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			myData.pCancellableWatcher,
			_on_register_host,
			NULL);
		
		if (myConfig.bCompactMode)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (NULL);  // remove the broken image if it was set beforehand, to get the default icon in the items GUI.
	}
	else  // no more watcher on the bus.
	{
		// note: we keep the proxy in case it comes back (e.g. since we start our own watcher)
		
		if (! bFirstRun)
		{
			// but we disconnect our signal handlers, since we only want to get new items if we successfully re-registered
			g_signal_handlers_disconnect_by_func (myData.pProxyWatcher, _watcher_signal, NULL);
			
			// in this case, we had a watcher that disappeared, we should delete our items
			g_list_free_full (myData.pItems, _free_item2);
			myData.pItems = NULL;
			
			g_hash_table_remove_all (myData.pThemePaths);
			
			// empty the list of items and redraw.
			if (! myConfig.bCompactMode)
			{
				CD_APPLET_DELETE_MY_ICONS_LIST;
			}
		}
		
		if (myConfig.bCompactMode)
		{
			// draw an 'failed' image to not have an empty icon.
			CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/icon-broken.svg");
		}
		myData.bBrokenWatcher = FALSE;
		
		myData.bNoWatcher = TRUE;
		cd_satus_notifier_launch_our_watcher ();
	}
	
	if (! bFirstRun)
	{
		CD_APPLET_LEAVE ();
	}
}

static void _watcher_proxy_created (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	
	GError *error = NULL;
	GDBusProxy* pProxyWatcher = g_dbus_proxy_new_for_bus_finish (pRes, &error);
	if (error)
	{
		// Note: G_IO_ERROR_CANCELLED is not really an error, this just means that creating the
		// proxy was cancelled; this can happen if we the applet is being unloaded.
		if (! g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error creating SNI DBus proxy: %s", error->message);
		g_error_free (error);
		CD_APPLET_LEAVE ();
	}
	
	// here we have a valid DBus proxy object, but we might not have a watcher service connected to it
	// in any case, we can store it in our applet data struct
	myData.pProxyWatcher = pProxyWatcher;
	
	// connect to the name-owner-changed signal in case the watcher starts later or disappears
	g_signal_connect (G_OBJECT(pProxyWatcher), "notify::g-name-owner", G_CALLBACK (_on_watcher_owner_changed), NULL);
	
	// check if we have a watcher service
	_on_watcher_owner_changed (NULL, NULL, GINT_TO_POINTER (1));
	
	CD_APPLET_LEAVE ();
}


void cd_satus_notifier_detect_watcher (void)
{
	// this is called only once when the applet is started
	
	GError *error = NULL;
	
	GDBusNodeInfo *pNodeInfo = g_dbus_node_info_new_for_xml (
	"\
		<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\
		<node>\
		  <interface name=\"org.kde.StatusNotifierWatcher\">\
\
			<!-- methods -->\
			<method name=\"RegisterStatusNotifierItem\">\
			   <arg name=\"service\" type=\"s\" direction=\"in\"/>\
			</method>\
\
			<method name=\"RegisterStatusNotifierHost\">\
			   <arg name=\"service\" type=\"s\" direction=\"in\"/>\
			</method>\
\
\
			<!-- properties -->\
\
			<property name=\"RegisteredStatusNotifierItems\" type=\"as\" access=\"read\">\
			   <annotation name=\"com.trolltech.QtDBus.QtTypeName.Out0\" value=\"QStringList\"/>\
			</property>\
\
			<property name=\"IsStatusNotifierHostRegistered\" type=\"b\" access=\"read\"/>\
\
			<property name=\"ProtocolVersion\" type=\"i\" access=\"read\"/>\
\
\
			<!-- signals -->\
\
			<signal name=\"StatusNotifierItemRegistered\">\
				<arg type=\"s\"/>\
			</signal>\
\
			<signal name=\"StatusNotifierItemUnregistered\">\
				<arg type=\"s\"/>\
			</signal>\
\
			<signal name=\"StatusNotifierHostRegistered\">\
			</signal>\
\
			<signal name=\"StatusNotifierHostUnregistered\">\
			</signal>\
		  </interface>\
		</node>\
	",
		&error);
	
	if (error)
	{
		// should not happen, we know the XML is valid
		cd_warning ("%s", error->message);
		g_error_free (error);
		return;
	}
	
	// note: return value is owned by pNodeInfo
	GDBusInterfaceInfo *pInfo = g_dbus_node_info_lookup_interface (pNodeInfo, CD_STATUS_NOTIFIER_WATCHER_IFACE);
	
	if (!pInfo)
	{
		cd_warning ("Cannot build SNI DBus interface info!");
		g_dbus_node_info_unref (pNodeInfo);
	}
	
	myData.pCancellableWatcher = g_cancellable_new ();
	g_dbus_proxy_new_for_bus (
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
		pInfo, // GDBusInterfaceInfo
		CD_STATUS_NOTIFIER_WATCHER_ADDR,
		CD_STATUS_NOTIFIER_WATCHER_OBJ,
		CD_STATUS_NOTIFIER_WATCHER_IFACE,
		myData.pCancellableWatcher,
		_watcher_proxy_created,
		NULL
	);
	
	g_dbus_node_info_unref (pNodeInfo); // the previous call should have taken a ref to pInfo
}

void cd_satus_notifier_unregister_from_watcher (void)
{
	if (myData.pCancellableWatcher != NULL)
	{
		g_cancellable_cancel (myData.pCancellableWatcher); // cancel any ongoing operation
		g_object_unref (myData.pCancellableWatcher);
		myData.pCancellableWatcher = NULL;
	}
	if (myData.pProxyWatcher != NULL)
	{
		g_object_unref (myData.pProxyWatcher);
		myData.pProxyWatcher = NULL;
	}
}


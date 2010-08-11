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
#include "applet-host.h"

#define CD_STATUS_NOTIFIER_HOST_ADDR "org.kde.NotificationHost"
#define CD_STATUS_NOTIFIER_WATCHER_ADDR "org.kde.NotificationItemWatcher"  // org.kde.StatusNotifierWatcher
#define CD_STATUS_NOTIFIER_WATCHER_OBJ "/NotificationItemWatcher"  // StatusNotifierWatcher
#define CD_STATUS_NOTIFIER_WATCHER_IFACE "org.kde.NotificationItemWatcher"  // org.kde.StatusNotifierWatcher


static CDStatusNotifierItem * _cd_satus_notifier_find_item_from_service (const gchar *cService)
{
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


static void on_new_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("new item : '%s'\n", cService);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_create_item (cService);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	myData.pItems = g_list_prepend (myData.pItems, pItem);
	
	if (myConfig.bCompactMode)
	{
		cairo_dock_load_icon_image (myIcon, myContainer);
	}
	else
	{
		Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (pItem->cTitle?pItem->cTitle:pItem->cId),
			g_strdup (pItem->cIconName),
			g_strdup (pItem->cService),
			NULL,
			pItem->iCategory);
		CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_removed_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("item removed : '%s'\n", cService);
	
	gchar *str = strchr (cService, '/');
	if (str)
		*str = '\0';
	
	CDStatusNotifierItem *pItem = _cd_satus_notifier_find_item_from_service (cService);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	myData.pItems = g_list_remove (myData.pItems, pItem);
	
	if (myConfig.bCompactMode)
	{
		cairo_dock_load_icon_image (myIcon, myContainer);
	}
	else
	{
		Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (pItem->cTitle?pItem->cTitle:pItem->cId),
			g_strdup (pItem->cIconName),
			g_strdup (pItem->cService),
			NULL,
			pItem->iCategory);
		CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
	}
	
	cd_free_item (pItem);
	
	CD_APPLET_LEAVE ();
}


static int _compare_items (const CDStatusNotifierItem *i1, const CDStatusNotifierItem *i2)
{
	if (!i1)
		return -1;
	if (!i2)
		return 1;
	return (i1->iCategory < i2->iCategory ? -1 : (i1->iCategory > i2->iCategory ? 1 : 0));
}

static void _load_my_icon_image (Icon *pIcon)
{
	CD_APPLET_ENTER;
	int iWidth = pIcon->iImageWidth;
	int iHeight = pIcon->iImageHeight;
	
	if (myConfig.bCompactMode)
	{
		pIcon->pIconBuffer = cairo_dock_create_blank_surface (iWidth, iHeight);
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		/// for each item
			/// create surface
			/// draw
		cairo_destroy (pIconContext);
	}
	else
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->cFileName);
		if (cIconPath != NULL && *cIconPath != '\0')
			pIcon->pIconBuffer = cairo_dock_create_surface_from_image_simple (cIconPath,
				iWidth,
				iHeight);
		g_free (cIconPath);
	}
	
	CD_APPLET_LEAVE ();
}


static gboolean _cd_satus_notifier_register_host (void)
{
	g_print ("%s ()\n", __func__);
	
	// register to the watcher.
	g_print ("registering to the watcher...\n");
	GError *erreur = NULL;
	dbus_g_proxy_call (myData.pProxyWatcher, "RegisterNotificationHost"/**"RegisterStatusNotifierHost"*/, &erreur,
		G_TYPE_STRING, myData.cHostName,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't register to the Notification Watcher (%s)\nYour system doesn't support Systray 2.0", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	// connect to the signals.
	dbus_g_proxy_add_signal(myData.pProxyWatcher, "ServiceRegistered"/**"StatusNotifierItemRegistered"*/,
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyWatcher, "ServiceRegistered"/**"StatusNotifierItemRegistered"*/,
		G_CALLBACK(on_new_item), myApplet, NULL);
	dbus_g_proxy_add_signal(myData.pProxyWatcher, "ServiceUnregistered"/**"StatusNotifierItemUnregistered"*/,
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyWatcher, "ServiceUnregistered"/**"StatusNotifierItemUnregistered"*/,
		G_CALLBACK(on_removed_item), myApplet, NULL);
	return TRUE;
}

static void _cd_satus_notifier_get_items (void)
{
	g_print ("%s ()\n", __func__);
	
	// get the items.
	DBusGProxy *pProxyWatcherProps = cairo_dock_create_new_session_proxy (
		CD_STATUS_NOTIFIER_WATCHER_ADDR,
		CD_STATUS_NOTIFIER_WATCHER_OBJ,
		"org.freedesktop.DBus.Properties");
	gchar **cItemsName = cairo_dock_dbus_get_property_as_string_list (pProxyWatcherProps, CD_STATUS_NOTIFIER_WATCHER_IFACE, "RegisteredStatusNotifierItems");
	g_object_unref (pProxyWatcherProps);
	
	// create all the icons.
	if (cItemsName != NULL)
	{
		GList *pIcons = NULL;
		CDStatusNotifierItem *pItem;
		int i;
		for (i = 0; cItemsName[i] != NULL; i ++)
		{
			pItem = cd_satus_notifier_create_item (cItemsName[i]);
			myData.pItems = g_list_prepend (myData.pItems, pItem);
			
			if (! myConfig.bCompactMode)
			{
				Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (pItem->cTitle?pItem->cTitle:pItem->cId),
					g_strdup (pItem->cIconName),
					g_strdup (pItem->cService),
					NULL,
					pItem->iCategory);
				pIcons = g_list_prepend (pIcons, pIcon);
			}
		}
		g_strfreev (cItemsName);
		
		if (! myConfig.bCompactMode)
			CD_APPLET_LOAD_MY_ICONS_LIST (pIcons, NULL, "Slide", NULL);
	}
	
	myIcon->iface.load_image = _load_my_icon_image;
}

static gboolean _get_watcher (CairoDockModuleInstance *myApplet)
{
	myData.pProxyWatcher = cairo_dock_create_new_session_proxy (
		CD_STATUS_NOTIFIER_WATCHER_ADDR,
		CD_STATUS_NOTIFIER_WATCHER_OBJ,
		CD_STATUS_NOTIFIER_WATCHER_IFACE);
	if (myData.pProxyWatcher != NULL)
	{
		myData.iSidGetWatcher = 0;
		_cd_satus_notifier_register_host ();
		
		_cd_satus_notifier_get_items ();
		return FALSE;
	}
	return TRUE;
}
void cd_satus_notifier_launch_service (void)
{
	g_print ("%s ()\n", __func__);
	
	// Register the service name no the bus.
	pid_t pid = getpid ();
	myData.cHostName = g_strdup_printf (CD_STATUS_NOTIFIER_HOST_ADDR"-%d", pid);
	g_print ("registering %s ...\n", myData.cHostName);
	cairo_dock_register_service_name (myData.cHostName);
	
	// get the watcher.
	g_print ("getting the watcher...\n");
	myData.pProxyWatcher = cairo_dock_create_new_session_proxy (
		CD_STATUS_NOTIFIER_WATCHER_ADDR,
		CD_STATUS_NOTIFIER_WATCHER_OBJ,
		CD_STATUS_NOTIFIER_WATCHER_IFACE);  /// dbus_g_proxy_new_for_name_owner ?...
	if (myData.pProxyWatcher == NULL)  // no watcher yet, let's try again in a few moment.
	{
		g_print ("no watcher yet, let's try again in a few moment\n");
		myData.iSidGetWatcher = g_timeout_add (1000., (GSourceFunc)_get_watcher, myApplet);
		return;
	}
	
	_cd_satus_notifier_register_host ();
	
	_cd_satus_notifier_get_items ();
}


void cd_satus_notifier_stop_service (void)
{
	if (myData.iSidGetWatcher != 0)
		g_source_remove (myData.iSidGetWatcher);
	
	g_object_unref (myData.pProxyWatcher);
	
	g_list_foreach (myData.pItems, (GFunc) cd_free_item, NULL);
	g_list_free (myData.pItems);
	
	if (! myConfig.bCompactMode)
		CD_APPLET_DELETE_MY_ICONS_LIST;
}

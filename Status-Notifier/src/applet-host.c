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
#include "applet-host.h"

#define CD_STATUS_NOTIFIER_ITEM_IFACE "org.kde.StatusNotifierItem"
#define CD_STATUS_NOTIFIER_ITEM_OBJ "/StatusNotifierItem"
#define CD_STATUS_NOTIFIER_HOST_ADDR "org.kde.StatusNotifierHost"
#define CD_STATUS_NOTIFIER_WATCHER_ADDR "org.kde.StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_OBJ "/StatusNotifierWatcher"
#define CD_STATUS_NOTIFIER_WATCHER_IFACE "org.kde.StatusNotifierWatcher"

static void cd_free_tooltip (CDToolTip *pToolTip);
static void cd_free_item_data (CDStatusNotifierItemData *pItemData);
static Icon *cd_satus_notifier_create_item_icon (const gchar *cService);


G_DEFINE_TYPE(statusNotifierHostObject, cd_satus_notifier_host, G_TYPE_OBJECT);

static void cd_satus_notifier_host_class_init(statusNotifierHostObjectClass *klass)
{
	cd_message("");
}
static void cd_satus_notifier_host_init (statusNotifierHostObject *pMainObject)
{
	cd_message("");
	
	// Initialise the DBus connection
	pMainObject->connection = cairo_dock_get_session_connection ();
	
	//dbus_g_object_type_install_info(cd_satus_notifier_host_get_type(), &dbus_glib_cd_satus_notifier_host_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(pMainObject->connection, "/org/kde/StatusNotifierHost", G_OBJECT(pMainObject));
}


static CDCategoryEnum _find_category (const gchar *cCategory)
{
	if (!cCategory)
		return CD_CATEGORY_APPLICATION_STATUS;
	if (*cCategory == 'A')
		return CD_CATEGORY_APPLICATION_STATUS;
	if (*cCategory == 'C')
		return CD_CATEGORY_COMMUNICATIONS;
	if (*cCategory == 'S')
		return CD_CATEGORY_SYSTEM_SERVICES;
	if (*cCategory == 'H')
		return CD_CATEGORY_HARDWARE;
	return CD_CATEGORY_APPLICATION_STATUS;
}

static CDStatusEnum _find_status (const gchar *cStatus)
{
	if (!cStatus)
		return CD_STATUS_ACTIVE;
	if (*cStatus == 'N')
		return CD_STATUS_NEEDS_ATTENTION;
	if (*cStatus == 'A')
		return CD_STATUS_ACTIVE;
	if (*cStatus == 'P')
		return CD_STATUS_PASSIVE;
	return CD_STATUS_ACTIVE;
}

static CDToolTip *_make_tooltip_from_dbus_struct (GValueArray *pToolTipTab)
{
	CDToolTip *pToolTip = NULL;
	if (pToolTipTab)
	{
		pToolTip = g_new0 (CDToolTip, 1);
		GValue *v = &pToolTipTab->values[0];
		if (v && G_VALUE_HOLDS_STRING (v))
			pToolTip->cIconName = g_strdup (g_value_get_string (v));
		v = &pToolTipTab->values[2];
		if (v && G_VALUE_HOLDS_STRING (v))
			pToolTip->cTitle = g_strdup (g_value_get_string (v));
		v = &pToolTipTab->values[3];
		if (v && G_VALUE_HOLDS_STRING (v))
			pToolTip->cMessage = g_strdup (g_value_get_string (v));
		if (pToolTip->cMessage != NULL)
		{
			if (strncmp (pToolTip->cMessage, "<qt>", 4) == 0)
			{
				gchar *str = pToolTip->cMessage;
				int n = strlen (str);
				*(str + n - 5) = '\0';
				pToolTip->cMessage = g_strdup (str+4);
				g_free (str);
			}
			/// remplacer <br/> pas \n
			
			/// virer les <nobr> et </nobr>
			
			/// virer les <img src=...> et </img>
			
		}
	}
	return pToolTip;
}

static void _show_item_tooltip (Icon *pIcon, CDStatusNotifierItemData *pItemData)
{
	gchar *cText = g_strdup_printf ("<b>%s</b>\n%s", pItemData->pToolTip->cTitle, pItemData->pToolTip->cMessage);
	cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 4000, pItemData->pToolTip->cIconName);
	g_free (cText);
}

static void _show_item_status (Icon *pIcon, CDStatusNotifierItemData *pItemData)
{
	switch (pItemData->iStatus)
	{
		case CD_STATUS_PASSIVE :
			pIcon->fAlpha = 0.5;
			cairo_dock_stop_icon_attention (pIcon, myIcon->pSubDock);
			cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		break;
		case CD_STATUS_ACTIVE :
		default:
			pIcon->fAlpha = 1.;
			cairo_dock_stop_icon_attention (pIcon, myIcon->pSubDock);
			cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		break;
		case CD_STATUS_NEEDS_ATTENTION:
			pIcon->fAlpha = 1.;
			cairo_dock_request_icon_attention (pIcon, myIcon->pSubDock, "rotate", 60);
		break;
	}
}


static void on_new_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("new item : '%s'\n", cService);
	
	Icon *pIcon = cd_satus_notifier_create_item_icon (cService);
	CD_APPLET_LEAVE_IF_FAIL (pIcon != NULL);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	cairo_dock_fill_icon_buffers (pIcon, cairo_dock_get_max_scale (myIcon->pSubDock), myIcon->pSubDock->container.bIsHorizontal, myIcon->pSubDock->container.bDirectionUp);
	
	if (myIcon->pSubDock)
		cairo_dock_insert_icon_in_dock (pIcon, myIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
	else
	{
		CD_APPLET_LOAD_MY_ICONS_LIST (g_list_copy (myData.pIcons), NULL, "Caroussel", NULL);
	}
	
	if (pItemData->iStatus != CD_STATUS_ACTIVE)
		_show_item_status (pIcon, pItemData);
	
	CD_APPLET_LEAVE ();
}

static void on_removed_item (DBusGProxy *proxy_watcher, const gchar *cService, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("item removed : '%s'\n", cService);
	
	gchar *str = strchr (cService, '/');
	if (str)
		*str = '\0';
	
	Icon *pIcon = cd_satus_notifier_find_icon_from_service (cService);
	CD_APPLET_LEAVE_IF_FAIL (pIcon != NULL);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	myData.pIcons = g_list_remove (myData.pIcons, pIcon);
	if (myIcon->pSubDock)
		cairo_dock_detach_icon_from_dock (pIcon, myIcon->pSubDock, TRUE);
	
	cd_free_item_data (pItemData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	cairo_dock_free_icon (pIcon);
	
	cairo_dock_update_dock_size (myIcon->pSubDock);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_title (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	gchar *cTitle = cairo_dock_dbus_get_property_as_string (pItemData->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "Title");
	cairo_dock_set_icon_name (cTitle, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
	g_free (cTitle);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	gchar *cIconName = cairo_dock_dbus_get_property_as_string (pItemData->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "IconName");
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, pItemData->cAttentionIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
	cairo_destroy (pIconContext);
	g_free (cIconName);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_attention_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	g_free (pItemData->cAttentionIconName);
	pItemData->cAttentionIconName = cairo_dock_dbus_get_property_as_string (pItemData->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "AttentionIconName");
	if (pIcon->bIsDemandingAttention && pIcon->pIconBuffer)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, pItemData->cAttentionIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_destroy (pIconContext);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_overlay_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	g_free (pItemData->cOverlayIconName);
	pItemData->cOverlayIconName = cairo_dock_dbus_get_property_as_string (pItemData->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "OverlayIconName");
	if (pIcon->pIconBuffer)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, pIcon->cFileName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_destroy (pIconContext);
		
		if (pItemData->cOverlayIconName != NULL)
		{
			CairoEmblem *pEmblem = cairo_dock_make_emblem (pItemData->cOverlayIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			cairo_dock_set_emblem_position (pEmblem, CAIRO_DOCK_EMBLEM_MIDDLE);
			cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			cairo_dock_free_emblem (pEmblem);
		}
	}
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_tooltip (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	cairo_dock_remove_dialog_if_any (pIcon);
	cd_free_tooltip (pItemData->pToolTip);
	pItemData->pToolTip = NULL;
	
	GValueArray *pToolTipTab = cairo_dock_dbus_get_property_as_boxed (pItemData->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "ToolTip");
	if (pToolTipTab)
	{
		pItemData->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
		
		if (pItemData->pToolTip && pItemData->pToolTip->cMessage != NULL)
			_show_item_tooltip (pIcon, pItemData);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_status (DBusGProxy *proxy_item, const gchar *cStatus, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s (%s)\n", __func__, cStatus);
	CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItemData != NULL);
	
	pItemData->iStatus = _find_status (cStatus);
	_show_item_status (pIcon, pItemData);
	
	CD_APPLET_LEAVE ();
}


static int _compare_items (const CDStatusNotifierItemData *i1, const CDStatusNotifierItemData *i2)
{
	if (!i1)
		return -1;
	if (!i2)
		return 1;
	return (i1->iCategory < i2->iCategory ? -1 : (i1->iCategory > i2->iCategory ? 1 : 0));
}
static Icon *cd_satus_notifier_create_item_icon (const gchar *cService)
{
	gchar *str = strchr (cService, '/');
	if (str)
		*str = '\0';
	g_print (" + item '%s' on the bus\n", cService);
	
	//\_________________ get the properties of the item.
	DBusGProxy *pProxyItemProp = cairo_dock_create_new_session_proxy (
		cService,
		"/StatusNotifierItem",
		"org.freedesktop.DBus.Properties");
	if (pProxyItemProp == NULL)
		return NULL;
	
	g_print ("getting properties ...\n");
	GHashTable *hProps = cairo_dock_dbus_get_all_properties (pProxyItemProp, CD_STATUS_NOTIFIER_ITEM_IFACE);
	if (hProps == NULL)
		return NULL;
	
	GValue *v;
	const gchar *cId = NULL;
	v = g_hash_table_lookup (hProps, "Id");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cId = g_value_get_string (v);
	}
	g_print ("  ID '%s\n", cId);
	
	const gchar *cTitle = NULL;
	v = g_hash_table_lookup (hProps, "Title");  // -> cName
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cTitle = g_value_get_string (v);
	}
	g_print ("  Title '%s\n", cTitle);
	
	const gchar *cCategory = NULL;
	v = g_hash_table_lookup (hProps, "Category");  // (ApplicationStatus, Communications, SystemServices, Hardware) -> fOrder
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cCategory = g_value_get_string (v);
	}
	g_print ("  Category '%s'\n", cCategory);
	
	const gchar *cStatus = NULL;
	v = g_hash_table_lookup (hProps, "Category");  // (Passive, Active, NeedsAttention) -> demands attention
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cStatus = g_value_get_string (v);
	}
	g_print ("  Status '%s'\n", cStatus);
	
	v = g_hash_table_lookup (hProps, "WindowId");
	guint iWindowId = 0;
	if (v && G_VALUE_HOLDS_UINT(v))
	{
		iWindowId = g_value_get_uint (v);
	}
	g_print ("  WindowId '%d'\n", iWindowId);
	
	const gchar *cIconName = NULL;
	v = g_hash_table_lookup (hProps, "IconName");  // -> cIFileName
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cIconName = g_value_get_string (v);
	}
	g_print ("  IconName '%s'\n", cIconName);
	
	const gchar *cIconThemePath = NULL;
	v = g_hash_table_lookup (hProps, "IconThemePath");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cIconThemePath = g_value_get_string (v);
	}
	g_print ("  IconThemePath '%s'\n", cIconThemePath);
	
	const gchar *cOverlayIconName = NULL;
	v = g_hash_table_lookup (hProps, "OverlayIconName");  // -> emblem
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cOverlayIconName = g_value_get_string (v);
	}
	g_print ("  OverlayIconName '%s'\n", cOverlayIconName);
	
	const gchar *cAttentionIconName = NULL;
	v = g_hash_table_lookup (hProps, "AttentionIconName");  // -> keep for demands of attention
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cAttentionIconName = g_value_get_string (v);
	}
	g_print ("  AttentionIconName '%s'\n", cAttentionIconName);
	
	const gchar *cAttentionMovieName = NULL;
	v = g_hash_table_lookup (hProps, "AttentionMovieName");  // -> idem
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cAttentionMovieName = g_value_get_string (v);
	}
	g_print ("  AttentionMovieName '%s'\n", cAttentionMovieName);
	
	GValueArray *pToolTipTab = NULL;
	v = g_hash_table_lookup (hProps, "ToolTip");
	if (v && G_VALUE_HOLDS_BOXED (v))
	{
		pToolTipTab = g_value_get_boxed (v);
	}
	
	g_print ("creating the icon...\n");
	DBusGProxy *pProxyItem = cairo_dock_create_new_session_proxy (
		cService,
		"/StatusNotifierItem",
		CD_STATUS_NOTIFIER_ITEM_IFACE);
	if (pProxyItem == NULL)
		return NULL;
	
	//\_________________ create a new item.
	CDStatusNotifierItemData *pItemData = g_new0 (CDStatusNotifierItemData, 1);
	pItemData->cService = g_strdup (cService);
	pItemData->pProxyProps = pProxyItemProp;
	pItemData->pProxy = pProxyItem;
	pItemData->cId = g_strdup (cId);
	pItemData->iWindowId = iWindowId;
	pItemData->iCategory = _find_category (cCategory);
	pItemData->iStatus = _find_status (cStatus);
	pItemData->cIconThemePath = g_strdup (cIconThemePath);
	pItemData->cAttentionIconName = g_strdup (cAttentionIconName);
	pItemData->cAttentionMovieName = g_strdup (cAttentionMovieName);
	pItemData->cOverlayIconName = g_strdup (cOverlayIconName);
	if (pToolTipTab)
	{
		pItemData->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
	}
	
	//\_________________ create a new associated icon.
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->cName = g_strdup (cTitle);
	pIcon->cFileName = g_strdup (cIconName);
	pIcon->fOrder = pItemData->iCategory;
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->cCommand = g_strdup ("none");
	CD_APPLET_SET_MY_ICON_DATA (pIcon, pItemData);
	
	myData.pIcons = g_list_insert_sorted (myData.pIcons, pIcon, (GCompareFunc)_compare_items);
	
	//\_________________ track any changes in the item.
	dbus_g_proxy_add_signal(pProxyItem, "NewTitle",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewTitle",
		G_CALLBACK(on_new_item_title), myApplet, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewIcon",
		G_CALLBACK(on_new_item_icon), myApplet, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewAttentionIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewAttentionIcon",
		G_CALLBACK(on_new_item_attention_icon), myApplet, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewOverlayIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewOverlayIcon",
		G_CALLBACK(on_new_item_overlay_icon), myApplet, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewToolTip",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewToolTip",
		G_CALLBACK(on_new_item_tooltip), myApplet, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewStatus",
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewStatus",
		G_CALLBACK(on_new_item_status), myApplet, NULL);
	
	g_print ("destroy props...\n");
	g_hash_table_destroy (hProps);
	return pIcon;
}

static void cd_free_tooltip (CDToolTip *pToolTip)
{
	if (pToolTip == NULL)
		return;
	g_free (pToolTip->cIconName);
	g_free (pToolTip->cTitle);
	g_free (pToolTip->cMessage);
	g_free (pToolTip);
}

static void cd_free_item_data (CDStatusNotifierItemData *pItemData)
{
	if (pItemData == NULL)
		return;
	if (pItemData->iSidPopupTooltip != 0)
		g_source_remove (pItemData->iSidPopupTooltip);
	g_object_unref (pItemData->pProxy);
	g_object_unref (pItemData->pProxyProps);
	g_free (pItemData->cService);
	g_free (pItemData->cId);
	g_free (pItemData->cAttentionIconName);
	g_free (pItemData->cAttentionMovieName);
	g_free (pItemData->cOverlayIconName);
	cd_free_tooltip (pItemData->pToolTip);
	g_free (pItemData);
}



void cd_satus_notifier_launch_service (void)
{
	g_return_if_fail (myData.pMainObject == NULL);
	g_type_init();
	
	// on cree l'objet distant principal.
	g_print ("satus-notifier : Lancement du service\n");
	pid_t pid = getpid ();
	///myData.pMainObject = g_object_new (cd_satus_notifier_host_get_type(), NULL);  // appelle class_init() et init().
	
	// Register the service name no the bus.
	myData.cHostName = g_strdup_printf (CD_STATUS_NOTIFIER_HOST_ADDR"-%d", pid);
	g_print ("registering %s ...\n", myData.cHostName);
	cairo_dock_register_service_name (myData.cHostName);
	
	// get the watcher.
	g_print ("getting the watcher...\n");
	myData.pProxyWatcher = cairo_dock_create_new_session_proxy (
		CD_STATUS_NOTIFIER_WATCHER_ADDR,
		CD_STATUS_NOTIFIER_WATCHER_OBJ,
		CD_STATUS_NOTIFIER_WATCHER_IFACE);
	g_return_if_fail (myData.pProxyWatcher != NULL);
	
	// register to the watcher.
	g_print ("registering to the watcher...\n");
	GError *erreur = NULL;
	dbus_g_proxy_call (myData.pProxyWatcher, "RegisterStatusNotifierHost", &erreur,
		G_TYPE_STRING, myData.cHostName,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_warning ("couldn't find a Status Notifier Watcher (%s)\nYour system doesn't support Systray 2.0", erreur->message);
		g_error_free (erreur);
		return;
	}
	// connect to the signals.
	dbus_g_proxy_add_signal(myData.pProxyWatcher, "StatusNotifierItemRegistered",
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyWatcher, "StatusNotifierItemRegistered",
		G_CALLBACK(on_new_item), myApplet, NULL);
	dbus_g_proxy_add_signal(myData.pProxyWatcher, "StatusNotifierItemUnregistered",
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pProxyWatcher, "StatusNotifierItemUnregistered",
		G_CALLBACK(on_removed_item), myApplet, NULL);
	
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
		int i;
		for (i = 0; cItemsName[i] != NULL; i ++)
		{
			cd_satus_notifier_create_item_icon (cItemsName[i]);
		}
		g_strfreev (cItemsName);
		
		CD_APPLET_LOAD_MY_ICONS_LIST (g_list_copy (myData.pIcons), NULL, "Caroussel", NULL);
	}
}

void cd_satus_notifier_stop_service (void)
{
	if (myData.pMainObject = NULL)
		return;
	g_object_unref (myData.pMainObject);
	g_object_unref (myData.pProxyWatcher);
	
	Icon *pIcon = NULL;
	GList *ic;
	for (ic = myData.pIcons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		cd_free_item_data (pItemData);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	}
	g_list_free (myData.pIcons);
	CD_APPLET_DELETE_MY_ICONS_LIST;
}

Icon * cd_satus_notifier_find_icon_from_service (const gchar *cService)
{
	Icon *pIcon = NULL;
	GList *ic;
	for (ic = myData.pIcons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		CDStatusNotifierItemData *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		if (pItemData && pItemData->cService && strcmp (pItemData->cService, cService) == 0)
			return pIcon;
	}
	return NULL;
}


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
			/// remplacer <br/> par \n
			
			/// virer les <nobr> et </nobr>
			
			/// virer les <img src=...> et </img>
			
		}
	}
	return pToolTip;
}

static void _show_item_tooltip (Icon *pIcon, CDStatusNotifierItem *pItem)
{
	gchar *cText = g_strdup_printf ("<b>%s</b>\n%s", pItem->pToolTip->cTitle, pItem->pToolTip->cMessage);
	cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 4000, pItem->pToolTip->cIconName);
	g_free (cText);
}

static void _show_item_status (Icon *pIcon, CDStatusNotifierItem *pItem)
{
	switch (pItem->iStatus)
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


static void on_new_item_title (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	gchar *cTitle = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "Title");
	cairo_dock_set_icon_name (cTitle, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
	g_free (cTitle);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	gchar *cIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "IconName");
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, pItem->cAttentionIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
	cairo_destroy (pIconContext);
	g_free (cIconName);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_attention_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	g_free (pItem->cAttentionIconName);
	pItem->cAttentionIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "AttentionIconName");
	if (pIcon->bIsDemandingAttention && pIcon->pIconBuffer)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, pItem->cAttentionIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_destroy (pIconContext);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_overlay_icon (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	g_free (pItem->cOverlayIconName);
	pItem->cOverlayIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "OverlayIconName");
	if (pIcon->pIconBuffer)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, pIcon->cFileName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_destroy (pIconContext);
		
		if (pItem->cOverlayIconName != NULL)
		{
			CairoEmblem *pEmblem = cairo_dock_make_emblem (pItem->cOverlayIconName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			cairo_dock_set_emblem_position (pEmblem, CAIRO_DOCK_EMBLEM_MIDDLE);
			cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			cairo_dock_free_emblem (pEmblem);
		}
	}
	
	CD_APPLET_LEAVE ();
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

static void on_new_item_tooltip (DBusGProxy *proxy_item, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s ()\n", __func__);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	cairo_dock_remove_dialog_if_any (pIcon);
	cd_free_tooltip (pItem->pToolTip);
	pItem->pToolTip = NULL;
	
	GValueArray *pToolTipTab = cairo_dock_dbus_get_property_as_boxed (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "ToolTip");
	if (pToolTipTab)
	{
		pItem->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
		
		if (pItem->pToolTip && pItem->pToolTip->cMessage != NULL)
			_show_item_tooltip (pIcon, pItem);
	}
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_status (DBusGProxy *proxy_item, const gchar *cStatus, Icon *pIcon)
{
	CD_APPLET_ENTER;
	g_print ("%s (%s)\n", __func__, cStatus);
	CDStatusNotifierItem *pItem = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	pItem->iStatus = _find_status (cStatus);
	_show_item_status (pIcon, pItem);
	
	CD_APPLET_LEAVE ();
}


CDStatusNotifierItem *cd_satus_notifier_create_item (const gchar *cService)
{
	gchar *str = strchr (cService, '/');
	if (str)
		*str = '\0';
	g_print (" + item '%s' on the bus\n", cService);
	
	//\_________________ get the properties of the item.
	DBusGProxy *pProxyItemProp = cairo_dock_create_new_session_proxy (
		cService,
		CD_STATUS_NOTIFIER_ITEM_OBJ,
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
	v = g_hash_table_lookup (hProps, "Status");  // (Passive, Active, NeedsAttention) -> demands attention
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
	CDStatusNotifierItem *pItem = g_new0 (CDStatusNotifierItem, 1);
	pItem->cService = g_strdup (cService);
	pItem->pProxyProps = pProxyItemProp;
	pItem->pProxy = pProxyItem;
	pItem->cId = g_strdup (cId);
	pItem->cTitle = g_strdup (cTitle);
	pItem->iWindowId = iWindowId;
	pItem->iCategory = _find_category (cCategory);
	pItem->iStatus = _find_status (cStatus);
	pItem->cIconName = g_strdup (cIconName);
	pItem->cIconThemePath = g_strdup (cIconThemePath);
	pItem->cAttentionIconName = g_strdup (cAttentionIconName);
	pItem->cAttentionMovieName = g_strdup (cAttentionMovieName);
	pItem->cOverlayIconName = g_strdup (cOverlayIconName);
	if (pToolTipTab)
	{
		pItem->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
	}
	
	myData.pItems = g_list_prepend (myData.pItems, pItem);
	
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
	return pItem;
}

void cd_free_item (CDStatusNotifierItem *pItem)
{
	if (pItem == NULL)
		return;
	if (pItem->iSidPopupTooltip != 0)
		g_source_remove (pItem->iSidPopupTooltip);
	g_object_unref (pItem->pProxy);
	g_object_unref (pItem->pProxyProps);
	g_free (pItem->cService);
	g_free (pItem->cId);
	g_free (pItem->cIconName);
	g_free (pItem->cAttentionIconName);
	g_free (pItem->cAttentionMovieName);
	g_free (pItem->cOverlayIconName);
	cd_free_tooltip (pItem->pToolTip);
	g_free (pItem);
}

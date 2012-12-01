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
#include "applet-draw.h"
#include "applet-notifications.h"
#include "applet-item.h"

#define CD_STATUS_NOTIFIER_ITEM_IFACE "org.kde.StatusNotifierItem"
#define CD_STATUS_NOTIFIER_ITEM_OBJ "/StatusNotifierItem"

#define CD_INDICATOR_APPLICATION_ITEM_OBJ "/org/ayatana/NotificationItem"


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
	cd_debug ("STATUS: %s", cStatus);
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

static void cd_free_tooltip (CDToolTip *pToolTip)
{
	if (pToolTip == NULL)
		return;
	g_free (pToolTip->cIconName);
	g_free (pToolTip->cTitle);
	g_free (pToolTip->cMessage);
	g_free (pToolTip);
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
/* Not used
static void _show_item_tooltip (Icon *pIcon, CDStatusNotifierItem *pItem)
{
	gchar *cText = g_strdup_printf ("<b>%s</b>\n%s", pItem->pToolTip->cTitle, pItem->pToolTip->cMessage);
	cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 4000, pItem->pToolTip->cIconName);
	g_free (cText);
}
*/
/*static void _show_item_status (Icon *pIcon, CDStatusNotifierItem *pItem)
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
}*/


static void on_new_item_icon (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s ()", __func__);
	
	g_free (pItem->cIconName);
	pItem->cIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "IconName");
	g_free (pItem->cAccessibleDesc);
	pItem->cAccessibleDesc = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "IconAccessibleDesc");
	cd_debug ("===  new icon : %s", pItem->cIconName);
	
	if (pItem->iStatus != CD_STATUS_NEEDS_ATTENTION)
	{
		cd_satus_notifier_update_item_image (pItem);
	}
	CD_APPLET_LEAVE ();
}

static void on_new_item_attention_icon (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s ()", __func__);
	
	g_free (pItem->cAttentionIconName);
	pItem->cAttentionIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "AttentionIconName");
	cd_debug ("===  new attention icon : %s", pItem->cAttentionIconName);
	
	if (pItem->iStatus == CD_STATUS_NEEDS_ATTENTION)
	{
		cd_satus_notifier_update_item_image (pItem);
	}
	CD_APPLET_LEAVE ();
}

static void on_new_item_status (DBusGProxy *proxy_item, const gchar *cStatus, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	//g_print ("=== %s (%s)\n", __func__, cStatus);
	
	// get the new status
	CDStatusEnum iPrevStatus = pItem->iStatus;
	pItem->iStatus = _find_status (cStatus);
	if (pItem->iStatus == iPrevStatus)
		CD_APPLET_LEAVE ();
	
	// update the item
	if ((iPrevStatus == CD_STATUS_PASSIVE || pItem->iStatus == CD_STATUS_PASSIVE)
	&& myConfig.bHideInactive)  // status was/is passive => hide/show the item.
	{
		if (myConfig.bCompactMode)
		{
			cd_satus_notifier_reload_compact_mode ();
		}
		else
		{
			if (pItem->iStatus == CD_STATUS_PASSIVE)  // remove passive item
			{
				Icon *pIcon = cd_satus_notifier_get_icon_from_item (pItem);
				CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);
			}
			else  // add newly active item
			{
				Icon *pIcon = cd_satus_notifier_create_icon_for_item (pItem);
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
			}
		}
	}
	else  // status has changed => image has changed too.
	{
		cd_satus_notifier_update_item_image (pItem);
	}
	
	CD_APPLET_LEAVE ();
}


static void on_new_item_label (DBusGProxy *proxy_item, const gchar *cLabel, const gchar *cLabelGuide, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	cd_debug ("=== %s (%s, %s)", __func__, cLabel, cLabelGuide);
	
	g_free (pItem->cLabel);
	pItem->cLabel = g_strdup (cLabel);
	g_free (pItem->cLabelGuide);
	pItem->cLabelGuide = g_strdup (cLabelGuide);
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_theme_path (DBusGProxy *proxy_item, const gchar *cNewThemePath, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	//g_print ("=== %s (%s)\n", __func__, cNewThemePath);
	
	if (g_strcmp0 (cNewThemePath, pItem->cIconThemePath) != 0)
	{
		if (pItem->cIconThemePath != NULL)  // if the item previously provided a theme, remove it first.
			cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
		g_free (pItem->cIconThemePath);
		pItem->cIconThemePath = g_strdup (cNewThemePath);
		
		cd_satus_notifier_update_item_image (pItem);
	}
	
	CD_APPLET_LEAVE ();
}


static void on_new_item_title (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	//g_print ("=== %s ()\n", __func__);
	
	g_free (pItem->cTitle);
	pItem->cTitle = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "Title");
	cd_debug ("===  new title : %s", pItem->cTitle);
	
	//cairo_dock_set_icon_name (cTitle, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
	
	CD_APPLET_LEAVE ();
}

static void on_new_item_overlay_icon (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	//g_print ("=== %s ()\n", __func__);
	
	g_free (pItem->cOverlayIconName);
	pItem->cOverlayIconName = cairo_dock_dbus_get_property_as_string (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "OverlayIconName");
	//g_print ("===  new overlay : %s\n", pItem->cOverlayIconName);
	
	/*if (pIcon->pIconBuffer)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, pIcon->cFileName, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_destroy (pIconContext);
		
		if (pItem->cOverlayIconName != NULL)
		{
			CairoEmblem *pEmblem = cairo_dock_make_emblem (pItem->cOverlayIconName, pIcon);
			cairo_dock_set_emblem_position (pEmblem, CAIRO_DOCK_EMBLEM_MIDDLE);
			cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			cairo_dock_free_emblem (pEmblem);
		}
	}*/
	
	CD_APPLET_LEAVE ();
}



static void on_new_item_tooltip (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	//g_print ("=== %s ()\n", __func__);
	
	cd_free_tooltip (pItem->pToolTip);
	pItem->pToolTip = NULL;
	
	//cairo_dock_remove_dialog_if_any (pIcon);
	
	GValueArray *pToolTipTab = cairo_dock_dbus_get_property_as_boxed (pItem->pProxyProps, CD_STATUS_NOTIFIER_ITEM_IFACE, "ToolTip");
	if (pToolTipTab)
	{
		pItem->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
		
		//if (pItem->pToolTip && pItem->pToolTip->cMessage != NULL)
		//	_show_item_tooltip (pIcon, pItem);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_item_proxy_destroyed (DBusGProxy *proxy_item, CDStatusNotifierItem *pItem)
{
	if (pItem->bInvalid)
		return;
	CD_APPLET_ENTER;
	//g_print ("=== this item (%s) was suddenly removed\n", __func__, pItem->cService);
	
	myData.pItems = g_list_remove (myData.pItems, pItem);
	
	if (myConfig.bCompactMode)
	{
		cd_satus_notifier_reload_compact_mode ();
	}
	else
	{
		Icon *pIcon = cd_satus_notifier_get_icon_from_item (pItem);
		CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);
	}
	
	cd_free_item (pItem);
	CD_APPLET_LEAVE ();
}

static gboolean _update_icon_delayed (CDStatusNotifierItem *pItem)
{
	cd_debug ("");
	if (pItem->cIconName != NULL)
	{
		cd_satus_notifier_update_item_image (pItem);
	}
	pItem->iSidUpdateIcon = 0;
	return FALSE;
}
gchar *cd_satus_notifier_search_item_icon_s_path (CDStatusNotifierItem *pItem, gint iSize)
{
	g_return_val_if_fail (pItem != NULL, NULL);
	gchar *cImageName = (pItem->iStatus == CD_STATUS_NEEDS_ATTENTION ? pItem->cAttentionIconName: pItem->cIconName);
	
	gchar *cIconPath = NULL;
	if (pItem->cIconThemePath != NULL)  // workaround pour des applis telles que dropbox qui trouvent malin de specifier des icones avec des noms hyper generiques (idle.png).
	{
		cIconPath = g_strdup_printf ("%s/%s", pItem->cIconThemePath, cImageName);
		if (! g_file_test (cIconPath, G_FILE_TEST_EXISTS))
		{
			g_free (cIconPath);
			cIconPath = NULL;
		}
	}
	
	if (cIconPath == NULL)
	{
		cIconPath = cairo_dock_search_icon_s_path (cImageName, iSize);
		if (cIconPath == NULL)  // in case we have a buggy app, try some heuristic
		{
			cIconPath = cairo_dock_search_icon_s_path (pItem->cId, iSize);
			if (cIconPath == NULL && pItem->pSurface == NULL)  // only use the fallback icon if the item is still empty (to not have an invisible item).
			{
				cIconPath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
			
			// skype strikes again ! on startup, it indicates its icon theme path (a temporary folder in /tmp); BUT it copies the icons after, so for a few seconds, the icons it tells us don't exist.
			// so we trigger an update in a few seconds.
			if (pItem->iSidUpdateIcon == 0)
				pItem->iSidUpdateIcon = g_timeout_add_seconds (7, (GSourceFunc)_update_icon_delayed, pItem);
		}
	}
	else if (pItem->iSidUpdateIcon != 0)  // we found an icon, discard any pending update.
	{
		g_source_remove (pItem->iSidUpdateIcon);
		pItem->iSidUpdateIcon = 0;
	}
	
	return cIconPath;
}

static void _cd_cclosure_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	typedef void (*GMarshalFunc_VOID__STRING_STRING) (
		gpointer   data1,
		gchar      *arg_1,
		gchar      *arg_2,
		gpointer   data2);
	register GMarshalFunc_VOID__STRING_STRING callback;
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
	callback = (GMarshalFunc_VOID__STRING_STRING) (marshal_data ? marshal_data : cc->callback);
	
	g_return_if_fail (callback != NULL);
	g_return_if_fail (G_VALUE_HOLDS_STRING (param_values + 1));
	g_return_if_fail (G_VALUE_HOLDS_STRING (param_values + 2));
	
	callback (data1,
		(char*) g_value_get_string (param_values + 1),
		(char*) g_value_get_string (param_values + 2),
		data2);
}

CDStatusNotifierItem *cd_satus_notifier_create_item (const gchar *cService, const gchar *cObjectPath)
{
	g_return_val_if_fail  (cService != NULL, NULL);
	cd_debug ("=== %s (%s, %s)", __func__, cService, cObjectPath);
	
	// avoid creating an item that already exists. This can happen in the following case (skype):
	// watcher starts -> dock registers to it   -> dock asks the items - - - - - - - - - - - - - - - - - -> dock receives the items -> skype item is already here !
	//                -> skype creates its item -> 'new-item' is emitted -> dock receives the signal -> creates the item
	if (cd_satus_notifier_find_item_from_service (cService) != NULL)
	{
		cd_debug ("The service %s / %s is already listed, skip it", cService, cObjectPath);
		return NULL;
	}
	
	gchar *str = strchr (cService, '/');  // just to be sure.
	if (str)
		*str = '\0';
	
	// special case for Ubuntu indicators: we don't know their object path.
	gchar *cRealObjectPath = NULL;
	if (cObjectPath != NULL && strncmp (cObjectPath, CD_INDICATOR_APPLICATION_ITEM_OBJ, strlen (CD_INDICATOR_APPLICATION_ITEM_OBJ)) == 0 && g_str_has_suffix (cObjectPath, "/Menu"))
	{
		// I think this is because this path is actually the menu path, and fortunately it's just under the item object's path.
		const gchar *str = strrchr (cObjectPath, '/');
		if (str)
		{
			cRealObjectPath = g_strndup (cObjectPath, str - cObjectPath);
		}
	}
	else if (cObjectPath == NULL || *cObjectPath == '\0')  // no path, let's assume it's the common one.
	{
		cObjectPath = CD_STATUS_NOTIFIER_ITEM_OBJ;
	}
	
	//g_print ("=== %s (cObjectPath: %s)\n", __func__, cRealObjectPath ? cRealObjectPath : cObjectPath);
	
	//\_________________ get the properties of the item.
	DBusGProxy *pProxyItemProp = cairo_dock_create_new_session_proxy (
		cService,
		cRealObjectPath ? cRealObjectPath : cObjectPath,
		DBUS_INTERFACE_PROPERTIES);
	if (pProxyItemProp == NULL)
		return NULL;
	//g_print ("=== owner : %s\n", dbus_g_proxy_get_bus_name (pProxyItemProp));

	//cd_debug ("%s, %s, %s", cService, cObjectPath, dbus_g_proxy_get_bus_name (pProxyItemProp));

	//g_print ("=== getting properties ...\n");
	GHashTable *hProps = cairo_dock_dbus_get_all_properties (pProxyItemProp, CD_STATUS_NOTIFIER_ITEM_IFACE);
	if (hProps == NULL)
		return NULL;
	
	// properties supported by KDE and Ubuntu.
	GValue *v;
	const gchar *cId = NULL;
	v = g_hash_table_lookup (hProps, "Id");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cId = g_value_get_string (v);
	}
	cd_debug ("===   ID '%s'", cId);
	
	const gchar *cCategory = NULL;
	v = g_hash_table_lookup (hProps, "Category");  // (ApplicationStatus, Communications, SystemServices, Hardware) -> fOrder
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cCategory = g_value_get_string (v);
	}
	//g_print ("===   Category '%s'\n", cCategory);
	
	const gchar *cStatus = NULL;
	v = g_hash_table_lookup (hProps, "Status");  // (Passive, Active, NeedsAttention) -> demands attention
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cStatus = g_value_get_string (v);
	}
	//g_print ("===   Status '%s'\n", cStatus);
	
	const gchar *cIconName = NULL;
	v = g_hash_table_lookup (hProps, "IconName");  // -> cIFileName
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cIconName = g_value_get_string (v);
	}
	cd_debug ("===   IconName '%s'", cIconName);
	
	const gchar *cIconThemePath = NULL;
	v = g_hash_table_lookup (hProps, "IconThemePath");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cIconThemePath = g_value_get_string (v);
	}
	cd_debug ("===   IconThemePath '%s'", cIconThemePath);
	
	const gchar *cAttentionIconName = NULL;
	v = g_hash_table_lookup (hProps, "AttentionIconName");  // -> keep for demands of attention
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cAttentionIconName = g_value_get_string (v);
	}
	//g_print ("===   AttentionIconName '%s'\n", cAttentionIconName);
	
	const gchar *cMenuPath = NULL;
	v = g_hash_table_lookup (hProps, "Menu");  // object path to a dbus-menu
	if (v && G_VALUE_HOLDS_BOXED(v))
	{
		cMenuPath = (gchar*) g_value_get_boxed (v);
	}
	cd_debug ("===   cMenuPath '%s'", cMenuPath);
	
	// properties supported by Ubuntu.
	gint iPosition = -1;
	v = g_hash_table_lookup (hProps, "XAyatanaOrderingIndex");
	if (v && G_VALUE_HOLDS_UINT(v))
	{
		iPosition = g_value_get_uint (v);
	}
	cd_debug ("===   iPosition '%d'", iPosition);
	// wrong values from the service !
	iPosition = -1;
	
	const gchar *cLabel = NULL;
	v = g_hash_table_lookup (hProps, "XAyatanaLabel");
	if (v && G_VALUE_HOLDS_STRING(v))
	{
		cLabel = g_value_get_string (v);
	}
	cd_debug ("===   cLabel '%s'", cLabel);
	
	const gchar *cLabelGuide = NULL;
	v = g_hash_table_lookup (hProps, "XAyatanaLabelGuide");
	if (v && G_VALUE_HOLDS_STRING(v))
	{
		cLabelGuide = g_value_get_string (v);
	}
	//g_print ("===   cLabelGuide '%s'\n", cLabelGuide);

	const gchar *cAccessibleDesc = NULL;
	v = g_hash_table_lookup (hProps, "IconAccessibleDesc");
	if (v && G_VALUE_HOLDS_STRING(v))
	{
		cAccessibleDesc = g_value_get_string (v);
	} // Updated with ApplicationIconChanged
	
	// properties supported by KDE.
	const gchar *cTitle = NULL;
	v = g_hash_table_lookup (hProps, "Title");  // -> cName
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cTitle = g_value_get_string (v);
	}
	cd_debug ("===   Title '%s'", cTitle);
	
	v = g_hash_table_lookup (hProps, "WindowId");
	guint iWindowId = 0;
	if (v && G_VALUE_HOLDS_UINT(v))
	{
		iWindowId = g_value_get_uint (v);
	}
	//g_print ("===   WindowId '%d'\n", iWindowId);
	
	const gchar *cOverlayIconName = NULL;
	v = g_hash_table_lookup (hProps, "OverlayIconName");  // -> emblem
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cOverlayIconName = g_value_get_string (v);
	}
	//g_print ("===   OverlayIconName '%s'\n", cOverlayIconName);
	
	const gchar *cAttentionMovieName = NULL;
	v = g_hash_table_lookup (hProps, "AttentionMovieName");  // -> idem
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cAttentionMovieName = g_value_get_string (v);
	}
	//g_print ("===   AttentionMovieName '%s'\n", cAttentionMovieName);
	
	GValueArray *pToolTipTab = NULL;
	v = g_hash_table_lookup (hProps, "ToolTip");
	if (v && G_VALUE_HOLDS_BOXED (v))
	{
		pToolTipTab = g_value_get_boxed (v);
	}
	
	gboolean bItemIsMenu = FALSE;  // "when is true the dbusmenu will be shown instead of emitting Activate()"
	v = g_hash_table_lookup (hProps, "ItemIsMenu");
	if (v && G_VALUE_HOLDS_BOOLEAN (v))
	{
		bItemIsMenu = g_value_get_boolean (v);
	}
	
	DBusGProxy *pProxyItem = cairo_dock_create_new_session_proxy (
		cService,
		cRealObjectPath ? cRealObjectPath : cObjectPath,
		CD_STATUS_NOTIFIER_ITEM_IFACE);
	if (pProxyItem == NULL)
		return NULL;
	
	//\_________________ create a new item.
	CDStatusNotifierItem *pItem = g_new0 (CDStatusNotifierItem, 1);
	pItem->cService = g_strdup (cService);
	pItem->pProxyProps = pProxyItemProp;
	pItem->pProxy = pProxyItem;
	pItem->cId = g_strdup (cId);
	pItem->iPosition = iPosition;
	pItem->cTitle = g_strdup (cTitle);
	pItem->cLabel = g_strdup (cLabel);
	pItem->cLabelGuide = g_strdup (cLabelGuide);
	pItem->cAccessibleDesc = g_strdup (cAccessibleDesc);
	///pItem->cMenuPath = (cMenuPath ? g_strdup (cMenuPath) : g_strdup (cObjectPath));
	pItem->cMenuPath = g_strdup (cMenuPath);  // if NULL, we'll just send the ContextMenu() signal.
	pItem->iWindowId = iWindowId;
	pItem->iCategory = _find_category (cCategory);
	pItem->iStatus = _find_status (cStatus);
	pItem->cIconName = g_strdup (cIconName);
	pItem->cIconThemePath = g_strdup (cIconThemePath);
	pItem->cAttentionIconName = g_strdup (cAttentionIconName);
	pItem->cAttentionMovieName = g_strdup (cAttentionMovieName);
	pItem->cOverlayIconName = g_strdup (cOverlayIconName);
	pItem->bItemIsMenu = bItemIsMenu;
	if (pToolTipTab)
	{
		pItem->pToolTip = _make_tooltip_from_dbus_struct (pToolTipTab);
	}
	if (pItem->cIconThemePath && *pItem->cIconThemePath != '\0')  // on le rajoute au theme d'icones par defaut; comme le launcher-manager va deja chercher dedans pour charger l'icone, on n'a rien d'autre a faire.
	{
		cd_satus_notifier_add_theme_path (pItem->cIconThemePath);
	}
	
	// build the dbusmenu right now, so that the menu is complete when the user first click on the item (otherwise, the menu is not placed correctly).
	cd_satus_notifier_build_item_dbusmenu (pItem);
	
	//\_________________ track any changes in the item.
	// signals supported by both.
	dbus_g_proxy_add_signal(pProxyItem, "NewStatus",
		G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewStatus",
		G_CALLBACK(on_new_item_status), pItem, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewIcon",
		G_CALLBACK(on_new_item_icon), pItem, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewAttentionIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewAttentionIcon",
		G_CALLBACK(on_new_item_attention_icon), pItem, NULL);
	
	// signals supported by Ubuntu.
	dbus_g_object_register_marshaller(_cd_cclosure_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);	
	dbus_g_proxy_add_signal(pProxyItem, "XAyatanaNewLabel",
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "XAyatanaNewLabel",
		G_CALLBACK(on_new_item_label), pItem, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewIconThemePath",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewIconThemePath",
		G_CALLBACK(on_new_item_theme_path), pItem, NULL);
	
	// signals supported by KDE.
	dbus_g_proxy_add_signal(pProxyItem, "NewOverlayIcon",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewOverlayIcon",
		G_CALLBACK(on_new_item_overlay_icon), pItem, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewTitle",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewTitle",
		G_CALLBACK(on_new_item_title), pItem, NULL);
	
	dbus_g_proxy_add_signal(pProxyItem, "NewToolTip",
		G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(pProxyItem, "NewToolTip",
		G_CALLBACK(on_new_item_tooltip), pItem, NULL);
	
	g_signal_connect (G_OBJECT(pProxyItem), "destroy", G_CALLBACK (_on_item_proxy_destroyed), pItem);  // attention, dangereux car on va etre appele lorsqu'on detruit un item.
	
	g_hash_table_destroy (hProps);
	g_free (cRealObjectPath);
	return pItem;
}

static gboolean _on_draw_menu_reposition (GtkWidget *pWidget, G_GNUC_UNUSED gpointer useless, CDStatusNotifierItem *pItem);

void cd_free_item (CDStatusNotifierItem *pItem)
{
	if (pItem == NULL)
		return;
	pItem->bInvalid = TRUE;
	if (pItem->iSidPopupTooltip != 0)
		g_source_remove (pItem->iSidPopupTooltip);
	if (pItem->iSidUpdateIcon != 0)
		g_source_remove (pItem->iSidUpdateIcon);
	if (pItem->cIconThemePath)
		cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
	if (pItem->pMenu != NULL)
		g_object_unref (pItem->pMenu);  // will remove the 'reposition' callback too.
	g_object_unref (pItem->pProxy);
	g_object_unref (pItem->pProxyProps);
	g_free (pItem->cService);
	g_free (pItem->cId);
	g_free (pItem->cIconName);
	g_free (pItem->cAttentionIconName);
	g_free (pItem->cLabel);
	g_free (pItem->cLabelGuide);
	g_free (pItem->cAccessibleDesc);
	g_free (pItem->cTitle);
	g_free (pItem->cAttentionMovieName);
	g_free (pItem->cOverlayIconName);
	cd_free_tooltip (pItem->pToolTip);
	cairo_surface_destroy (pItem->pSurface);
	g_free (pItem);
}


static void _load_item_image (Icon *icon)
{
	int iWidth = icon->iImageWidth;
	int iHeight = icon->iImageHeight;
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_get_item_from_icon (icon);
	gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem, MAX (iWidth, iHeight));
	if (cIconPath != NULL && *cIconPath != '\0')
		icon->pIconBuffer = cairo_dock_create_surface_from_image_simple (cIconPath,
			iWidth,
			iHeight);
	g_free (cIconPath);
}
Icon *cd_satus_notifier_create_icon_for_item (CDStatusNotifierItem *pItem)
{
	g_return_val_if_fail (pItem != NULL, NULL);
	Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (pItem->cTitle?pItem->cTitle:pItem->cId),
		g_strdup (pItem->cIconName),
		g_strdup (pItem->cService),
		NULL,
		pItem->iPosition > -1 ? pItem->iPosition : (int)pItem->iCategory);
	pIcon->iface.load_image = _load_item_image;  /// a voir...
	return pIcon;
}


CDStatusNotifierItem *cd_satus_notifier_get_item_from_icon (Icon *pIcon)
{
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pIcon->cCommand && strcmp (pIcon->cCommand, pItem->cService) == 0)
			return pItem;
	}
	return NULL;
}

Icon *cd_satus_notifier_get_icon_from_item (CDStatusNotifierItem *pItem)
{
	//g_print ("=== %s (%s)\n", __func__, pItem->cService);
	GList *ic, *pIcons = CD_APPLET_MY_ICONS_LIST;
	Icon *pIcon;
	for (ic = pIcons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		//g_print ("===   %s \n", pIcon->cCommand);
		if (pIcon->cCommand && strcmp (pIcon->cCommand, pItem->cService) == 0)
		{
			return pIcon;
		}
	}
	return NULL;
}


static gboolean _on_draw_menu_reposition (GtkWidget *pWidget, G_GNUC_UNUSED gpointer useless, CDStatusNotifierItem *pItem)
{
	g_return_val_if_fail (pItem != NULL, FALSE);

	int iMenuWidth = gtk_widget_get_allocated_width (pWidget);

	if (pItem->iMenuWidth != iMenuWidth)  // if the width has changed, reposition the menu to be sure it won't out of the screen.
	{
		pItem->iMenuWidth = iMenuWidth;
		gtk_menu_reposition (GTK_MENU (pWidget));
	}
	
	return FALSE; // FALSE to propagate the event further.
}
void cd_satus_notifier_build_item_dbusmenu (CDStatusNotifierItem *pItem)
{
	if (pItem->pMenu == NULL)  // menu not yet built
	{
		if (pItem->cMenuPath != NULL && *pItem->cMenuPath != '\0' && strcmp (pItem->cMenuPath, "/NO_DBUSMENU") != 0)  // hopefully, if the item doesn't provide a dbusmenu, it will not set something different as these 2 choices  (ex.: Klipper).
		{
			pItem->pMenu = dbusmenu_gtkmenu_new ((gchar *)pItem->cService, (gchar *)pItem->cMenuPath);
			if (g_object_is_floating (pItem->pMenu))  // claim ownership on the menu.
				g_object_ref_sink (pItem->pMenu);
			/* Position of the menu: GTK doesn't do its job :-/
			 * e.g. with Dropbox: the menu is out of the screen every time
			 * something has changed in this menu (it displays 'connecting',
			 * free space available, etc.) -> we need to reposition it.
			 * (maybe it's due to a delay because Python and DBus are slower...)
			 * We can't watch the 'configure' event (which should be triggered
			 * each time the menu is resized) because it seems this notification
			 * is not send...
			 * This is why we need to watch the 'draw' event...
			 */
			g_signal_connect (G_OBJECT (pItem->pMenu),
				#if (GTK_MAJOR_VERSION < 3)
				"expose-event",
				#else
				"draw",
				#endif
				G_CALLBACK (_on_draw_menu_reposition),
				pItem);
		}
	}
}

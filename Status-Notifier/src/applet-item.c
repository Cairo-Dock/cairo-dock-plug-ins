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

static CDToolTip *_make_tooltip_from_dbus_struct (GVariant *pToolTipTab)
{
	CDToolTip *pToolTip = NULL;
	if (pToolTipTab)
	{
		// type: "(sa(iiay)ss)", checked by the caller
		pToolTip = g_new0 (CDToolTip, 1);
		GVariant *v = g_variant_get_child_value (pToolTipTab, 0);
		pToolTip->cIconName = g_variant_dup_string (v, NULL);
		g_variant_unref (v);
		
		v = g_variant_get_child_value (pToolTipTab, 2);
		pToolTip->cTitle = g_variant_dup_string (v, NULL);
		g_variant_unref (v);
		
		v = g_variant_get_child_value (pToolTipTab, 3);
		pToolTip->cMessage = g_variant_dup_string (v, NULL);
		g_variant_unref (v);
		
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
	gldi_dialog_show_temporary_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 4000, pItem->pToolTip->cIconName);
	g_free (cText);
}
*/
/*static void _show_item_status (Icon *pIcon, CDStatusNotifierItem *pItem)
{
	switch (pItem->iStatus)
	{
		case CD_STATUS_PASSIVE :
			pIcon->fAlpha = 0.5;
			gldi_icon_stop_attention (pIcon, myIcon->pSubDock);
			cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		break;
		case CD_STATUS_ACTIVE :
		default:
			pIcon->fAlpha = 1.;
			gldi_icon_stop_attention (pIcon, myIcon->pSubDock);
			cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		break;
		case CD_STATUS_NEEDS_ATTENTION:
			pIcon->fAlpha = 1.;
			gldi_icon_request_attention (pIcon, "rotate", 60);
		break;
	}
}*/

static gboolean _get_string_from_variant (GVariant *v2, gchar **str)
{
	gboolean ret = FALSE;
	if (g_variant_is_of_type (v2, G_VARIANT_TYPE ("s")))
	{
		g_free (*str);
		*str = g_variant_dup_string (v2, NULL);
		ret = TRUE;
	}
	else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (v2));
	g_variant_unref (v2);
	return ret;
}

static gboolean _parse_string_prop (GVariant *res, gchar **str)
{
	gboolean ret = FALSE;
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *v1 = g_variant_get_child_value (res, 0);
		GVariant *v2 = g_variant_get_variant (v1);
		ret = _get_string_from_variant (v2, str);
		g_variant_unref (v1);
	}
	else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (res));
	g_variant_unref (res);
	return ret;
}

/// General callback for updating string properties without any further action
static void _got_string_prop (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		// We do not show a warning if the operation was canceled
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting string property for item: %s", err->message);
		g_error_free (err);
	}
	else _parse_string_prop (res, (gchar**)ptr);
	CD_APPLET_LEAVE ();
}

/// Special casing for IconAccessibleDesc since this property might not exist and we should avoid showing an error
static void _got_icon_desc (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		// We do not show a warning as the error can come from the property not being present
		g_error_free (err);
	}
	else _parse_string_prop (res, (gchar**)ptr);
	CD_APPLET_LEAVE ();
}

static gboolean _update_icon_description (CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	
	g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "IconAccessibleDesc"),
		G_DBUS_CALL_FLAGS_NO_AUTO_START,
		-1,
		pItem->pCancel,
		_got_icon_desc,
		&pItem->cAccessibleDesc);
	
	pItem->iSidUpdateIcon = 0;
	
	CD_APPLET_LEAVE (FALSE);
}

static void _on_new_item_icon (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting icon name: %s", err->message);
		g_error_free (err);
	}
	else
	{
		if (_parse_string_prop (res, &pItem->cIconName))
		{
			if (pItem->iStatus != CD_STATUS_NEEDS_ATTENTION)
				cd_satus_notifier_update_item_image (pItem);
			
			// the label may have changed too, but avoid updating it too often
			if (pItem->iSidUpdateIcon == 0)
				pItem->iSidUpdateIcon = g_timeout_add (400, (GSourceFunc)_update_icon_description, pItem);
		}
	}
	CD_APPLET_LEAVE ();
}

static void _on_new_item_attention_icon (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting icon name: %s", err->message);
		g_error_free (err);
	}
	else
	{
		if (_parse_string_prop (res, &pItem->cAttentionIconName))
		{
			if (pItem->iStatus == CD_STATUS_NEEDS_ATTENTION)
				cd_satus_notifier_update_item_image (pItem);
			
			// the label may have changed too, but avoid updating it too often
			if (pItem->iSidUpdateIcon == 0)
				pItem->iSidUpdateIcon = g_timeout_add (400, (GSourceFunc)_update_icon_description, pItem);
		}
	}
	CD_APPLET_LEAVE ();
}

static void on_new_item_status (const gchar *cStatus, CDStatusNotifierItem *pItem)
{
	// get the new status
	CDStatusEnum iPrevStatus = pItem->iStatus;
	pItem->iStatus = _find_status (cStatus);
	if (pItem->iStatus == iPrevStatus) return;
	
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
				pItem->pIcon = NULL;
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
}


static void on_new_item_label (const gchar *cLabel, const gchar *cLabelGuide, CDStatusNotifierItem *pItem)
{
	cd_debug ("=== %s (%s, %s)", __func__, cLabel, cLabelGuide);
	
	g_free (pItem->cLabel);
	pItem->cLabel = g_strdup (cLabel);
	g_free (pItem->cLabelGuide);
	pItem->cLabelGuide = g_strdup (cLabelGuide);
}

static void _on_new_item_icon_theme (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting icon name: %s", err->message);
		g_error_free (err);
	}
	else
	{
		char *cNewThemePath = NULL;
		if (_parse_string_prop (res, &cNewThemePath))
		{
			if (g_strcmp0 (cNewThemePath, pItem->cIconThemePath) != 0)
			{
				if (pItem->cIconThemePath != NULL)  // if the item previously provided a theme, remove it first.
					cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
				g_free (pItem->cIconThemePath);
				pItem->cIconThemePath = cNewThemePath; // can be NULL
				
				cd_satus_notifier_update_item_image (pItem);
			}
			else g_free (cNewThemePath);
		}
	}
	CD_APPLET_LEAVE ();
}

static void _on_new_item_tooltip (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting icon name: %s", err->message);
		g_error_free (err);
	}
	else
	{
		if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
		{
			GVariant *v1 = g_variant_get_child_value (res, 0);
			GVariant *v2 = g_variant_get_variant (v1);
			if (g_variant_is_of_type (v2, G_VARIANT_TYPE ("(sa(iiay)ss)")))
			{
				cd_free_tooltip (pItem->pToolTip);
				pItem->pToolTip = _make_tooltip_from_dbus_struct (v2);
			}
			else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (v2));
			g_variant_unref (v2);
			g_variant_unref (v1);
		}
		else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (res));
		g_variant_unref (res);
	}
	CD_APPLET_LEAVE ();
}

static void _on_item_name_owner_changed (G_GNUC_UNUSED GObject *pObj, G_GNUC_UNUSED GParamSpec *par, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	
	char *cNameOwner = g_dbus_proxy_get_name_owner (pItem->pProxy);
	if (!cNameOwner)
	{
		cd_status_notifier_remove_item_in_list (pItem);
		
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
	}
	else g_free (cNameOwner);
	
	CD_APPLET_LEAVE ();
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
		// in case we have a buggy app, try some heuristic
		if (cIconPath == NULL && pItem->pFallbackIcon == NULL && (pItem->iStatus != CD_STATUS_NEEDS_ATTENTION || pItem->pFallbackIconAttention == NULL))
		{
			cIconPath = cairo_dock_search_icon_s_path (pItem->cId, iSize);
			if (cIconPath == NULL && pItem->pSurface == NULL)  // only use the fallback icon if the item is still empty (to not have an invisible item).
			{
				cIconPath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
		}
	}
	
	return cIconPath;
}

static gboolean _parse_pixmap_property (GVariant *v, cairo_surface_t **pResult)
{
	// type: 'a(iiay)'
	if (g_variant_is_of_type (v, G_VARIANT_TYPE ("a(iiay)")))
	{
		GVariant *v2 = NULL;
		int w, h;
		
		GVariantIter iter;
		GVariant *v1;
		
		g_variant_iter_init (&iter, v);
		
		int iSize;
		if (myConfig.bCompactMode) iSize = myData.iItemSize;
		else
		{
			// try to guess icon size based on any existing icon
			GList *icons = CD_APPLET_MY_ICONS_LIST;
			if (icons)
			{
				Icon *pIcon = (Icon*)icons->data;
				iSize = cairo_dock_icon_get_allocated_width (pIcon);
			}
			else // just use the general config
				iSize = myIconsParam.iIconWidth * (1. + myIconsParam.fAmplitude);
		}
		
		// iterate over all offered icons, take the first one that is large enough
		while ((v1 = g_variant_iter_next_value (&iter)))
		{		
			g_variant_unref (v2);
			g_variant_get_child (v1, 0, "i", &w);
			g_variant_get_child (v1, 1, "i", &h);
			v2 = g_variant_get_child_value (v1, 2);
			g_variant_unref (v1);
			if (w >= iSize && h >= iSize)
				break;
		}
		
		if (!v2) return FALSE; // empty array
		
		gsize len;
		const void *data = g_variant_get_fixed_array (v2, &len, 1);
		if (len != 4*w*h) cd_warning ("Unexpected image size!");
		else
		{
			cairo_surface_t *s = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
			unsigned char *dest = cairo_image_surface_get_data (s);
			gsize stride = cairo_image_surface_get_stride (s);
			const uint32_t *src = (const uint32_t*)data;
			gsize i, j;
			for (i = 0; i < (gsize)h; i++)
			{
				uint32_t *dest_row = (uint32_t*)(dest + i * stride);
				for (j = 0; j < (gsize)w; j++) dest_row[j] = g_ntohl (src[i * w + j]); // note: src is in "network byte order"
			}
			cairo_surface_mark_dirty (s);
			
			cairo_surface_destroy (*pResult);
			*pResult = s;
		}
		g_variant_unref (v2);
		return TRUE;
	}
	cd_warning ("Unexpected result type: %s", g_variant_get_type_string (v));
	return FALSE;
}

static void _on_new_item_icon_pixmap2 (CDStatusNotifierItem *pItem, cairo_surface_t **ptr, GVariant *res)
{
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *v1 = g_variant_get_child_value (res, 0);
		GVariant *v2 = g_variant_get_variant (v1);
		if (_parse_pixmap_property (v2, (cairo_surface_t**)ptr))
			cd_satus_notifier_update_item_image (pItem);
		// warning already shown if type does not match
		g_variant_unref (v2);
		g_variant_unref (v1);
	}
	else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (res));
	g_variant_unref (res);
}

static void _on_new_item_icon_pixmap (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		// Do not display a warning, it is OK if the pixmap properties do not exist
		g_error_free (err);
	}
	else
	{
		CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
		_on_new_item_icon_pixmap2 (pItem, &pItem->pFallbackIcon, res);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_new_item_icon_attention_pixmap (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		// Do not display a warning, it is OK if the pixmap properties do not exist
		g_error_free (err);
	}
	else
	{
		CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)ptr;
		_on_new_item_icon_pixmap2 (pItem, &pItem->pFallbackIconAttention, res);
	}
	
	CD_APPLET_LEAVE ();
}


static void _item_signal (G_GNUC_UNUSED GDBusProxy *pProxy, G_GNUC_UNUSED const gchar *cSender, const gchar *cSignal,
	GVariant* pPar, CDStatusNotifierItem *pItem)
{
	CD_APPLET_ENTER;
	
	// signals supported by both.
	if (! strcmp (cSignal, "NewStatus"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("s")))
			on_new_item_status (g_variant_get_string (pPar, NULL), pItem);
	}
	// weirdly, it seems that we do not get the PropertiesChanged signal, so we need to get the properties ourselves
	else if (! strcmp (cSignal, "NewIcon"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "IconName"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_icon,
			pItem);
		
		// also update the IconPixmap property as this might have changed as well
		// (although it is wasteful to read it again if it did not)
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "IconPixmap"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_icon_pixmap,
			pItem);
	}
	else if (! strcmp (cSignal, "NewAttentionIcon"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "AttentionIconName"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_attention_icon,
			pItem);
		
		// also update the AttentionIconPixmap property as this might have changed as well
		// (although it is wasteful to read it again if it did not)
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "AttentionIconPixmap"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_icon_attention_pixmap,
			pItem);
	}
	
	// signals supported by Ubuntu.
	// TODO:
	//  - is this only a notification about change in two linked properties?
	//  - is this the correct GVariant type? (does any app actually emit this signal?)
	else if (! strcmp (cSignal, "XAyatanaNewLabel"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(ss)")))
		{
			GVariant *v1 = g_variant_get_child_value (pPar, 0);
			GVariant *v2 = g_variant_get_child_value (pPar, 0);
			on_new_item_label (g_variant_get_string (v1, NULL), g_variant_get_string (v2, NULL), pItem);
			g_variant_unref (v1);
			g_variant_unref (v2);
		}
	}
	else if (! strcmp (cSignal, "NewIconThemePath"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "IconThemePath"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_icon_theme,
			pItem);
	}
	else if (! strcmp (cSignal, "NewTitle"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "Title"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_got_string_prop,
			&pItem->cTitle);
	}
	else if (! strcmp (cSignal, "NewOverlayIcon"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "Title"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_got_string_prop,
			&pItem->cOverlayIconName);
	}
	else if (! strcmp (cSignal, "NewToolTip"))
	{
		g_dbus_proxy_call (pItem->pProxyProps, "Get", g_variant_new ("(ss)", CD_STATUS_NOTIFIER_ITEM_IFACE, "ToolTip"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			pItem->pCancel,
			_on_new_item_tooltip,
			pItem);
	}
	
	// !! TODO: NewMenu ??
	CD_APPLET_LEAVE ();
}


static void _item_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, gpointer data);

void cd_satus_notifier_create_item (CDStatusNotifierItem *pItem, const gchar *cObjectPath)
{
	if (pItem->cService == NULL)
	{
		cd_warning ("no service address provided!");
		cd_free_item (pItem);
		return;
	}
	
	// avoid creating an item that already exists. This can happen in the following case (skype):
	// watcher starts -> dock registers to it   -> dock asks the items - - - - - - - - - - - - - - - - - -> dock receives the items -> skype item is already here !
	//                -> skype creates its item -> 'new-item' is emitted -> dock receives the signal -> creates the item
	// note: however, we need to be careful as some apps provide multiple icons with the same service name and different object path
	const gchar *cObjectPathTmp = (cObjectPath && *cObjectPath) ? cObjectPath : CD_STATUS_NOTIFIER_ITEM_OBJ;
	if (cd_satus_notifier_find_item_from_service (pItem->cService, cObjectPathTmp) != NULL)
	{
		cd_debug ("The service %s / %s is already listed, skip it", pItem->cService, cObjectPath);
		cd_free_item (pItem);
		return;
	}
	// we save the object path we got for later lookup
	pItem->cObjectPath = g_strdup (cObjectPathTmp);
	
	gchar *str = strchr (pItem->cService, '/');  // just to be sure.
	if (str)
		*str = '\0';
	
	// special case for Ubuntu indicators: we don't know their object path.
	gchar *cRealObjectPath = NULL;
	if (cObjectPath != NULL && strncmp (cObjectPath, CD_INDICATOR_APPLICATION_ITEM_OBJ, strlen (CD_INDICATOR_APPLICATION_ITEM_OBJ)) == 0 &&
		(g_str_has_suffix (cObjectPath, "/Menu") || g_str_has_suffix (cObjectPath, "/MenuBar"))) // not sure if both are used, but just to be safe
	{
		// I think this is because this path is actually the menu path, and fortunately it's just under the item object's path.
		const gchar *str = strrchr (cObjectPath, '/');
		if (str)
		{
			cRealObjectPath = g_strndup (cObjectPath, str - cObjectPath);
		}
	}
	else if (cObjectPath != NULL && (!strcmp (cObjectPath, "/Menu") || !strcmp (cObjectPath, "/MenuBar")))
	{
		// ayatana-indicator-application.service has a weird idea of object paths (see above) that is incompatible
		// with the standard and not followed by normal apps, try to correct
		cObjectPath = CD_STATUS_NOTIFIER_ITEM_OBJ;
	}
	else if (cObjectPath == NULL || *cObjectPath == '\0')  // no path, let's assume it's the common one.
	{
		cObjectPath = CD_STATUS_NOTIFIER_ITEM_OBJ;
	}
	
	cd_debug ("cObjectPath: %s\n", cRealObjectPath ? cRealObjectPath : cObjectPath);
	
	pItem->pCancel = g_cancellable_new ();
	
	// add it in the list already so that we can properly destroy it in case it is removed before the call below completes
	cd_status_notifier_add_item_in_list (pItem);
	
	//\_________________ create the main proxy for this item
	g_dbus_proxy_new_for_bus (
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // GDBusInterfaceInfo
		pItem->cService,
		cRealObjectPath ? cRealObjectPath : cObjectPath,
		CD_STATUS_NOTIFIER_ITEM_IFACE,
		pItem->pCancel,
		_item_proxy_created,
		pItem
	);
	
	g_free (cRealObjectPath);
}

static void _item_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	CDStatusNotifierItem *pItem = (CDStatusNotifierItem*)data;
	
	GDBusProxy* pProxyItem = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	
	if (err)
	{
		if (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			// Not really an error, this just means that creating the proxy was cancelled; this can happen
			// if we got a signal about the app being removed, or if this applet is being unloaded.
			// In this case, pItem was already freed, we should not access it.
			g_error_free (err);
			CD_APPLET_LEAVE ();
		}
		
		cd_warning ("Error creating DBus proxy for item '%s': %s", pItem->cService, err->message);
		// in this case, we need to free our item
		cd_status_notifier_remove_item_in_list (pItem);
		cd_free_item (pItem);
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}

	pItem->pProxy = pProxyItem;
	// in this case, we have a valid DBus proxy, check its name owner just to be safe
	char *cNameOwner = g_dbus_proxy_get_name_owner (pProxyItem);
	if (!cNameOwner)
	{
		// This is not necessary an error, the app might have disconnected.
		cd_debug ("DBus proxy for item '%s' without name owner, removing", pItem->cService);
		cd_status_notifier_remove_item_in_list (pItem);
		cd_free_item (pItem);
		CD_APPLET_LEAVE ();
	}
	else g_free (cNameOwner);
	
	// We create a separate proxy for retrieving DBus properties when there is an update.
	// Note: we can use the _sync version since this call will not block (and should not fail).
	GDBusProxy *pProxyProps = g_dbus_proxy_new_for_bus_sync (
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
			G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | // there are no properties
			G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS, // and we do not get the PropertiesChanged signal anyway ...
		NULL, // GDBusInterfaceInfo
		pItem->cService,
		g_dbus_proxy_get_object_path (pProxyItem),
		DBUS_INTERFACE_PROPERTIES,
		NULL,
		&err
	);
	
	
	if (err)
	{
		cd_warning ("Error creating DBus proxy for item '%s': %s", pItem->cService, err->message);
		cd_status_notifier_remove_item_in_list (pItem);
		cd_free_item (pItem);
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	pItem->pProxyProps = pProxyProps;

	// We have an item, now fill out the expected properties.
	// properties supported by KDE and Ubuntu.
	GVariant *v;
	v = g_dbus_proxy_get_cached_property (pProxyItem, "Id");
	if (v) _get_string_from_variant (v, &pItem->cId);
	cd_debug ("===   ID '%s'", pItem->cId);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "Category");  // (ApplicationStatus, Communications, SystemServices, Hardware) -> fOrder
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("s")))
		{
			const gchar *cCategory = g_variant_get_string (v, NULL);
			pItem->iCategory = _find_category (cCategory);
		}
		g_variant_unref (v);			
	}
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "Status");  // (Passive, Active, NeedsAttention) -> demands attention
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("s")))
		{
			const gchar *cStatus = g_variant_get_string (v, NULL);
			pItem->iStatus = _find_status (cStatus);
		}
		g_variant_unref (v);			
	}
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "IconName");
	if (v) _get_string_from_variant (v, &pItem->cIconName); // note: will update icon name if the result is valid and will also free v
	cd_debug ("===   IconName '%s'", pItem->cIconName);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "IconThemePath");
	if (v) _get_string_from_variant (v, &pItem->cIconThemePath);
	cd_debug ("===   IconThemePath '%s'", pItem->cIconThemePath);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "AttentionIconName");
	if (v) _get_string_from_variant (v, &pItem->cAttentionIconName);
	cd_debug ("===   AttentionIconName '%s'", pItem->cAttentionIconName);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "IconPixmap");
	if (v)
	{
		_parse_pixmap_property (v, &pItem->pFallbackIcon);
		g_variant_unref (v);
	}
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "AttentionIconPixmap");
	if (v)
	{
		_parse_pixmap_property (v, &pItem->pFallbackIconAttention);
		g_variant_unref (v);
	}
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "Menu"); // object path to a dbus-menu
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("o")) || g_variant_is_of_type (v, G_VARIANT_TYPE ("s")))
		{
			g_free (pItem->cMenuPath); // note: the caller might have set a default value, we need to free it
			pItem->cMenuPath = g_variant_dup_string (v, NULL);  // if NULL, we'll just send the ContextMenu() signal.
		}
		g_variant_unref (v);
	}
	cd_debug ("===   MenuPath '%s'", pItem->cMenuPath);
	
	// properties supported by Ubuntu.
	v = g_dbus_proxy_get_cached_property (pProxyItem, "XAyatanaLabel");
	if (v) _get_string_from_variant (v, &pItem->cLabel);
	cd_debug ("===   Label '%s'", pItem->cLabel);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "XAyatanaLabelGuide");
	if (v) _get_string_from_variant (v, &pItem->cLabelGuide);
	cd_debug ("===   LabelGuide '%s'", pItem->cLabelGuide);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "IconAccessibleDesc"); // Updated with ApplicationIconChanged
	if (v) _get_string_from_variant (v, &pItem->cAccessibleDesc);
	
	// properties supported by KDE.
	v = g_dbus_proxy_get_cached_property (pProxyItem, "Title");
	if (v) _get_string_from_variant (v, &pItem->cTitle);
	cd_debug ("===   Title '%s'", pItem->cTitle);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "WindowId");
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("u")))
			pItem->iWindowId = g_variant_get_uint32 (v);
		else if (g_variant_is_of_type (v, G_VARIANT_TYPE ("i")))
			pItem->iWindowId = g_variant_get_int32 (v);
		g_variant_unref (v);
	}
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "OverlayIconName"); // not used currently
	if (v) _get_string_from_variant (v, &pItem->cOverlayIconName);
	cd_debug ("===   OverlayIconName '%s'", pItem->cOverlayIconName);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "AttentionMovieName");
	if (v) _get_string_from_variant (v, &pItem->cAttentionMovieName);
	cd_debug ("===   AttentionMovieName '%s'", pItem->cAttentionMovieName);
	
	v = g_dbus_proxy_get_cached_property (pProxyItem, "ToolTip");
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("(sa(iiay)ss)"))) // TODO: is this valid if the array is empty?
			pItem->pToolTip = _make_tooltip_from_dbus_struct (v);
		g_variant_unref (v);
	}
	
	// "when is true the dbusmenu will be shown instead of emitting Activate()"
	v = g_dbus_proxy_get_cached_property (pProxyItem, "ItemIsMenu");
	if (v)
	{
		if (g_variant_is_of_type (v, G_VARIANT_TYPE ("b")))
			pItem->bItemIsMenu = g_variant_get_boolean (v);
		g_variant_unref (v);
	}
	
	if (pItem->cIconThemePath && *pItem->cIconThemePath != '\0')  // on le rajoute au theme d'icones par defaut; comme le launcher-manager va deja chercher dedans pour charger l'icone, on n'a rien d'autre a faire.
	{
		cd_satus_notifier_add_theme_path (pItem->cIconThemePath);
	}
	
	// build the dbusmenu right now, so that the menu is complete when the user first clicks on the item (otherwise, the menu is not placed correctly).
	cd_satus_notifier_build_item_dbusmenu (pItem);
	
	//\_________________ track any changes in the item.
	g_signal_connect (G_OBJECT(pProxyItem), "g-signal", G_CALLBACK (_item_signal), pItem);
	
	// potential disconnect
	g_signal_connect (G_OBJECT(pProxyItem), "notify::g-name-owner", G_CALLBACK (_on_item_name_owner_changed), pItem); // attention, dangereux car on va etre appele lorsqu'on detruit un item.
	
	// now that we have a valid item, ensure that it is shown
	if (_item_is_visible (pItem))
	{
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
	
	CD_APPLET_LEAVE ();
}

void cd_free_item (CDStatusNotifierItem *pItem)
{
	if (pItem == NULL)
		return;
	if (pItem->iSidPopupTooltip != 0)
		g_source_remove (pItem->iSidPopupTooltip);
	if (pItem->iSidUpdateIcon != 0)
		g_source_remove (pItem->iSidUpdateIcon);
	if (pItem->cIconThemePath)
		cd_satus_notifier_remove_theme_path (pItem->cIconThemePath);
	if (pItem->pMenu != NULL)
		g_object_unref (pItem->pMenu);  // will remove the 'reposition' callback too.
	if (pItem->pProxy) g_object_unref (pItem->pProxy);
	if (pItem->pProxyProps) g_object_unref (pItem->pProxyProps);
	if (pItem->pCancel)
	{
		// this is to cancel any asynchronous operations, e.g. getting property updates
		g_cancellable_cancel (pItem->pCancel);
		g_object_unref (pItem->pCancel);
	}
	g_free (pItem->cService);
	g_free (pItem->cObjectPath);
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
	cairo_surface_destroy (pItem->pFallbackIcon);
	cairo_surface_destroy (pItem->pFallbackIconAttention);
	g_free (pItem);
}


static void _load_item_image (Icon *icon)
{
	int iWidth = cairo_dock_icon_get_allocated_width (icon);
	int iHeight = cairo_dock_icon_get_allocated_height (icon);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_get_item_from_icon (icon);
	gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem, MAX (iWidth, iHeight));
	if (cIconPath != NULL && *cIconPath != '\0')
	{
		cairo_surface_t *pSurface = cairo_dock_create_surface_from_image_simple (cIconPath,
			iWidth,
			iHeight);
		cairo_dock_load_image_buffer_from_surface (&icon->image, pSurface, iWidth, iHeight);
	}
	else
	{
		cairo_surface_t *pSurface = ((pItem->iStatus == CD_STATUS_NEEDS_ATTENTION) && pItem->pFallbackIconAttention) ?
					pItem->pFallbackIconAttention : pItem->pFallbackIcon;
		if (pSurface)
		{
			cairo_surface_t *pNewSurface = cairo_dock_create_blank_surface (iWidth, iHeight);
			cairo_t *pCairoContext = cairo_create (pNewSurface);
			double w = cairo_image_surface_get_width (pSurface);
			double h = cairo_image_surface_get_height (pSurface);
			double s = MIN (iWidth / w, iHeight / h);
			cairo_scale (pCairoContext, s, s);
			cairo_set_source_surface (pCairoContext, pSurface, 0., 0.);
			cairo_paint (pCairoContext);
			cairo_destroy (pCairoContext);
			
			cairo_dock_load_image_buffer_from_surface (&icon->image, pNewSurface, iWidth, iHeight);
		}
	}
	g_free (cIconPath);
}
Icon *cd_satus_notifier_create_icon_for_item (CDStatusNotifierItem *pItem)
{
	g_return_val_if_fail (pItem != NULL, NULL);
	Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (pItem->cTitle?pItem->cTitle:pItem->cId), // cName
		g_strdup (pItem->cIconName), // cFileName
		// cCommand -- this is only used for debugging, include both the DBus name and the object path
		// (note: multiple items can share the same DBus name, e.g. update-notifier and livepatch on Ubuntu)
		g_strdup_printf ("%s%s", pItem->cService, pItem->cObjectPath),
		NULL,
		pItem->iPosition > -1 ? pItem->iPosition : (int)pItem->iCategory);
	pIcon->iface.load_image = _load_item_image;  /// a voir...
	pItem->pIcon = pIcon;
	return pIcon;
}

static gboolean _icon_belongs_to_item (const Icon *pIcon, const CDStatusNotifierItem *pItem)
{
	return (pItem && pIcon && (pItem->pIcon == pIcon));
}

CDStatusNotifierItem *cd_satus_notifier_get_item_from_icon (Icon *pIcon)
{
	if (! pIcon->cCommand) return NULL;
	
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (_icon_belongs_to_item (pIcon, pItem)) return pItem;
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
		if (_icon_belongs_to_item (pIcon, pItem)) return pIcon;
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
			gldi_menu_init (GTK_WIDGET(pItem->pMenu), myIcon);
			/* Position of the menu: GTK doesn't do its job :-/
			 * e.g. with Dropbox: the menu is out of the screen every time
			 * something has changed in this menu (it displays 'connecting',
			 * free space available, etc.) -> we need to reposition it.
			 * (maybe it's due to a delay because Python and DBus are slower...)
			 * We can't watch the 'configure' event (which should be triggered
			 * each time the menu is resized) because it seems this notification
			 * is not sent...
			 * This is why we need to watch the 'draw' event...
			 */
			g_signal_connect (G_OBJECT (pItem->pMenu),
				"draw",
				G_CALLBACK (_on_draw_menu_reposition),
				pItem);
		}
	}
}

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
#include "applet-host-kde.h"
#include "applet-host-ias.h"

CDStatusNotifierItem * cd_satus_notifier_find_item_from_service (const gchar *cService, const gchar *cObjectPath)
{
	g_return_val_if_fail (cService != NULL && cObjectPath != NULL, NULL);
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (strcmp (pItem->cService, cService) == 0 && strcmp (pItem->cObjectPath, cObjectPath) == 0)
			return pItem;
	}
	return NULL;
}

CDStatusNotifierItem * cd_satus_notifier_find_item_from_position (int iPosition)
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


  ///////////////////////
 /// Add/remove item ///
///////////////////////

gint _compare_pos (gconstpointer a, gconstpointer b)
{
	const CDStatusNotifierItem *pItemA = (CDStatusNotifierItem*)a;
	const CDStatusNotifierItem *pItemB = (CDStatusNotifierItem*)b;
	if (pItemA->iPosition < pItemB->iPosition) return -1;
	if (pItemA->iPosition > pItemB->iPosition) return 1;
	return 0;
}

void cd_status_notifier_add_item_in_list (CDStatusNotifierItem *pItem)
{
	if (myData.pItems == NULL)
		gldi_icon_insert_in_container (myIcon, myContainer, ! CAIRO_DOCK_ANIMATE_ICON);

	myData.pItems = g_list_insert_sorted (myData.pItems, pItem, _compare_pos);
}

void cd_status_notifier_remove_item_in_list (CDStatusNotifierItem *pItem)
{
	myData.pItems = g_list_remove (myData.pItems, pItem);

	if (myData.pItems == NULL)
		gldi_icon_detach (myIcon);
}

void cd_satus_notifier_remove_item (const gchar *cService, const gchar *cObjectPath, int iPosition)
{
	CDStatusNotifierItem *pItem = NULL;
	if (cService) pItem = cd_satus_notifier_find_item_from_service (cService,
		(cObjectPath && *cObjectPath) ? cObjectPath : CD_STATUS_NOTIFIER_ITEM_OBJ);
	else pItem = cd_satus_notifier_find_item_from_position (iPosition);
	// not an error since we automatically remove items if they disconnect, but also get
	// removal notifications from the watcher
	if (pItem == NULL) return;
	
	cd_status_notifier_remove_item_in_list (pItem);
	
	if (! _item_is_visible (pItem))  // the item was passive, therefore not visible.
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
	
	cd_debug ("=== item %s removed", pItem->cTitle?pItem->cTitle:pItem->cLabel);
	cd_free_item (pItem);
}


  //////////////////////////
 /// Start/stop service ///
//////////////////////////

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
	
	// see if a Watcher and/or an Indicator Application Service (IAS) is on the bus.
	cd_satus_notifier_detect_watcher ();
	cd_satus_notifier_detect_ias ();
}


void cd_satus_notifier_stop_service (void)
{
	// disconnect from the watcher
	cd_satus_notifier_unregister_from_watcher ();
	cd_satus_notifier_unregister_from_ias ();
	
	// free all the items.
	g_list_free_full (myData.pItems, (GDestroyNotify) cd_free_item);
	
	if (! myConfig.bCompactMode)
		CD_APPLET_DELETE_MY_ICONS_LIST;
	
	// free the themes table.
	g_hash_table_destroy (myData.pThemePaths);
}


void cd_satus_notifier_launch_our_watcher (void)
{
	if (! myData.bHaveIAS && ! myData.bHaveWatcher)
	{
		cd_message ("starting our own watcher...");
		cairo_dock_launch_command_single (CD_PLUGINS_DIR"/status-notifier-watcher");
	}
}

  ///////////////////
 /// THEMES PATH ///
///////////////////

void cd_satus_notifier_add_theme_path (const gchar * cThemePath)
{
	g_return_if_fail (cThemePath != NULL && *cThemePath != '\0');
	int ref = GPOINTER_TO_INT (g_hash_table_lookup (myData.pThemePaths, cThemePath));  // 0 si le theme n'est pas dans la table.
	ref ++;  // on incremente la reference.
	g_hash_table_insert (myData.pThemePaths, g_strdup (cThemePath), GINT_TO_POINTER (ref));  // et on la met a jour dans la table.
	
	if (ref == 1)  // premiere fois qu'on voit ce chemin.
		///gtk_icon_theme_append_search_path (gtk_icon_theme_get_default(), cThemePath);  // append car ce sont des icones par defaut.
		cairo_dock_add_path_to_icon_theme (cThemePath);
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
		
		cairo_dock_remove_path_from_icon_theme (cThemePath);
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
		if (_item_is_visible (pItem))
		{
			Icon *pIcon = cd_satus_notifier_create_icon_for_item (pItem);
			if (pIcon)
				pIcons = g_list_prepend (pIcons, pIcon);
		}
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (pIcons, NULL, "Slide", NULL);
}

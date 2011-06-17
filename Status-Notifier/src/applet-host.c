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

// our address basename
#define CD_STATUS_NOTIFIER_HOST_ADDR "org.kde.StatusNotifierHost"


static CDStatusNotifierItem * _cd_satus_notifier_find_item_from_service (const gchar *cService)
{
	g_return_val_if_fail (cService != NULL, NULL);
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

static CDStatusNotifierItem * _cd_satus_notifier_find_item_from_position (int iPosition)
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

void cd_satus_notifier_add_new_item (const gchar *cService, const gchar *cObjectPath, int iPosition)
{
	CDStatusNotifierItem *pItem = _cd_satus_notifier_find_item_from_service (cService);
	g_return_if_fail (pItem == NULL);  // on evite d'ajouter 2 fois le meme service.
	
	pItem = cd_satus_notifier_create_item (cService, cObjectPath);
	g_return_if_fail (pItem != NULL);
	
	pItem->iPosition = iPosition;
	if (pItem->cLabel == NULL && pItem->cTitle == NULL)
		pItem->cLabel = g_strdup (pItem->cId);  // cService is often a dbus name like :1.355
	myData.pItems = g_list_prepend (myData.pItems, pItem);
	cd_debug ("item '%s' appended", pItem->cId);
	
	if (pItem->iStatus == CD_STATUS_PASSIVE)  // don't show a passive item.
		return;
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

void cd_satus_notifier_remove_item (const gchar *cService, int iPosition)
{
	CDStatusNotifierItem *pItem = (cService ? _cd_satus_notifier_find_item_from_service (cService) : _cd_satus_notifier_find_item_from_position (iPosition));
	g_return_if_fail (pItem != NULL);
	
	myData.pItems = g_list_remove (myData.pItems, pItem);
	
	if (pItem->iStatus == CD_STATUS_PASSIVE)  // the item was passive, therefore not visible.
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
	
	// Register our service name on the bus.
	pid_t pid = getpid ();
	myData.cHostName = g_strdup_printf (CD_STATUS_NOTIFIER_HOST_ADDR"-%d", pid);
	//cd_debug ("=== registering name '%s' on the bus ...", myData.cHostName);
	cairo_dock_register_service_name (myData.cHostName);
	
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
	g_list_foreach (myData.pItems, (GFunc) cd_free_item, NULL);
	g_list_free (myData.pItems);
	
	if (! myConfig.bCompactMode)
		CD_APPLET_DELETE_MY_ICONS_LIST;
	
	// free the themes table.
	g_hash_table_destroy (myData.pThemePaths);
}


void cd_satus_notifier_launch_our_watcher (void)
{
	if (myData.bNoIAS && myData.bNoWatcher)
	{
		cd_message ("starting our own watcher...\n");
		cairo_dock_launch_command (CD_PLUGINS_DIR"/status-notifier-watcher");
	}
}

  ///////////////////
 /// THEMES PATH ///
///////////////////

void cd_satus_notifier_add_theme_path (const gchar * cThemePath)
{
	g_return_if_fail (cThemePath != NULL);
	int ref = GPOINTER_TO_INT (g_hash_table_lookup (myData.pThemePaths, cThemePath));  // 0 si le theme n'est pas dans la table.
	ref ++;  // on incremente la reference.
	g_hash_table_insert (myData.pThemePaths, g_strdup (cThemePath), GINT_TO_POINTER (ref));  // et on la met a jour dans la table.
	
	if (ref == 1)  // premiere fois qu'on voit ce chemin.
		gtk_icon_theme_append_search_path (gtk_icon_theme_get_default(), cThemePath);  // append car ce sont des icones par defaut.
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
		
		GtkIconTheme *pIconTheme = gtk_icon_theme_get_default();  // et du theme.
		gchar **paths = NULL;
		gint iNbPaths = 0;
		gtk_icon_theme_get_search_path (pIconTheme, &paths, &iNbPaths);
	
		int i;
		for (i = 0; i < iNbPaths; i++)  // on cherche sa position dans le tableau.
		{
			if (strcmp (paths[i], cThemePath))
				break;
		}
		if (i < iNbPaths)  // trouve
		{
			g_free (paths[i]);
			for (i = i+1; i < iNbPaths; i++)  // on decale tous les suivants vers l'arriere.
			{
				paths[i-1] = paths[i];
			}
			paths[i-1] = NULL;
			gtk_icon_theme_set_search_path (pIconTheme, (const gchar **)paths, iNbPaths - 1);
		}
		
		g_strfreev (paths);
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
		if (pItem->iStatus != CD_STATUS_PASSIVE)
		{
			Icon *pIcon = cd_satus_notifier_create_icon_for_item (pItem);
			if (pIcon)
				pIcons = g_list_prepend (pIcons, pIcon);
		}
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (pIcons, NULL, "Slide", NULL);
}

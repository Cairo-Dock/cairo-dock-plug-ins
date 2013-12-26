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

#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-recent.h"
#include "applet-entry.h"
#include "applet-apps.h"
#include "applet-tree.h"

static void cd_menu_append_poweroff_to_menu (GtkWidget *menu, GldiModuleInstance *myApplet);


static gboolean _remove_empty_submenus (GtkWidget *pWidget)
{
	GList *pChildren = gtk_container_get_children (GTK_CONTAINER (pWidget));

	GList *ic;
	GtkWidget *pCurrentWidget;
	GtkWidget *pSubMenu;
	gint i = 0;
	for (ic = pChildren; ic != NULL; ic = ic->next)
	{
		pCurrentWidget = ic->data;
		if (! GTK_IS_SEPARATOR_MENU_ITEM (pCurrentWidget)
			&& GTK_IS_MENU_ITEM (pCurrentWidget))
		{
			pSubMenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (pCurrentWidget));
			if (pSubMenu != NULL && _remove_empty_submenus (pSubMenu))
				gtk_container_remove (GTK_CONTAINER (pWidget), pCurrentWidget);
			else
				i++; // a menu item or a non empty menu: not hide
		}
	}

	g_list_free (pChildren);

	if (i == 0)
		return TRUE;
	return FALSE;
}

static gboolean _make_menu_from_trees (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	myData.pTrees = pSharedMemory->pTrees;
	pSharedMemory->pTrees = NULL;
	
	// create the menu
	myData.pMenu = gldi_menu_new (myIcon);
	
	cd_menu_append_entry ();
	
	// append the trees we got: it will populate menu
	GMenuTree *tree;
	GList *t;
	for (t = myData.pTrees; t != NULL; t = t->next)
	{
		tree = t->data;
		cd_append_tree_in_menu (tree, myData.pMenu);
	}

	// A section can contain items which are not in the menu => NoDisplay=true
	_remove_empty_submenus (myData.pMenu);

	// append recent events
	if (myConfig.bShowRecent)
		cd_menu_append_recent_to_menu (myData.pMenu, myApplet);
	
	// append logout item
	if (myConfig.iShowQuit != CD_GMENU_SHOW_QUIT_NONE)
		cd_menu_append_poweroff_to_menu (myData.pMenu, myApplet);
	
	cd_menu_check_for_new_apps ();
	
	if (myData.bShowMenuPending)
	{
		cd_menu_show_menu ();
		myData.bShowMenuPending = FALSE;
	}
	
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	
	CD_APPLET_LEAVE (FALSE);
}

static void _load_trees_async (CDSharedMemory *pSharedMemory)
{
	GMenuTree *tree = cd_load_tree_from_file ("applications.menu");
	if (tree)
		pSharedMemory->pTrees = g_list_append (pSharedMemory->pTrees, tree);

	if (myConfig.bLoadSettingsMenu)
	{
		tree = cd_load_tree_from_file ("settings.menu");
		if (tree)
			pSharedMemory->pTrees = g_list_append (pSharedMemory->pTrees, tree);
	}
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_list_foreach (pSharedMemory->pTrees, (GFunc)g_object_unref, NULL);
	g_list_free (pSharedMemory->pTrees);
	g_free (pSharedMemory);
}

void cd_menu_start (void)
{
	cd_menu_init_apps ();
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	
	myData.pTask = cairo_dock_new_task_full (0,  // 1 shot task.
		(CairoDockGetDataAsyncFunc) _load_trees_async,
		(CairoDockUpdateSyncFunc) _make_menu_from_trees,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	
	if (cairo_dock_is_loading ())
		cairo_dock_launch_task_delayed (myData.pTask, 0);  // 0 <=> g_idle
	else
		cairo_dock_launch_task (myData.pTask);
}

void cd_menu_stop (void)
{
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	
	g_list_foreach (myData.pTrees, (GFunc)g_object_unref, NULL);
	g_list_free (myData.pTrees);
	myData.pTrees = NULL;
	
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (myData.pMenu);
		myData.pMenu = NULL;
		myData.pRecentMenuItem = NULL;
	}
}


// == cairo_dock_add_in_menu_with_stock_and_data   with icon size 24
static GtkWidget *_append_one_item_to_menu (const gchar *cLabel, const gchar *gtkStock, GFunc pFunction, GtkWidget *pMenu, gpointer pData)
{
	GtkWidget *pMenuItem = gldi_menu_item_new_full (cLabel, gtkStock, FALSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
	if (pFunction)
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (pFunction), pData);
	return pMenuItem;
}

static void cd_menu_append_poweroff_to_menu (GtkWidget *menu, GldiModuleInstance *myApplet)
{
	GtkWidget *menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	if (myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_LOGOUT || myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_BOTH)
		_append_one_item_to_menu (D_("Logout"), "system-log-out", (GFunc) cairo_dock_fm_logout, menu, NULL);

	if (myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_SHUTDOWN || myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_BOTH)
		_append_one_item_to_menu (D_("Shutdown"), "system-shutdown", (GFunc) cairo_dock_fm_shutdown, menu, NULL);
}

void cd_menu_show_menu (void)
{
	if (myData.pMenu != NULL)
	{
		CD_APPLET_POPUP_MENU_ON_MY_ICON (myData.pMenu);
		gtk_widget_grab_focus (myData.pEntry);
	}
	else
	{
		myData.bShowMenuPending = TRUE;
	}
}

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

#include <string.h>

#include "applet-struct.h"
#include "applet-menu.h"

static void _on_activate_item (GtkWidget *pMenuItem, CDQuickBrowserItem *pItem);
static gboolean _on_click_item (GtkWidget *pWidget, GdkEventButton* pButton, CDQuickBrowserItem *pItem);

static int _sort_item (CDQuickBrowserItem *pItem1, CDQuickBrowserItem *pItem2)
{
	if (pItem1 == NULL)
		return -1;
	if (pItem2 == NULL)
		return 1;
	GldiModuleInstance *myApplet = pItem2->pApplet;
	if (myConfig.bFoldersFirst)
	{
		if (pItem1->pSubMenu && ! pItem2->pSubMenu)
			return -1;
		if (! pItem1->pSubMenu && pItem2->pSubMenu)
			return 1;
	}
	if (myConfig.bCaseUnsensitive)
		return g_ascii_strcasecmp (pItem1->cTmpFileName, pItem2->cTmpFileName);
	else
		return strcmp (pItem1->cTmpFileName, pItem2->cTmpFileName);
}
static GList *_list_dir (const gchar *cDirPath, GldiModuleInstance *myApplet)
{
	//\______________ On ouvre le repertoire en lecture.
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	//\______________ On recupere chaque item, qu'on classe dans une liste temporaire.
	CDQuickBrowserItem *pItem;
	const gchar *cFileName;
	GList *pLocalItemList = NULL;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		if (! myConfig.bShowHiddenFiles && ( *cFileName == '.' || cFileName[strlen(cFileName) - 1] == '~' ) )
			continue;
		pItem = g_new0 (CDQuickBrowserItem, 1);
		pItem->cPath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		pItem->cTmpFileName = cFileName;  // valable uniquement dans cette boucle, ca tombe bien le classement se fait ici.
		pItem->pApplet = myApplet;
		if (g_file_test (pItem->cPath, G_FILE_TEST_IS_DIR))
			pItem->pSubMenu = gldi_menu_new (NULL);
		
		pLocalItemList = g_list_insert_sorted (pLocalItemList,
			pItem,
			(GCompareFunc)_sort_item);
	}
	while (1);
	g_dir_close (dir);
	
	return pLocalItemList;
}
static void _init_fill_menu_from_dir (CDQuickBrowserItem *pItem)
{
	const gchar *cDirPath = pItem->cPath;
	GtkWidget *pMenu = pItem->pSubMenu;
	GldiModuleInstance *myApplet = pItem->pApplet;
	
	//\______________ On recupere les items du repertoire.
	GList *pLocalItemList = _list_dir (cDirPath, myApplet);
	
	//\______________ On rajoute en premier une entree pour ouvrir le repertoire.
	CDQuickBrowserItem *pOpenDirItem = g_new0 (CDQuickBrowserItem, 1);
	pOpenDirItem->cPath = g_strdup (cDirPath);
	pOpenDirItem->pApplet = myApplet;
	pItem->pLocalItemList = g_list_prepend (pLocalItemList, pOpenDirItem);
	pItem->pCurrentItem = pItem->pLocalItemList->next;  // on la rajoute au menu ici, pas en meme temps que les autres.
	
	//\______________ On ajoute cette entree dans le menu des maintenant.
	GtkWidget *pMenuItem = gldi_menu_add_item (pMenu, D_("Open this folder"), myConfig.bHasIcons ? GTK_STOCK_OPEN : NULL, G_CALLBACK(_on_activate_item), pOpenDirItem);  // right click (e.g. open Bonobo or another file mgr)
	g_signal_connect (G_OBJECT (pMenuItem), "button-release-event", G_CALLBACK(_on_click_item), pOpenDirItem);
}


static void _drag_begin (GtkWidget *pWidget, GdkDragContext *pDragContext, GtkWidget *pMenuItem)
{
	// add an icon: the current pixbuf (add it now, once and for all).
	if (GTK_IS_IMAGE_MENU_ITEM (pMenuItem)) // some items don't have any icon.
	{
		gtk_drag_source_set_icon_pixbuf (pMenuItem,
			gtk_image_get_pixbuf (GTK_IMAGE (gldi_menu_item_get_image (pMenuItem))));  // GTK+ retains a reference on the pixbuf; when pMenuItem disappear, it will naturally loose this reference.
	}
}

static void _drag_data_get (GtkWidget *pWidget, GdkDragContext *pDragContext,
	GtkSelectionData *pSelectionData, guint iInfo, guint iTime, CDQuickBrowserItem *pItem)
{
	gchar *cURI = g_filename_to_uri (pItem->cPath, NULL, NULL);
	if (cURI != NULL)
	{
		gtk_selection_data_set (pSelectionData, gtk_selection_data_get_target (pSelectionData), 8, (guchar *) cURI, strlen (cURI));
		g_free (cURI);
	}
}

static void _fill_submenu_with_items (CDQuickBrowserItem *pRootItem, int iNbSubItemsAtOnce)
{
	GldiModuleInstance *myApplet = pRootItem->pApplet;
	GtkWidget *pMenu = pRootItem->pSubMenu;
	GList *pFirstItem = pRootItem->pCurrentItem;

	// static GtkTargetEntry s_pMenuItemTargets[] = { {(gchar*) "text/uri-list", 0, 0} }; // for drag and drop support

	CDQuickBrowserItem *pItem;
	gchar *cFileName;
	GtkWidget *pMenuItem;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fOrder;
	GList *l;
	int i;
	for (l = pFirstItem, i = 0; l != NULL && i < iNbSubItemsAtOnce; l = l->next, i ++)
	{
		pItem = l->data;
		
		//\______________ On cree l'entree avec son icone si necessaire.
		if (myConfig.bHasIcons)
		{
			cairo_dock_fm_get_file_info (pItem->cPath, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
			g_free (cName);
			cName = NULL;
			g_free (cURI);
			cURI = NULL;
		}

		cFileName = strrchr (pItem->cPath, '/');
		if (cFileName)
			cFileName ++;
		
		if (cIconName != NULL)
		{
			gchar *cPath = cairo_dock_search_icon_s_path (cIconName, cairo_dock_search_icon_size (GTK_ICON_SIZE_MENU));
			pMenuItem = gldi_menu_item_new (cFileName, cPath);
			g_free (cPath);
			g_free (cIconName);
			cIconName = NULL;
		}
		else
		{
			pMenuItem = gldi_menu_item_new (cFileName, "");
		}

		//\______________ On l'insere dans le menu.
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
		
		if (pItem->pSubMenu != NULL)
		{
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pItem->pSubMenu);
		}
		else
		{
			//\______________ Add drag and drop support for files only
			gtk_drag_source_set (pMenuItem, GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
				NULL, 0,
				GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK);
			
			gtk_drag_source_add_text_targets (pMenuItem);
			gtk_drag_source_add_uri_targets (pMenuItem);
			
			g_signal_connect (G_OBJECT (pMenuItem), "button-release-event", G_CALLBACK(_on_click_item), pItem); // left and right click
			g_signal_connect (G_OBJECT (pMenuItem), "drag-begin", G_CALLBACK (_drag_begin), pMenuItem); // to create pixbuf
			g_signal_connect (G_OBJECT (pMenuItem), "drag-data-get", G_CALLBACK (_drag_data_get), pItem); // when the item is dropped
		}
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), pItem); // select or over (submenu)
	}
	pRootItem->pCurrentItem = l;
}
static gboolean _fill_submenu_idle (CDQuickBrowserItem *pItem)
{
	GldiModuleInstance *myApplet = pItem->pApplet;
	CD_APPLET_ENTER;
	if (pItem->pLocalItemList == NULL)
	{
		_init_fill_menu_from_dir (pItem);
		if (pItem->pLocalItemList == NULL)  // cas particulier d'un repertoire vide, inutile de revenir ici pour rien faire.
			pItem->bMenuBuilt = TRUE;
	}
	else
	{
		_fill_submenu_with_items (pItem, myConfig.iNbSubItemsAtOnce);
		if (pItem->pCurrentItem == NULL)
			pItem->bMenuBuilt = TRUE;
	}

	if (pItem->bMenuBuilt)
	{
		GldiModuleInstance *myApplet = pItem->pApplet;
		myData.iSidFillDirIdle = 0;
		gtk_widget_realize (pItem->pSubMenu); // force to compute the size of the menu before displaying it -> avoid big menu that are out of the screen
		gtk_widget_show_all (pItem->pSubMenu);
		CD_APPLET_LEAVE (FALSE);
	}
	CD_APPLET_LEAVE (TRUE);
}
static void _on_activate_item (GtkWidget *pMenuItem, CDQuickBrowserItem *pItem)
{
	g_return_if_fail (pItem != NULL);
	GldiModuleInstance *myApplet = pItem->pApplet;
	CD_APPLET_ENTER;
	if (pItem->pSubMenu != NULL)
	{
		if (! pItem->bMenuBuilt)
		{
			if (myData.iSidFillDirIdle != 0)
				g_source_remove (myData.iSidFillDirIdle);
			myData.iSidFillDirIdle = g_idle_add ((GSourceFunc) _fill_submenu_idle, pItem);
		}
	}
	else // left click, no drag
	{
		cairo_dock_fm_launch_uri (pItem->cPath);
		cd_quick_browser_destroy_menu (myApplet);
	}
	CD_APPLET_LEAVE ();
}

static void _free_app_list_data (gpointer *data)
{
	gchar *cExec = data[1];
	g_free (cExec);
	g_free (data);
}

void cd_quick_browser_free_apps_list (GldiModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) _free_app_list_data, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_launch_with (GtkMenuItem *pMenuItem, gpointer *data)
{
	CDQuickBrowserItem *pItem = data[0];
	const gchar *cExec = data[1];

	cairo_dock_launch_command_printf ("%s \"%s\"", NULL, cExec, pItem->cPath);  // in case the program doesn't handle URI (geeqie, etc).

	cd_quick_browser_destroy_menu (pItem->pApplet);
}

static void _cd_open_parent (GtkMenuItem *pMenuItem, CDQuickBrowserItem *pItem)
{
	gchar *cUri = g_filename_to_uri (pItem->cPath, NULL, NULL);
	gchar *cFolder = g_path_get_dirname (cUri);
	cairo_dock_fm_launch_uri (cFolder);
	g_free (cFolder);
	g_free (cUri);

	cd_quick_browser_destroy_menu (pItem->pApplet);
}
static void _cd_copy_location (GtkMenuItem *pMenuItem, CDQuickBrowserItem *pItem)
{
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);  // GDK_SELECTION_PRIMARY

	gtk_clipboard_set_text (pClipBoard, pItem->cPath, -1);

	cd_quick_browser_destroy_menu (pItem->pApplet);
}
static gboolean _on_click_item (GtkWidget *pWidget, GdkEventButton* pButton, CDQuickBrowserItem *pItem)
{
	g_return_val_if_fail (pItem != NULL, FALSE);
	GldiModuleInstance *myApplet = pItem->pApplet;
	CD_APPLET_ENTER;

	if (pButton->button == 3) // right click
	{
		gchar *cUri = g_filename_to_uri (pItem->cPath, NULL, NULL);
		g_return_val_if_fail (cUri != NULL, FALSE);

		GtkWidget *pMenu = gldi_menu_new (NULL);
		
		GList *pApps = cairo_dock_fm_list_apps_for_file (cUri);
		if (pApps != NULL)
		{
			GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Open with"), pMenu, GTK_STOCK_OPEN);

			cd_quick_browser_free_apps_list (myApplet);

			GList *a;
			gchar **pAppInfo;
			gchar *cIconPath;
			for (a = pApps; a != NULL; a = a->next)
			{
				pAppInfo = a->data;

				if (pAppInfo[2] != NULL)
					cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2], cairo_dock_search_icon_size (GTK_ICON_SIZE_MENU));
				else
					cIconPath = NULL;

				gpointer *data = g_new (gpointer, 2);
				data[0] = pItem;
				data[1] = pAppInfo[1];
				myData.pAppList = g_list_prepend (myData.pAppList, data); // to save the exec command

				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pAppInfo[0], cIconPath, _cd_launch_with, pSubMenu, data);

				g_free (cIconPath);
				g_free (pAppInfo[0]);
				g_free (pAppInfo[2]);
				g_free (pAppInfo);
			}
			g_list_free (pApps);
		}
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open parent folder"), GTK_STOCK_DIRECTORY, _cd_open_parent, pMenu, pItem);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Copy the location"), GTK_STOCK_COPY, _cd_copy_location, pMenu, pItem);
		
		gtk_widget_show_all (pMenu);
		gtk_menu_popup (GTK_MENU (pMenu),
			NULL,
			NULL,
			NULL,  // popup on mouse.
			NULL,
			1,
			gtk_get_current_event_time ());
		g_free (cUri);
		CD_APPLET_LEAVE (TRUE); // do not remove quick_browser menu now
	}

	CD_APPLET_LEAVE (FALSE);
}

CDQuickBrowserItem *cd_quick_browser_make_menu_from_dir (const gchar *cDirPath, GldiModuleInstance *myApplet)
{
	CDQuickBrowserItem *pRootItem = g_new0 (CDQuickBrowserItem, 1);
	pRootItem->cPath = g_strdup (cDirPath);
	pRootItem->pApplet = myApplet;
	pRootItem->pSubMenu = gldi_menu_new (myIcon);
	
	_init_fill_menu_from_dir (pRootItem);
	_fill_submenu_with_items (pRootItem, 1e6);
	pRootItem->bMenuBuilt = TRUE;
	gtk_widget_show_all (pRootItem->pSubMenu);
	
	return pRootItem;
}

static void _free_item (CDQuickBrowserItem *pItem)
{
	g_free (pItem->cPath);
	if (pItem->pLocalItemList != NULL)
	{
		g_list_foreach (pItem->pLocalItemList, (GFunc) _free_item, NULL);
		g_list_free (pItem->pLocalItemList);
	}
	g_free (pItem);
}
void cd_quick_browser_destroy_menu (GldiModuleInstance *myApplet)
{
	if (myData.iSidFillDirIdle != 0)
		g_source_remove (myData.iSidFillDirIdle);
	myData.iSidFillDirIdle = 0;
	
	if (myData.pRootItem != NULL)
	{
		gtk_widget_destroy (myData.pRootItem->pSubMenu);  // detruit tous les pSubMenu en cascade.
		_free_item (myData.pRootItem);
		myData.pRootItem = NULL;
	}
}

void cd_quick_browser_show_menu (GldiModuleInstance *myApplet)
{
	cd_quick_browser_destroy_menu (myApplet);
	
	myData.pRootItem = cd_quick_browser_make_menu_from_dir (myConfig.cDirPath, myApplet);
	g_return_if_fail (myData.pRootItem != NULL && myData.pRootItem->pSubMenu != NULL);
	
	CD_APPLET_POPUP_MENU_ON_MY_ICON (myData.pRootItem->pSubMenu);
}

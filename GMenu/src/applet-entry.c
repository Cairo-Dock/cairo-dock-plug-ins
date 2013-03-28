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
#include "applet-entry.h"

#include <gdk/gdkkeysyms.h>
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION > 20)
#include <gdk/gdkkeysyms-compat.h>
#endif

static void cd_menu_build_entry_model (void);


static void _launch_selected_app (void)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (selection, &pModel, &iter))
	{
		GAppInfo *pAppInfo = NULL;
		gtk_tree_model_get (pModel, &iter,
			2, &pAppInfo, -1);
		g_print (" -> %s\n", g_app_info_get_name (pAppInfo));
		g_app_info_launch (pAppInfo, NULL, NULL, NULL);
	}
}

static gboolean _on_entry_changed (GtkWidget *pEntry,
	CairoDockModuleInstance *myApplet)
{
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	g_print ("%s (%s)\n", __func__, cText);
	if (cText && *cText != '\0')
	{
		cd_menu_build_entry_model ();
		
		gtk_widget_show_all (myData.pAppsWindow);
		gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (myData.pModelFilter));
		
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
		GtkTreeModel *pModel;
		GtkTreeIter iter;
		if (! gtk_tree_selection_get_selected (selection, &pModel, &iter))  // nothing selected -> select the first row
		{
			if (gtk_tree_model_get_iter_first (pModel, &iter))
				gtk_tree_selection_select_iter (selection, &iter);
		}
		
		gint n = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (myData.pModelFilter), NULL);  // even if n=0, it's probably better to display the empty treeview, to show that there is no result, than hiding it suddenly.
		g_print ("%d elements x %d\n", n, myData.iTreeViewCellHeight);
		int iHeight = MAX (myData.iTreeViewCellHeight, n * myData.iTreeViewCellHeight);
		
		int x, y;
		gtk_window_get_position (GTK_WINDOW (gtk_widget_get_toplevel (myData.pMenu)), &x, &y);
		g_print ("menu: %d; %d\n", x, y);
		int entry_x, entry_y;
		gtk_widget_translate_coordinates (myData.pEntry,
			myData.pMenu,
			0, 0,
			&entry_x, &entry_y);
		g_print ("entry: %d; %d\n", entry_x, entry_y);
		
		gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (myData.pScrolledWindow), iHeight);
		
		if (iHeight > y + entry_y)  // not enough space to display it above the entry -> display it under
			gtk_window_move (GTK_WINDOW (myData.pAppsWindow), x + entry_x, y + myData.iTreeViewCellHeight + 2);
		else
			gtk_window_move (GTK_WINDOW (myData.pAppsWindow), x + entry_x, y - iHeight);
	}
	else
	{
		gtk_widget_hide (myData.pAppsWindow);
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
		gtk_tree_selection_unselect_all (selection);
	}
	
	return FALSE;
}

static gboolean _select_next (GtkTreeModel *pModel, GtkTreePath *path, GtkTreeIter *iter, GtkTreeIter *pCurrentIter)
{
	if (iter->stamp == pCurrentIter->stamp)
	{
		if (gtk_tree_model_iter_next (pModel, pCurrentIter))
		{
			GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
			gtk_tree_selection_select_iter (selection, pCurrentIter);
		}
		return TRUE;
	}
	return FALSE;
}
static gboolean _select_previous (GtkTreeModel *pModel, GtkTreePath *path, GtkTreeIter *iter, GtkTreeIter *pCurrentIter)
{
	if (iter->stamp == pCurrentIter->stamp)
	{
		if (gtk_tree_model_iter_previous (pModel, pCurrentIter))
		{
			GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
			gtk_tree_selection_select_iter (selection, pCurrentIter);
		}
		return TRUE;
	}
	return FALSE;
}
static gboolean _select_last (GtkTreeModel *pModel, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GtkTreeIter next_iter;
	memcpy (&next_iter, iter, sizeof (GtkTreeIter));
	if (! gtk_tree_model_iter_next (pModel, &next_iter))  // it's the last
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
			gtk_tree_selection_select_iter (selection, iter);
	}
	return FALSE;
}

static gboolean _on_key_pressed_menu (GtkWidget *pMenuItem,
	GdkEventKey *pEvent,
	CairoDockModuleInstance *myApplet)
{
	// redirect the signal to the entry
	g_signal_emit_by_name (myData.pEntry, "key-press-event", pEvent, myApplet);
	
	// redirect the signal to the apps window if it's already visible.
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (myData.pEntry));
	if (cText && *cText != '\0')  // some text is typed
	{
		switch (pEvent->keyval)
		{
			case GDK_Return:
			{
				_launch_selected_app ();
				gtk_menu_shell_deactivate (GTK_MENU_SHELL (myData.pMenu));
				return TRUE;  // don't pass to the menu
			}
			case GDK_Up:
			case GDK_Down:
			case GDK_Page_Down:
			case GDK_Page_Up:
			{
				GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (myData.pTreeView));
				GtkTreeIter iter;
				if (gtk_tree_selection_get_selected (selection, NULL, &iter))
				{
					if (pEvent->keyval == GDK_Up)
						gtk_tree_model_foreach (myData.pModelFilter, (GtkTreeModelForeachFunc)_select_previous, &iter);
					else if (pEvent->keyval == GDK_Down)
						gtk_tree_model_foreach (myData.pModelFilter, (GtkTreeModelForeachFunc)_select_next, &iter);
					else if (pEvent->keyval == GDK_Page_Up)
					{
						if (gtk_tree_model_get_iter_first (myData.pModelFilter, &iter))
							gtk_tree_selection_select_iter (selection, &iter);
					}
					else
						gtk_tree_model_foreach (myData.pModelFilter, (GtkTreeModelForeachFunc)_select_last, NULL);
				}
				else
				{
					if (gtk_tree_model_get_iter_first (myData.pModelFilter, &iter))
						gtk_tree_selection_select_iter (selection, &iter);
				}
				return TRUE;  // don't pass to the menu
			}
			case GDK_Home:
			case GDK_End:
			{
				return TRUE;  // don't pass to the menu
			}
			default:
			break;
		}
	}
	
	// pass the signal to the menu (for navigation by arrows)
	return FALSE;
}


void cd_menu_append_entry (void)
{
	GtkWidget *pMenuItem = gtk_menu_item_new ();
	GtkWidget *hbox =  _gtk_hbox_new (3);
	gtk_container_add (GTK_CONTAINER (pMenuItem), hbox);  /// there's something to do to align the hbox to the left of the menu-item, but what ?...
	
	GtkWidget *pImage = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	
	GtkWidget *pEntry = gtk_entry_new ();
	
	gtk_box_pack_start (GTK_BOX (hbox), pImage, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), pEntry, TRUE, TRUE, 0);
	
	g_signal_connect (pEntry, "changed",
		G_CALLBACK (_on_entry_changed),
		myApplet);  // to find matching apps while the user is typing
	g_signal_connect (myData.pMenu, "key-press-event",
		G_CALLBACK (_on_key_pressed_menu),
		myApplet);  // to redirect the signal to the event, or it won't get it.
	
	gtk_widget_show_all (pMenuItem);
	gtk_menu_shell_append (GTK_MENU_SHELL (myData.pMenu), pMenuItem);
	myData.pEntry = pEntry;  // make it global so that we can grab the focus before we pop the menu up
}


static gboolean _app_match (GAppInfo *pAppInfo, const gchar *key)
{
	int n = strlen (key);
	const gchar *prop = g_app_info_get_executable (pAppInfo);
	if (!prop || strncmp (prop, key, n) != 0)
	{
		prop = g_app_info_get_name (pAppInfo);
		if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
		{
			prop = g_app_info_get_display_name (pAppInfo);
			if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
			{
				if (n < 3)
					return FALSE;
				prop = g_app_info_get_description (pAppInfo);
				gchar *lower_prop = g_ascii_strdown (prop, -1);
				if (!prop || strstr (lower_prop, key) == NULL)
				{
					g_free (lower_prop);
					return FALSE;
				}
				g_free (lower_prop);
			}
		}
	}
	return TRUE;
}

static gboolean _model_filter (GtkTreeModel *pModel, GtkTreeIter *iter, G_GNUC_UNUSED gpointer data)
{
	const gchar *key = gtk_entry_get_text (GTK_ENTRY (myData.pEntry));
	GAppInfo *pAppInfo = NULL;
	gtk_tree_model_get (pModel, iter,
		2, &pAppInfo, -1);
	
	return _app_match (G_APP_INFO (pAppInfo), key);
}

static void _realized (GtkWidget *pTreeView, gpointer data)
{
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW (pTreeView));  /// after realized...
	GtkTreeViewColumn *col = gtk_tree_view_get_column (GTK_TREE_VIEW (pTreeView), 1);
	gtk_tree_view_column_cell_get_size (col, NULL, NULL, NULL, NULL, &myData.iTreeViewCellHeight);
	myData.iTreeViewCellHeight = MAX (myData.iTreeViewCellHeight, myData.iPanelDefaultMenuIconSize);
	int vsep;
	gtk_widget_style_get (pTreeView, "vertical-separator", &vsep, NULL);
	myData.iTreeViewCellHeight += 2;  // did I miss something ? it seems not enoough...
	g_print ("H=%d\n", myData.iTreeViewCellHeight);
	
}

static void _add_app_to_model (G_GNUC_UNUSED gchar *cDesktopFilePath, GAppInfo *pAppInfo, GtkListStore *pModel)
{
	GtkTreeIter iter;
	memset (&iter, 0, sizeof (GtkTreeIter));
	gtk_list_store_append (GTK_LIST_STORE (pModel), &iter);
	
	GIcon *gicon = g_app_info_get_icon (pAppInfo);
	gchar *cFileName = g_icon_to_string (gicon);
	gchar *cImagePath = cairo_dock_search_icon_s_path (cFileName, myData.iPanelDefaultMenuIconSize);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cImagePath, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize, NULL);
	if (gdk_pixbuf_get_width (pixbuf) != myData.iPanelDefaultMenuIconSize)  // 'gdk_pixbuf_new_from_file_at_size' doesn't respect the given size with xpm images...
	{
		GdkPixbuf *src = pixbuf;
		pixbuf = gdk_pixbuf_scale_simple (src, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize, GDK_INTERP_BILINEAR);
		g_object_unref (src);
	}
	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter,
		0, g_app_info_get_name (pAppInfo),
		1, pixbuf,
		2, pAppInfo, -1);
	g_free (cImagePath);
	g_free (cFileName);
}

/*static gboolean _on_button_press_treeview (GtkWidget *pTreeView, GdkEventButton* event, G_GNUC_UNUSED gpointer data)
{
	g_print ("%s ()\n", __func__);
	_launch_selected_app ();
	return FALSE;
}*/

static void _on_menu_deactivated (GtkWidget *pMenu, G_GNUC_UNUSED gpointer data)
{
	gtk_widget_hide (myData.pAppsWindow);
	gtk_entry_set_text (GTK_ENTRY (myData.pEntry), "");
}


static void cd_menu_build_entry_model (void)
{
	if (myData.bModelLoaded)
		return;
	myData.bModelLoaded = TRUE;
	
	// build the model
	GtkListStore *pModel = gtk_list_store_new (3,
		G_TYPE_STRING,
		GDK_TYPE_PIXBUF,
		G_TYPE_POINTER);
	
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (pModel), 0, GTK_SORT_ASCENDING);
	g_hash_table_foreach (myData.pKnownApplications, (GHFunc)_add_app_to_model, pModel);  // elements in pKnownApplications are never removed
	
	// filter
	GtkTreeModel *pModelFilter = gtk_tree_model_filter_new (GTK_TREE_MODEL (pModel), NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (pModelFilter),
		(GtkTreeModelFilterVisibleFunc) _model_filter,
		NULL,
		(GDestroyNotify)NULL);
	myData.pModelFilter = pModelFilter;
	
	// make a treeview
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (pModel), 0, GTK_SORT_ASCENDING);
	GtkWidget *pTreeView = gtk_tree_view_new ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (pTreeView), GTK_TREE_MODEL (pModelFilter));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pTreeView), FALSE);
	gtk_tree_view_set_hover_selection (GTK_TREE_VIEW (pTreeView), TRUE);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pTreeView));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	
	GtkCellRenderer *rend = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pTreeView), -1, NULL, rend, "pixbuf", 1, NULL);
	rend = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pTreeView), -1, NULL, rend, "text", 0, NULL);
	
	g_signal_connect (G_OBJECT (pTreeView),
		"realize",
		G_CALLBACK (_realized),
		NULL);
	/*g_signal_connect (G_OBJECT (pTreeView),
		"button-press-event",
		G_CALLBACK (_on_button_press_treeview),
		NULL);
	g_object_set (pTreeView, "activate-on-single-click", TRUE, NULL);*/
	myData.pTreeView = pTreeView;
	
	// vertical scrollbar
	GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pTreeView);
	g_object_set (pScrolledWindow, "height-request", myData.iTreeViewCellHeight, NULL);
	myData.pScrolledWindow = pScrolledWindow;
	
	// window
	myData.pAppsWindow = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_resizable (GTK_WINDOW (myData.pAppsWindow), FALSE);
	gtk_window_set_type_hint (GTK_WINDOW (myData.pAppsWindow), GDK_WINDOW_TYPE_HINT_COMBO);
	gtk_container_add (GTK_CONTAINER (myData.pAppsWindow), pScrolledWindow);
	g_signal_connect (G_OBJECT (myData.pMenu),
		"deactivate",
		G_CALLBACK (_on_menu_deactivated),
		NULL);
	/*gtk_widget_add_events (myData.pAppsWindow,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |
		GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	gtk_window_set_modal (GTK_WINDOW (myData.pAppsWindow), TRUE);*/
}

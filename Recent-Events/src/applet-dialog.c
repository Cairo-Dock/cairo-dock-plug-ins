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
#include <time.h>

#include "applet-struct.h"
#include "applet-search.h"
#include "applet-dialog.h"

static void _on_got_events (ZeitgeistResultSet *events, GtkListStore *pModel);

void cd_trigger_search (void)
{
	if (myData.pDialog == NULL)
		return;

	static CDEventType iOldCategory = -1;

	const gchar *cQuery = gtk_entry_get_text (GTK_ENTRY (myData.pEntry));
	CDEventType iCategory = myData.iCurrentCaterogy;
	GtkListStore *pModel = myData.pModel;

	/* avoid rebuild when pressing "non letter" keys
	 * Note: cQuery's pointer is not automatically modified when the string change.
	 */
	if (iOldCategory == iCategory && g_strcmp0 (myData.cQuery, cQuery) == 0)
		return;

	g_free (myData.cQuery);
	myData.cQuery = g_strdup (cQuery);
	iOldCategory = iCategory;

	int iSortType = 0;
	if (iCategory >= CD_EVENT_TOP_RESULTS)
	{
		iCategory = CD_EVENT_ALL;
		iSortType = 1;
	}
	
	gtk_list_store_clear (pModel);
	if (cQuery != NULL && *cQuery != '\0')
		cd_search_events (cQuery, iCategory, (CDOnGetEventsFunc) _on_got_events, pModel);
	else
		cd_find_recent_events (iCategory, iSortType, (CDOnGetEventsFunc) _on_got_events, pModel);
}

static void on_click_category_button (GtkButton *button, gpointer data)
{
	if (! gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (button)))
		return;
	myData.iCurrentCaterogy = GPOINTER_TO_INT (data);
	cd_debug ("filter on category %d", myData.iCurrentCaterogy);
	cd_trigger_search ();
}

static void on_clear_filter (GtkEntry *pEntry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer data)
{
	gtk_entry_set_text (pEntry, "");
	cd_debug ("relaunch the search...");
	cd_trigger_search ();
}

static gboolean on_key_press_filter (G_GNUC_UNUSED GtkWidget *pWidget,
	GdkEventKey *pKey, G_GNUC_UNUSED gpointer data)
{
	if (pKey->keyval == GDK_KEY_Escape)
	{
		cd_toggle_dialog ();
		return TRUE;
	}
	cd_trigger_search ();
	return FALSE;
}

static void _on_got_events (ZeitgeistResultSet *pEvents, GtkListStore *pModel)
{
	int i, n;
	ZeitgeistEvent *event;
	ZeitgeistSubject *subject;
	gint64 iTimeStamp;
	const gchar *cEventURI;
	guint id;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL, *cIconPath, *cPath = NULL;
	double fOrder;
	int iVolumeID;
	gboolean bIsDirectory;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GHashTable *pHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);  // used to prevent doubles
	
	//\_____________ parse all the events.
	while (zeitgeist_result_set_has_next (pEvents))
	{
		#ifdef ZEITGEIST_1_0
		event = zeitgeist_result_set_next (pEvents);
		#else
		event = zeitgeist_result_set_next_value (pEvents);
		#endif
		iTimeStamp = zeitgeist_event_get_timestamp (event) / 1e3;
		id = zeitgeist_event_get_id (event);
		n = zeitgeist_event_num_subjects (event);
		if (n > 1)
			cd_debug (" +++ %s, %s, %d", zeitgeist_event_get_interpretation (event), zeitgeist_event_get_manifestation (event), n);
		for (i = 0; i < n; i++)
		{
			subject = zeitgeist_event_get_subject (event, i);
			
			//\_____________ prevent doubles.
			cEventURI = zeitgeist_subject_get_uri (subject);
			if (g_hash_table_lookup_extended  (pHashTable, cEventURI, NULL, NULL))
				continue;
			//g_print ("  %s:\n    %s, %s\n", cEventURI, zeitgeist_subject_get_interpretation (subject), zeitgeist_subject_get_manifestation (subject));
			
			//\_____________ ignore files that have been deleted
			cPath = g_filename_from_uri (cEventURI, NULL, NULL);  // NULL for anything else than file://*
			if (strncmp (cEventURI, "file://", 7) == 0 && ! g_file_test (cPath, G_FILE_TEST_EXISTS))
			{
				g_hash_table_insert (pHashTable, (gchar*)cEventURI, NULL);  // since we've checked it, insert it, even if we don't display it.
				g_free (cPath);
				continue;
			}
			//\_____________ get the text to display.
			const gchar *cText = zeitgeist_subject_get_text (subject);
			if (cText == NULL)  // skip empty texts (they are most of the times web page that redirect to another page, which is probably in the next event anyway).
				continue;
			
			//\_____________ find the icon.
			if (strncmp (cEventURI, "http", 4) == 0)  // gvfs is deadly slow to get info on distant URI...
			{
				cIconName = cairo_dock_search_icon_s_path ("text-html", myData.iDesiredIconSize);
			}
			else if (strncmp (cEventURI, "application://", 14) == 0)  // application URL
			{
				gchar *cClass = cairo_dock_register_class (cEventURI+14);
				cIconName = g_strdup (cairo_dock_get_class_icon (cClass));
				cText = cairo_dock_get_class_name (cClass);  // use the translated name
				g_free (cClass);
			}
			else
			{
				cairo_dock_fm_get_file_info (cEventURI, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_DATE);
			}
			if (cIconName != NULL)
			{
				cIconPath = cairo_dock_search_icon_s_path (cIconName, myData.iDesiredIconSize);
				pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, myData.iDesiredIconSize, myData.iDesiredIconSize, NULL);
				g_free (cIconPath);
			}
			else
				pixbuf = NULL;
			
			//\_____________ build the path to display.
			const gchar *cDisplayedPath = (cPath ? cPath : cEventURI);

			// need to escape the '&' (and ', etc.) because gtk-tooltips use markups by default.
			gchar *cEscapedPath = g_markup_escape_text (cDisplayedPath, -1);
			
			//\_____________ store in the model.
			memset (&iter, 0, sizeof (GtkTreeIter));
			gtk_list_store_append (GTK_LIST_STORE (pModel), &iter);
			gtk_list_store_set (GTK_LIST_STORE (pModel), &iter,
				CD_MODEL_NAME, cText,
				CD_MODEL_URI, cEventURI,
				CD_MODEL_PATH, cEscapedPath,
				CD_MODEL_ICON, pixbuf,
				CD_MODEL_DATE, iTimeStamp,
				CD_MODEL_ID, id, -1);
			
			g_free (cIconName);
			cIconName = NULL;
			g_free (cName);
			cName = NULL;
			g_free (cURI);
			cURI = NULL;
			if (pixbuf)
				g_object_unref (pixbuf);
			g_free (cPath);
			g_free (cEscapedPath);
			
			g_hash_table_insert (pHashTable, (gchar*)cEventURI, NULL);  // cEventURI stays valid in this function.
		}
	}
	g_hash_table_destroy (pHashTable);
}

void cd_folders_free_apps_list (GldiModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) g_free, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_launch_with (GtkMenuItem *pMenuItem, const gchar *cExec)
{
	gchar *cPath = g_filename_from_uri (myData.cCurrentUri, NULL, NULL);
	cairo_dock_launch_command_printf ("%s \"%s\"", NULL, cExec, cPath);  // in case the program doesn't handle URI (geeqie, etc).
	g_free (cPath);
}

static void _cd_open_parent (GtkMenuItem *pMenuItem, gpointer data)
{
	gchar *cFolder = g_path_get_dirname (myData.cCurrentUri);
	cairo_dock_fm_launch_uri (cFolder);
	g_free (cFolder);
}
static void _cd_copy_location (GtkMenuItem *pMenuItem, gpointer data)
{
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);  // GDK_SELECTION_PRIMARY
	gtk_clipboard_set_text (pClipBoard, myData.cCurrentUri, -1);
}
static void _on_event_deleted (int iNbEvents, gpointer data)
{
	cd_trigger_search ();
}
static void _cd_delete_event (GtkMenuItem *pMenuItem, gpointer data)
{
	guint32 id = GPOINTER_TO_UINT (data);
	cd_delete_event (id, _on_event_deleted, NULL);
}
static gboolean _on_click_module_tree_view (GtkTreeView *pTreeView, GdkEventButton* pButton, gpointer data)
{
	//g_print ("%s ()\n", __func__);
	if ((pButton->button == 3 && pButton->type == GDK_BUTTON_RELEASE)  // right-click
	|| (pButton->button == 1 && pButton->type == GDK_2BUTTON_PRESS))  // double-click
	{
		cd_debug ("%s ()", __func__);
		// get the current selected line.
		GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
		GtkTreeModel *pModel;
		GtkTreeIter iter;
		if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
			return FALSE;
		
		gchar *cName = NULL, *cUri = NULL;
		guint id = 0;
		gtk_tree_model_get (pModel, &iter,
			CD_MODEL_NAME, &cName,
			CD_MODEL_URI, &cUri,
			CD_MODEL_ID, &id, -1);
		
		//launch or build the menu.
		gboolean bIsAppli = (strncmp (cUri, "application://", 14) == 0);
		if (pButton->button == 1)  // double-click
		{
			if (bIsAppli)  // an appli -> run it
			{
				gchar *tmp = strrchr (cUri, '.');  // remove the '.desktop'
				if (tmp)
					*tmp = '\0';
				cairo_dock_launch_command (cUri+14);
			}
			else  // a file -> open it
			{
				cairo_dock_fm_launch_uri (cUri);
			}
			g_free (cUri);
		}
		else  // right-click
		{
			GtkWidget *pMenu = gldi_menu_new (NULL);
			g_free (myData.cCurrentUri);
			myData.cCurrentUri = cUri;
			
			if (!bIsAppli)
			{
				GList *pApps = cairo_dock_fm_list_apps_for_file (cUri);
				if (pApps != NULL)
				{
					GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Open with"), pMenu, GLDI_ICON_NAME_OPEN);
					
					cd_folders_free_apps_list (myApplet);
					
					GList *a;
					gchar **pAppInfo;
					gchar *cIconPath;
					for (a = pApps; a != NULL; a = a->next)
					{
						pAppInfo = a->data;
						myData.pAppList = g_list_prepend (myData.pAppList, pAppInfo[1]);
						
						if (pAppInfo[2] != NULL)
							cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2], cairo_dock_search_icon_size (GTK_ICON_SIZE_MENU));
						else
							cIconPath = NULL;
						CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pAppInfo[0], cIconPath, _cd_launch_with, pSubMenu, pAppInfo[1]);
						g_free (cIconPath);
						g_free (pAppInfo[0]);
						g_free (pAppInfo[2]);
						g_free (pAppInfo);
					}
					g_list_free (pApps);
				}
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open parent folder"), GLDI_ICON_NAME_DIRECTORY, _cd_open_parent, pMenu, NULL);
				
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Copy the location"), GLDI_ICON_NAME_COPY, _cd_copy_location, pMenu, NULL);
			}
			
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this event"), GLDI_ICON_NAME_REMOVE, _cd_delete_event, pMenu, GUINT_TO_POINTER (id));
			
			gtk_widget_show_all (pMenu);
			gtk_menu_popup_at_pointer (GTK_MENU (pMenu), (GdkEvent*)pButton);
		}
	}
	return FALSE;
}

/// not sure what's the point of this callback ... I suppress it for the moment (21/08/2012).
/**static gboolean _cairo_dock_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, gpointer *data)
{
	if (path_currently_selected)
		return TRUE;
	GtkTreeIter iter;
	if (! gtk_tree_model_get_iter (model, &iter, path))
		return FALSE;
	
	return TRUE;
}*/

#define DATE_BUFFER_LENGTH 50
static void _render_date (GtkTreeViewColumn *tree_column, GtkCellRenderer *cell, GtkTreeModel *model,GtkTreeIter *iter, gpointer data)
{
	gint64 iDate = 0;
	gtk_tree_model_get (model, iter, CD_MODEL_DATE, &iDate, -1);
	
	time_t epoch = iDate;
	struct tm t;
	localtime_r (&epoch, &t);
	
	static gchar s_cDateBuffer[50];
	const gchar *cFormat;
	if (myConfig.b24Mode)
		cFormat = "%a %d %b, %R";
	else
		cFormat = "%a %d %b, %I:%M %p";
	strftime (s_cDateBuffer, DATE_BUFFER_LENGTH, cFormat, &t);
	
	g_object_set (cell, "text", s_cDateBuffer, NULL);
}

static inline GtkToolItem *_add_category_button (GtkWidget *pToolBar, const gchar *cLabel, const gchar *cIconName, int pos, GtkToolItem *group)
{
	GtkToolItem *pCategoryButton;
	if (group)
		pCategoryButton= gtk_radio_tool_button_new_from_widget (GTK_RADIO_TOOL_BUTTON (group));
	else
		pCategoryButton = gtk_radio_tool_button_new (NULL);
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (pCategoryButton), cLabel);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (pCategoryButton), cIconName);
	g_signal_connect (G_OBJECT (pCategoryButton), "toggled", G_CALLBACK(on_click_category_button), GINT_TO_POINTER (pos));
	gtk_toolbar_insert (GTK_TOOLBAR (pToolBar) , pCategoryButton, -1);
	return pCategoryButton;
}
#define MARGIN 3
static GtkWidget *cd_build_events_widget (void)
{
	GtkWidget *pMainBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN);
	
	// category toolbar.
	GtkWidget *pToolBar = gtk_toolbar_new ();
	///gtk_toolbar_set_orientation (GTK_TOOLBAR (pToolBar), GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style (GTK_TOOLBAR (pToolBar), GTK_TOOLBAR_BOTH);  // overwrite system preference (GTK_TOOLBAR_ICONS)

	gtk_style_context_add_class (gtk_widget_get_style_context (pToolBar),
		GTK_STYLE_CLASS_INLINE_TOOLBAR); // style: inline
	GtkCssProvider *css = gtk_css_provider_new (); // but without border
	gtk_css_provider_load_from_data (css, ".inline-toolbar.toolbar { "
		"background: transparent; border-color: transparent; }", -1, NULL);
	GtkStyleContext *ctx = gtk_widget_get_style_context (pToolBar);
	gtk_style_context_add_provider (ctx, GTK_STYLE_PROVIDER (css),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (pToolBar), FALSE);  // force to display all the entries.
	gtk_box_pack_start (GTK_BOX (pMainBox), pToolBar, TRUE, TRUE, MARGIN);
	
	int i = 0;
	GtkToolItem *group = _add_category_button (pToolBar, D_("All"), "stock_search", i++, NULL);
	_add_category_button (pToolBar, D_("Applications"), "exec", i++, group);
	_add_category_button (pToolBar, D_("Documents"), "document", i++, group);
	///_add_category_button (pToolBar, D_("Folders"), "folder", i++, group);
	_add_category_button (pToolBar, D_("Images"), "image", i++, group);
	_add_category_button (pToolBar, D_("Audio"), "sound", i++, group);
	_add_category_button (pToolBar, D_("Videos"), "video", i++, group);
	_add_category_button (pToolBar, D_("Web"), "text-html", i++, group);
	_add_category_button (pToolBar, D_("Others"), "unknown", i++, group);
	_add_category_button (pToolBar, D_("Top Results"), "gtk-about", i, group);
	
	// search entry.
	GtkWidget *pFilterBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, CAIRO_DOCK_GUI_MARGIN);
	gtk_box_pack_start (GTK_BOX (pMainBox), pFilterBox, FALSE, FALSE, MARGIN);
	
	GtkWidget *pFilterLabel = gtk_label_new (D_("Look for events"));
	gldi_dialog_set_widget_text_color (GTK_WIDGET (pFilterLabel));
	gtk_box_pack_start (GTK_BOX (pFilterBox), pFilterLabel, FALSE, FALSE, MARGIN);
	
	GtkWidget *pEntry = gtk_entry_new ();
	// press any key:
	g_signal_connect (pEntry, "key-release-event", G_CALLBACK (on_key_press_filter), NULL);
	gtk_box_pack_start (GTK_BOX (pFilterBox), pEntry, TRUE, TRUE, MARGIN);
	gtk_widget_set_tooltip_text (pEntry, D_("The default boolean operator is AND. Thus the query foo bar will be interpreted as foo AND bar. To exclude a term from the result set prepend it with a minus sign - eg foo -bar. Phrase queries can be done by double quoting the string \"foo is a bar\". You can truncate terms by appending a *. "));

	gtk_entry_set_icon_activatable (GTK_ENTRY (pEntry), GTK_ENTRY_ICON_SECONDARY, TRUE);
	gtk_entry_set_icon_from_icon_name (GTK_ENTRY (pEntry), GTK_ENTRY_ICON_SECONDARY, GLDI_ICON_NAME_CLEAR);
	g_signal_connect (pEntry, "icon-press", G_CALLBACK (on_clear_filter), NULL);

	myData.pEntry = pEntry;
	gtk_widget_grab_focus (pEntry);
	
	// model
	GtkListStore *pModel = gtk_list_store_new (CD_MODEL_NB_COLUMNS,
		G_TYPE_STRING,  /* CD_MODEL_NAME */
		G_TYPE_STRING,  /* CD_MODEL_URI */
		G_TYPE_STRING,  /* CD_MODEL_PATH */
		GDK_TYPE_PIXBUF,  /* CD_MODEL_ICON */
		G_TYPE_INT64,  /* CD_MODEL_DATE */
		G_TYPE_UINT);  /* CD_MODEL_ID */
	myData.pModel = pModel;
	
	// tree-view
	GtkWidget *pOneWidget = gtk_tree_view_new ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (pOneWidget), GTK_TREE_MODEL (pModel));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pOneWidget), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (pOneWidget), TRUE);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pOneWidget));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	/**gtk_tree_selection_set_select_function (selection,
		(GtkTreeSelectionFunc) _cairo_dock_select_one_item_in_tree,
		NULL,
		NULL);*/
	g_signal_connect (G_OBJECT (pOneWidget), "button-release-event", G_CALLBACK (_on_click_module_tree_view), NULL);  // pour le menu du clic droit
	g_signal_connect (G_OBJECT (pOneWidget), "button-press-event", G_CALLBACK (_on_click_module_tree_view), NULL);  // pour le menu du clic droit
	
	g_object_set (G_OBJECT (pOneWidget), "tooltip-column", CD_MODEL_PATH, NULL);
	
	GtkTreeViewColumn* col;
	GtkCellRenderer *rend;
	// icon
	rend = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "pixbuf", CD_MODEL_ICON, NULL);
	// file name
	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (D_("File name"), rend, "text", CD_MODEL_NAME, NULL);
	gtk_tree_view_column_set_min_width (col, 200);
	gtk_tree_view_column_set_max_width (col, MAX (500, g_desktopGeometry.Xscreen.width / g_desktopGeometry.iNbScreens * .67));  // we don't know on which screen is place the container...
	gtk_tree_view_column_set_sort_column_id (col, CD_MODEL_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pOneWidget), col);
	// date
	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (D_("Last access"), rend, "text", CD_MODEL_DATE, NULL);
	gtk_tree_view_column_set_cell_data_func (col, rend, (GtkTreeCellDataFunc)_render_date, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (col, CD_MODEL_DATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pOneWidget), col);
	
	// barres de defilement
	GtkAdjustment *adj = gtk_adjustment_new (0., 0., 100., 1, 10, 10);
	gtk_scrollable_set_vadjustment (GTK_SCROLLABLE (pOneWidget), GTK_ADJUSTMENT (adj));

	GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (pScrolledWindow, "height-request", 300, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	#if GTK_CHECK_VERSION (3, 8, 0)
	gtk_container_add (GTK_CONTAINER (pScrolledWindow), pOneWidget);
	#else
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pOneWidget);
	#endif
	gtk_box_pack_start (GTK_BOX (pMainBox), pScrolledWindow, FALSE, FALSE, MARGIN);

	return pMainBox;
}

static void _on_dialog_destroyed (GldiModuleInstance *myApplet)
{
	myData.pDialog = NULL;
	myData.pEntry = NULL;  // interactive widget inside the dialog are destroyed with it.
	myData.iCurrentCaterogy = CD_EVENT_ALL;
	myData.pModel = NULL;	
}
static gboolean _show_dialog_delayed (gpointer data)
{
	cd_toggle_dialog ();
	#ifdef ZEITGEIST_1_0
	if (myData.pDialog != NULL)  // dialog built with success, quit.
	{
		myData.iSidTryDialog = 0;
		return FALSE;
	}
	else  // failed, retry up to 3 times.
	{
		myData.iNbTries ++;
		cd_debug (" %d tries", myData.iNbTries);
		if (myData.iNbTries >= 3)  // definitely no hope -> show a message to the user.
		{
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("You need to install the Zeitgeist data engine."), myIcon, myContainer, 6000, "same icon");
			myData.iSidTryDialog = 0;
			return FALSE;
		}
	}
	#endif
	return TRUE;
}
void cd_toggle_dialog (void)
{
	if (myData.pDialog != NULL)  // the dialog can be opened in the case it was called from the shortkey.
	{
		gldi_object_unref (GLDI_OBJECT(myData.pDialog));
		myData.pDialog = NULL;
	}
	else
	{
		// establish the connection to Zeitgesit.
		if (myData.pLog == NULL)  // first search.
		{
			cd_debug ("first search");
			myData.pLog = zeitgeist_log_new ();  // may launch the Zeitgeist daemon if it's not yet running.
		}
		#ifdef ZEITGEIST_1_0
		if (! zeitgeist_log_is_connected (myData.pLog))
		{
			cd_debug ("not yet connected");
			if (myData.iSidTryDialog == 0)
			{
				myData.iNbTries = 0;
				myData.iSidTryDialog = g_timeout_add_seconds (1, _show_dialog_delayed, NULL);
			}
			return;
		}
		#else
		gboolean bIsConnected;
		g_object_get (G_OBJECT (myData.pLog), "is-connected", &bIsConnected, NULL);
		if (! bIsConnected)
		{
			// will retry when it will be connected
			g_signal_connect (myData.pLog, "notify::is-connected",
				G_CALLBACK (_show_dialog_delayed), NULL);
			return;
		}
		#endif

		// build the dialog and the tree model.
		GtkWidget *pInteractiveWidget = cd_build_events_widget ();
		myData.pDialog = gldi_dialog_show (D_("Browse and search in recent events"),
			myIcon,
			myContainer,
			0,
			"same icon",
			pInteractiveWidget,
			NULL,
			myApplet,
			(GFreeFunc) _on_dialog_destroyed);
		gtk_widget_grab_focus (myData.pEntry);
		
		// trigger the search that will fill the model.
		cd_trigger_search ();
	}
}

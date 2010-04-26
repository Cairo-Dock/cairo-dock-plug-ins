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
#include <math.h>
#define __USE_POSIX
#include <signal.h>

#include "applet-struct.h"
#include "applet-calendar.h"

#define _cd_task_matches_month(pTask, iMonth, iYear) (((pTask)->iMonth == iMonth && ((pTask)->iYear == iYear || (pTask)->iFrequency == CD_TASK_EACH_YEAR)) || (pTask)->iFrequency == CD_TASK_EACH_MONTH)
#define _cd_task_matches_day(pTask, iDay, iMonth, iYear) ((pTask)->iDay == iDay && _cd_task_matches_month (pTask, iMonth, iYear))

typedef enum _CDClockTaskColumns
{
	CD_TASK_ID= 0,
	CD_TASK_ACTIVE,
	CD_TASK_TITLE,
	CD_TASK_TAGS,
	CD_TASK_TEXT,
	CD_TASK_TIME,
	CD_TASK_FREQ,
	CD_TASKS_NB_COLUMNS,
} CDClockTaskColumns;

static int _compare_task (CDClockTask *pTask1, CDClockTask *pTask2, CairoDockModuleInstance *myApplet);

void cd_clock_register_backend (CairoDockModuleInstance *myApplet, const gchar *cBackendName, CDClockTaskBackend *pBackend)
{
	if (myData.pBackends == NULL)
		myData.pBackends = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_free);
	g_hash_table_insert (myData.pBackends, g_strdup (cBackendName), pBackend);
}

CDClockTaskBackend *cd_clock_get_backend (CairoDockModuleInstance *myApplet, const gchar *cBackendName)
{
	CDClockTaskBackend *pBackend = NULL;
	if (cBackendName != NULL)
		pBackend = g_hash_table_lookup (myData.pBackends, cBackendName);
	
	return pBackend;
}

void cd_clock_set_current_backend (CairoDockModuleInstance *myApplet)
{
	if (myData.pBackend && myData.pBackend->stop)
		myData.pBackend->stop (myApplet);
	myData.pBackend = cd_clock_get_backend (myApplet, myConfig.cTaskMgrName);
	if (myData.pBackend == NULL)
		myData.pBackend = cd_clock_get_backend (myApplet, "Default");
	if (myData.pBackend->init)
		myData.pBackend->init (myApplet);
}

static CDClockTask *_cd_clock_get_task_by_id (const gchar *cID, CairoDockModuleInstance *myApplet)
{
	if (cID == NULL)
		return NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (strcmp (pTask->cID, cID) == 0)
			return pTask;
	}
	return NULL;
}

static gchar * _on_display_task_detail (GtkCalendar *calendar, guint iYear, guint iMonth, guint iDay, CairoDockModuleInstance *myApplet)
{
	if (myData.pTasks == NULL)
		return NULL;
	
	//g_print ("%s (%d/%d/%d)\n", __func__, iDay, iMonth, iYear);
	GString *sDetail = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (_cd_task_matches_day (pTask, iDay, iMonth, iYear))
		{
			if (sDetail == NULL)
				sDetail = g_string_new ("");
			g_string_append_printf (sDetail, "<b><u>%s</u></b>\n <i>at %d:%02d</i>\n %s\n", pTask->cTitle ? pTask->cTitle : D_("No title"), pTask->iHour, pTask->iMinute, pTask->cText);
		}
	}
	
	if (sDetail == NULL)
		return NULL;
	gchar *cDetail= sDetail->str;
	g_string_free (sDetail, FALSE);
	//g_print ("* detail : %s\n", cDetail);
	return cDetail;
}

static void _mark_days (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	guint iYear, iMonth, iDay;
	gtk_calendar_get_date (GTK_CALENDAR (pCalendar),
		&iYear,
		&iMonth,
		&iDay);
	
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (_cd_task_matches_month (pTask, iMonth, iYear))
		{
			gtk_calendar_mark_day (GTK_CALENDAR (pCalendar), pTask->iDay);
		}
	}
}

static void _on_day_selected (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	
}

static gboolean on_delete_task_window (GtkWidget *pWidget, GdkEvent *event, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	/// get day
	
	
	/// get text
	
	
	/// save task
	
	myData.pTaskWindow = NULL;
	return FALSE;  // detruit la fenetre.
}

static GtkListStore *cd_clock_create_model_for_current_day (guint iDay, guint iMonth, guint iYear, CairoDockModuleInstance *myApplet)
{
	g_print ("%s (%d/%d/%d)\n", __func__, iDay, iMonth, iYear);
	//\______________ On cree le modele.
	GtkListStore *pModel;
	if (myData.pModel != NULL)
	{
		gtk_list_store_clear (myData.pModel);
		pModel = myData.pModel;
	}
	else
	{
		pModel = gtk_list_store_new (CD_TASKS_NB_COLUMNS,
			G_TYPE_STRING,  // CD_TASK_ID
			G_TYPE_BOOLEAN,  // CD_TASK_ACTIVE
			G_TYPE_STRING,  // CD_TASK_TITLE
			G_TYPE_STRING,  // CD_TASK_TAGS
			G_TYPE_STRING,  // CD_TASK_TEXT
			G_TYPE_INT,  // CD_TASK_TIME
			G_TYPE_INT);  // CD_TASK_FREQ
		myData.pModel = pModel;
	}
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (pModel), CD_TASK_TIME, GTK_SORT_ASCENDING);
	
	//\______________ On remplit le modele.
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (_cd_task_matches_day (pTask, iDay, iMonth, iYear))
		{
			g_print (" + %s\n", pTask->cTitle);
			GtkTreeIter iter;
			memset (&iter, 0, sizeof (GtkTreeIter));
			gtk_list_store_append (GTK_LIST_STORE (pModel), &iter);
			gtk_list_store_set (GTK_LIST_STORE (pModel), &iter,
				CD_TASK_ID, pTask->cID,
				CD_TASK_ACTIVE, TRUE,
				CD_TASK_TITLE, pTask->cTitle,
				CD_TASK_TEXT, pTask->cText,
				CD_TASK_TAGS, 	 pTask->cTags,
				CD_TASK_TIME, pTask->iHour*60 + pTask->iMinute,
				CD_TASK_FREQ, pTask->iFrequency, -1);
		}
	}
	return pModel;
}

static void _cd_clock_add_new_task (GtkMenuItem *pMenuItem, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	CDClockTask *pTask = g_new0 (CDClockTask, 1);
	int iDay, iMonth, iYear;
	
	pTask->iDay = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (myData.pTaskWindow), "day"));
	pTask->iMonth = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (myData.pTaskWindow), "month"));
	pTask->iYear = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (myData.pTaskWindow), "year"));
	pTask->cTitle = g_strdup (D_("No title"));
	pTask->iHour = 12;
	gboolean bCreated = myData.pBackend->create_task (pTask, myApplet);
	if (bCreated)
	{
		myData.pTasks = g_list_insert_sorted (myData.pTasks, pTask, (GCompareFunc)_compare_task);
		
		GtkListStore *pModel = cd_clock_create_model_for_current_day (pTask->iDay, pTask->iMonth, pTask->iYear, myApplet);
		//gtk_tree_view_set_model (GTK_TREE_VIEW (myData.pTreeView), GTK_TREE_MODEL (pModel));
		gtk_widget_show_all (myData.pTaskWindow);
		
		gtk_calendar_clear_marks (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget));
		_mark_days (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget), myApplet);
	}
}
static void _cd_clock_delete_task (GtkMenuItem *pMenuItem, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	CairoDockModuleInstance *myApplet = data[0];
	CDClockTask *pTask = data[1];
	gboolean bDeleted = myData.pBackend->delete_task (pTask, myApplet);
	
	if (bDeleted)
	{
		myData.pTasks = g_list_remove (myData.pTasks, pTask);
		cd_clock_free_task (pTask);
		
		GtkListStore *pModel = cd_clock_create_model_for_current_day (pTask->iDay, pTask->iMonth, pTask->iYear, myApplet);
		//gtk_tree_view_set_model (GTK_TREE_VIEW (myData.pTreeView), GTK_TREE_MODEL (pModel));
		gtk_widget_show_all (myData.pTaskWindow);
		
		gtk_calendar_clear_marks (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget));
		_mark_days (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget), myApplet);
	}
}
static gboolean _on_click_tree_view (GtkTreeView *pTreeView, GdkEventButton* pButton, CairoDockModuleInstance *myApplet)
{
	static gpointer *data = NULL;
	if (pButton->button == 3 && pButton->type == GDK_BUTTON_RELEASE)
	{
		GtkWidget *pMenu = gtk_menu_new ();
		cairo_dock_add_in_menu_with_stock_and_data (D_("Add a new task"), GTK_STOCK_ADD, (GFunc)_cd_clock_add_new_task, pMenu, myApplet);
		
		GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
		GtkTreeModel *pModel;
		GtkTreeIter iter;
		if (gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		{
			gchar *cTaskID = NULL;
			gtk_tree_model_get (pModel, &iter,
				CD_TASK_ID, &cTaskID, -1);
			CDClockTask *pTask = _cd_clock_get_task_by_id (cTaskID, myApplet);
			g_free (cTaskID);
		
			if (pTask != NULL)
			{
				if (!data)
					data = g_new (gpointer, 2);
				
				data[0] = myApplet;
				data[1] = pTask;
				cairo_dock_add_in_menu_with_stock_and_data (D_("Delete this task"), GTK_STOCK_REMOVE, (GFunc)_cd_clock_delete_task, pMenu, data);
			}
		}
		gtk_widget_show_all (pMenu);
		gtk_menu_popup (GTK_MENU (pMenu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
	return FALSE;
}

static gboolean _cd_clock_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, CairoDockModuleInstance *myApplet)
{
	if (path_currently_selected)
		return TRUE;
	GtkTreeIter iter;
	if (! gtk_tree_model_get_iter (model, &iter, path))
		return FALSE;
	
	g_print ("%s ()\n", __func__);
	
	
	return TRUE;
}

static GtkListStore *_make_frequency_list_store (void)
{
	GtkListStore *freq_list = gtk_list_store_new (2, G_TYPE_INT, G_TYPE_STRING);
	GtkTreeIter iter;
	
	memset (&iter, 0, sizeof (GtkTreeIter));
	gtk_list_store_append (GTK_LIST_STORE (freq_list), &iter);
	gtk_list_store_set (GTK_LIST_STORE (freq_list), &iter,
		0, CD_TASK_DONT_REPEAT,
		1, D_("Don't repeat"), -1);
	
	memset (&iter, 0, sizeof (GtkTreeIter));
	gtk_list_store_append (GTK_LIST_STORE (freq_list), &iter);
	gtk_list_store_set (GTK_LIST_STORE (freq_list), &iter,
		0, CD_TASK_EACH_MONTH,
		1, D_("Each month"), -1);
	
	memset (&iter, 0, sizeof (GtkTreeIter));
	gtk_list_store_append (GTK_LIST_STORE (freq_list), &iter);
	gtk_list_store_set (GTK_LIST_STORE (freq_list), &iter,
		0, CD_TASK_EACH_YEAR,
		1, D_("Each year"), -1);
	
	return freq_list;
}

#define _get_task_from_path(new_text, path_string)\
	if (new_text == NULL || *new_text == '\0')\
		return;\
	GtkListStore *model = myData.pModel;\
	GtkTreeIter it;\
	if (! gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &it, path_string))\
		return ;\
	gchar *cID = NULL;\
	gtk_tree_model_get (GTK_TREE_MODEL (model), &it, CD_TASK_ID, &cID, -1);\
	CDClockTask *pTask = _cd_clock_get_task_by_id (cID, myApplet);\
	g_free (cID);\
	g_return_if_fail (pTask != NULL);
	
static void _on_change_title (GtkCellRendererText * cell, gchar * path_string, gchar * new_text, CairoDockModuleInstance *myApplet)
{
	_get_task_from_path (new_text, path_string);
	
	g_free (pTask->cTitle);
	pTask->cTitle = g_strdup (new_text);
	gboolean bUpdated = myData.pBackend->update_task (pTask, myApplet);
	if (bUpdated)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), &it, CD_TASK_TITLE, pTask->cTitle, -1);
	}
}

static void _on_change_text (GtkCellRendererText * cell, gchar * path_string, gchar * new_text, CairoDockModuleInstance *myApplet)
{
	_get_task_from_path (new_text, path_string);
	
	g_free (pTask->cText);
	pTask->cText = g_strdup (new_text);
	gboolean bUpdated = myData.pBackend->update_task (pTask, myApplet);
	if (bUpdated)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), &it, CD_TASK_TEXT, pTask->cText, -1);
	}
}

static void _on_change_tags (GtkCellRendererText * cell, gchar * path_string, gchar * new_text, CairoDockModuleInstance *myApplet)
{
	_get_task_from_path (new_text, path_string);
	
	g_free (pTask->cTags);
	pTask->cTags = g_strdup (new_text);
	gboolean bUpdated = myData.pBackend->update_task (pTask, myApplet);
	if (bUpdated)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), &it, CD_TASK_TAGS, pTask->cTags, -1);
	}
}

static void _on_change_time (GtkCellRendererText * cell, gchar * path_string, gchar * new_text, CairoDockModuleInstance *myApplet)
{
	_get_task_from_path(new_text, path_string);
	
	int h=0, m=0;
	sscanf (new_text, "%d:%d", &h, &m);
	pTask->iHour = MAX (0, MIN (23, h));
	pTask->iMinute = MAX (0, MIN (59, m));
	gboolean bUpdated = myData.pBackend->update_task (pTask, myApplet);
	if (bUpdated)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), &it, CD_TASK_TIME, pTask->iHour*60+pTask->iMinute, -1);
	}
}

static gboolean _search_frequency (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, gpointer *data)
{
	int iFrequency;
	gchar *cName = NULL;
	gtk_tree_model_get (model, iter,
		0, &iFrequency,
		1, &cName, -1);
	g_print ("freq %d : %s\n", iFrequency, cName);
	if (cName && strcmp (cName, data[0]) == 0)
	{
		data[1] = GINT_TO_POINTER (iFrequency);
		return TRUE;
	}
	return FALSE;
}
static void _on_change_frequency (GtkCellRendererText * cell, gchar * path_string, gchar * new_text, CairoDockModuleInstance *myApplet)
{
	_get_task_from_path(new_text, path_string);
	
	GtkListStore *pModel = NULL;
	g_object_get (cell, "model", &pModel, NULL);
	gpointer data[2] = {new_text, GINT_TO_POINTER (CD_TASK_NB_FREQUENCIES)};
	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _search_frequency, data);
	g_object_unref (pModel);
	
	pTask->iFrequency = GPOINTER_TO_INT (data[1]);
	gboolean bUpdated = myData.pBackend->update_task (pTask, myApplet);
	
	if (bUpdated)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), &it, CD_TASK_FREQ, pTask->iFrequency, -1);
	}
}

static void _cd_clock_render_frequency (GtkTreeViewColumn *tree_column, GtkCellRenderer *cell, GtkTreeModel *model,GtkTreeIter *iter, CairoDockModuleInstance *myApplet)
{
	int iFrequency;
	gtk_tree_model_get (model, iter, CD_TASK_FREQ, &iFrequency, -1);
	
	switch (iFrequency)
	{
		case CD_TASK_DONT_REPEAT :
		default :
			g_object_set (cell, "text", "-", NULL);
		break;
		case CD_TASK_EACH_MONTH :
			g_object_set (cell, "text", D_("each month"), NULL);
		break;
		case CD_TASK_EACH_YEAR :
			g_object_set (cell, "text", D_("each year"), NULL);
		break;
	}
}

static void _cd_clock_render_time (GtkTreeViewColumn *tree_column, GtkCellRenderer *cell, GtkTreeModel *model,GtkTreeIter *iter, CairoDockModuleInstance *myApplet)
{
	int iTime;
	gtk_tree_model_get (model, iter, CD_TASK_TIME, &iTime, -1);
	
	int h = iTime / 60;
	int m = iTime - h * 60;
	gchar *cTime = g_strdup_printf ("%02d:%02d", h, m);
	g_object_set (cell, "text", cTime, NULL);
	g_free (cTime);
}


static void _on_day_selected_double_click (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	if (myData.pTaskWindow != NULL)
	{
		/// save ... 
		
		gtk_widget_destroy (myData.pTaskWindow);
		myData.pTaskWindow = NULL;
	}
	
	//\______________ On remplit le modele avec la date courante.
	guint iYear, iMonth, iDay;
	gtk_calendar_get_date (pCalendar,
		&iYear,
		&iMonth,
		&iDay);
	GtkListStore *pModel = cd_clock_create_model_for_current_day (iDay, iMonth, iYear, myApplet);
	
	//\______________ On construit la fenetre.
	myData.pTaskWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar *cTitle = g_strdup_printf ("%d/%d/%d", (myConfig.bNormalDate ? iDay : iYear), iMonth, (myConfig.bNormalDate ? iYear : iDay));
	gtk_window_set_title (GTK_WINDOW (myData.pTaskWindow), cTitle);
	g_free (cTitle);
	g_object_set_data (G_OBJECT (myData.pTaskWindow), "day", GINT_TO_POINTER (iDay));
	g_object_set_data (G_OBJECT (myData.pTaskWindow), "month", GINT_TO_POINTER (iMonth));
	g_object_set_data (G_OBJECT (myData.pTaskWindow), "year", GINT_TO_POINTER (iYear));
	
	//\______________ On construit le treeview.
	GtkWidget *pTreeView = gtk_tree_view_new ();
	myData.pTreeView = pTreeView;
	gtk_tree_view_set_model (GTK_TREE_VIEW (pTreeView), GTK_TREE_MODEL (pModel));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pTreeView), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (pTreeView), TRUE);
	g_signal_connect (G_OBJECT (pTreeView), "button-release-event", G_CALLBACK (_on_click_tree_view), myApplet);
	GtkTreeViewColumn* col;
	GtkCellRenderer *rend;
	// title
	rend = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (rend), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT (rend), "edited", (GCallback) _on_change_title, myApplet);
	col = gtk_tree_view_column_new_with_attributes (D_("Title"), rend, "text", CD_TASK_TITLE, NULL);
	gtk_tree_view_column_set_sort_column_id (col, CD_TASK_TITLE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pTreeView), col);
	// text
	rend = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (rend), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT (rend), "edited", (GCallback) _on_change_text, myApplet);
	col = gtk_tree_view_column_new_with_attributes (D_("Text"), rend, "text", CD_TASK_TEXT, NULL);
	gtk_tree_view_column_set_sort_column_id (col, CD_TASK_TEXT);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pTreeView), col);
	// tags
	rend = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (rend), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT (rend), "edited", (GCallback) _on_change_tags, myApplet);
	col = gtk_tree_view_column_new_with_attributes (D_("Tags"), rend, "text", CD_TASK_TAGS, NULL);
	gtk_tree_view_column_set_sort_column_id (col, CD_TASK_TAGS);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pTreeView), col);
	// time
	rend = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (rend), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT (rend), "edited", (GCallback) _on_change_time, myApplet);
	col = gtk_tree_view_column_new_with_attributes (D_("Time"), rend, "text", CD_TASK_TIME, NULL);
	gtk_tree_view_column_set_cell_data_func (col, rend, (GtkTreeCellDataFunc)_cd_clock_render_time, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (col, CD_TASK_TIME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pTreeView), col);
	// frequency
	rend = gtk_cell_renderer_combo_new ();
	GtkListStore *freq_list = _make_frequency_list_store ();
	g_object_set (G_OBJECT (rend),
		"text-column", 1,
		"model", freq_list,
		"has-entry", FALSE,
		"editable", TRUE,
		NULL);
	g_signal_connect (G_OBJECT (rend), "edited", (GCallback) _on_change_frequency, myApplet);
	col = gtk_tree_view_column_new_with_attributes (D_("Freq."), rend, "text", CD_TASK_FREQ, NULL);
	gtk_tree_view_column_set_cell_data_func (col, rend, (GtkTreeCellDataFunc)_cd_clock_render_frequency, myApplet, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pTreeView), col);
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pTreeView));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	gtk_tree_selection_set_select_function (selection,
		(GtkTreeSelectionFunc) _cd_clock_select_one_item_in_tree,
		myApplet,
		NULL);
	
	//\______________ On l'ajoute a la fenetre.
	GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pTreeView);
	gtk_container_add (GTK_CONTAINER (myData.pTaskWindow), pScrolledWindow);
	//gtk_box_pack_start (GTK_BOX (pVBox), pScrolledWindow, FALSE, FALSE, 0);
	
	g_signal_connect (myData.pTaskWindow, "delete-event", G_CALLBACK (on_delete_task_window), myApplet);
	gtk_window_set_keep_above (GTK_WINDOW (myData.pTaskWindow), TRUE);
	gtk_window_set_modal (GTK_WINDOW (myData.pTaskWindow), TRUE);
	gtk_widget_show_all (myData.pTaskWindow);
	gtk_window_resize (GTK_WINDOW (myData.pTaskWindow), 640, 350);
}
static void _on_month_changed (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	gtk_calendar_clear_marks (pCalendar);
	_mark_days (pCalendar, myApplet);
}
static void _on_year_changed (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	gtk_calendar_clear_marks (pCalendar);
	_mark_days (pCalendar, myApplet);
}
static gboolean on_button_press_calendar (GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDockModuleInstance *myApplet)
{
	myData.iButtonPressTime = pButton->time;
	return FALSE;
}
static GtkWidget *cd_clock_build_calendar (CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	GtkWidget *pCalendar = gtk_calendar_new ();
	g_object_set (G_OBJECT (pCalendar), "show-details", FALSE, NULL);
	
	if (myData.pTasks == NULL)
	{
		myData.pTasks = cd_clock_list_tasks (myApplet);
	}
	
	_mark_days (GTK_CALENDAR (pCalendar), myApplet);
	
	g_signal_connect (G_OBJECT (pCalendar), "day-selected" , G_CALLBACK (_on_day_selected), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "day-selected-double-click" , G_CALLBACK (_on_day_selected_double_click), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "prev-month" , G_CALLBACK (_on_month_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "next-month" , G_CALLBACK (_on_month_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "prev-year" , G_CALLBACK (_on_year_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "next-year" , G_CALLBACK (_on_year_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar),
		"button-press-event",
		G_CALLBACK (on_button_press_calendar),
		myApplet);
	
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION > 14)
	gtk_calendar_set_detail_func (GTK_CALENDAR (pCalendar),
		(GtkCalendarDetailFunc) _on_display_task_detail,
		myApplet,
		(GDestroyNotify) NULL);
#endif
	return pCalendar;
}

void cd_clock_hide_dialogs (CairoDockModuleInstance *myApplet)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	myData.pCalendarDialog = NULL;
	if (myData.pTaskWindow != NULL)
	{
		gtk_widget_destroy (myData.pTaskWindow);
		myData.pTaskWindow = NULL;
	}
}

static gboolean on_button_press_dialog (GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (pButton->time > myData.iButtonPressTime)
	{
		cairo_dock_dialog_unreference (myData.pCalendarDialog);
		myData.pCalendarDialog = NULL;
	}
	CD_APPLET_LEAVE (FALSE);
}
void cd_clock_show_hide_calendar (CairoDockModuleInstance *myApplet)
{
	if (myData.pCalendarDialog != NULL)
	{
		cairo_dock_dialog_unreference (myData.pCalendarDialog);
		myData.pCalendarDialog = NULL;
		if (myData.pTaskWindow != NULL)
		{
			gtk_widget_destroy (myData.pTaskWindow);
			myData.pTaskWindow = NULL;
		}
	}
	else
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		GtkWidget *pCalendar = cd_clock_build_calendar (myApplet);
		myData.pCalendarDialog = cairo_dock_show_dialog_full (D_("Calendar"),
			myIcon, myContainer,
			0,
			MY_APPLET_SHARE_DATA_DIR"/dates.svg",
			pCalendar,
			NULL, NULL, NULL);
		g_signal_connect (G_OBJECT (myData.pCalendarDialog->container.pWidget),
			"button-press-event",
			G_CALLBACK (on_button_press_dialog),
			myApplet);
	}
}



static int _compare_task (CDClockTask *pTask1, CDClockTask *pTask2, CairoDockModuleInstance *myApplet)
{
	if (pTask1->iYear < pTask2->iYear)
		return -1;
	if (pTask1->iYear > pTask2->iYear)
		return 1;
	
	if (pTask1->iMonth < pTask2->iMonth)
		return -1;
	if (pTask1->iMonth > pTask2->iMonth)
		return 1;
	
	if (pTask1->iDay < pTask2->iDay)
		return -1;
	if (pTask1->iDay > pTask2->iDay)
		return 1;
	
	if (pTask1->iHour < pTask2->iHour)
		return -1;
	if (pTask1->iHour > pTask2->iHour)
		return 1;
	
	if (pTask1->iMinute < pTask2->iMinute)
		return -1;
	if (pTask1->iMinute > pTask2->iMinute)
		return 1;
	
	return 0;
	
}
GList *cd_clock_list_tasks (CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	GList *pTaskList = myData.pBackend->get_tasks (myApplet);
	
	pTaskList = g_list_sort_with_data (pTaskList,
		(GCompareDataFunc) _compare_task,
		myApplet);
	return pTaskList;
}


void cd_clock_free_task (CDClockTask *pTask)
{
	if (pTask == NULL)
		return;
	g_free (pTask->cTitle);
	g_free (pTask->cText);
	g_free (pTask->cTags);
	g_free (pTask->cID);
	g_free (pTask);
}

void cd_clock_reset_tasks_list (CairoDockModuleInstance *myApplet)
{
	g_list_foreach (myData.pTasks, (GFunc)cd_clock_free_task, NULL);
	g_list_free (myData.pTasks);
	myData.pTasks = NULL;
}

gchar *cd_clock_get_tasks_for_today (CairoDockModuleInstance *myApplet)
{
	if (myData.pTasks == NULL)
	{
		myData.pTasks = cd_clock_list_tasks (myApplet);
	}
	
	time_t epoch = (time_t) time (NULL);
	struct tm currentTime;
	localtime_r (&epoch, &currentTime);
	guint iDay = currentTime.tm_mday, iMonth = currentTime.tm_mon, iYear = currentTime.tm_year;
	
	GString *sTaskString = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (_cd_task_matches_day (pTask, iDay, iMonth, iYear))
		{
			if (sTaskString == NULL)
				sTaskString = g_string_new ("");
			g_string_append_printf (sTaskString, "<b><u>%s</u></b>\n <i>at %d:%02d</i>\n %s\n", pTask->cTitle ? pTask->cTitle : D_("No title"), pTask->iHour, pTask->iMinute, pTask->cText);
		}
	}
	
	if (sTaskString == NULL)
		return NULL;
	
	gchar *cTasks = sTaskString->str;
	g_string_free (sTaskString, FALSE);
	return cTasks;
}

gchar *cd_clock_get_tasks_for_this_week (CairoDockModuleInstance *myApplet)
{
	if (myData.pTasks == NULL)
	{
		myData.pTasks = cd_clock_list_tasks (myApplet);
	}
	
	time_t epoch = (time_t) time (NULL);
	struct tm currentTime;
	localtime_r (&epoch, &currentTime);
	guint iDay = currentTime.tm_mday, iMonth = currentTime.tm_mon, iYear = currentTime.tm_year;
	
	GDate* pCurrentDate = g_date_new_dmy (iDay, iMonth + 1, iYear);
	GDate* pDate = g_date_new ();
	int iDelta;
	GString *sTaskString = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		g_date_set_dmy (pDate,pTask->iDay, pTask->iMonth+1, pTask->iYear);
		iDelta = g_date_days_between (pCurrentDate, pDate) ;
		if (iDelta >= 0 && iDelta < 7)
		{
			if (sTaskString == NULL)
				sTaskString = g_string_new ("");
			g_string_append_printf (sTaskString, "<b><u>%s</u></b>\n <i>at %d:%02d</i>\n %s\n", pTask->cTitle ? pTask->cTitle : D_("No title"), pTask->iHour, pTask->iMinute, pTask->cText);
		}
		else if (iDelta >= 7)
			break;
	}
	g_date_free (pCurrentDate);
	
	if (sTaskString == NULL)
		return NULL;
	
	gchar *cTasks = sTaskString->str;
	g_string_free (sTaskString, FALSE);
	return cTasks;
}

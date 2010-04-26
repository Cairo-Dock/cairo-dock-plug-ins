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
		if (pTask->iDay == iDay && pTask->iMonth == iMonth && pTask->iYear == iYear)
		{
			//g_print ("show detail for %d/%d/%d\n", iDay, iMonth, iYear);
			if (sDetail == NULL)
				sDetail = g_string_new ("");
			g_string_append_printf (sDetail, "<b>%d/%d/%d (%d:%02d)</b>\n  %s\n", (myConfig.bNormalDate ? iDay : iYear), iMonth, (myConfig.bNormalDate ? iYear : iDay), pTask->iHour, pTask->iMinute, pTask->cText);
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
		if (pTask->iMonth == iMonth && pTask->iYear == iYear)
		{
			g_print ("mark %d/%d/%d\n", iDay, iMonth, iYear);
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
	
	return FALSE;  // detruit la fenetre.
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
	
	guint iYear, iMonth, iDay;
	gtk_calendar_get_date (GTK_CALENDAR (pCalendar),
		&iYear,
		&iMonth,
		&iDay);
	g_print (" -> %d/%d/%d\n", iDay, iMonth, iYear);
	
	myData.pTaskWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar *cTitle = g_strdup_printf ("%d/%d/%d", (myConfig.bNormalDate ? iDay : iYear), iMonth, (myConfig.bNormalDate ? iYear : iDay));
	gtk_window_set_title (GTK_WINDOW (myData.pTaskWindow), cTitle);
	g_free (cTitle);
	
	GString *sDetail = g_string_new ("");
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (pTask->iMonth == iMonth && pTask->iYear == iYear)
		{
			g_string_append_printf (sDetail, "%s\n", pTask->cText);
		}
	}
	
	GtkTextBuffer *pBuffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (pBuffer,
		sDetail->str,
		-1);
	g_string_free (sDetail, TRUE);
	GtkWidget *pTextView = gtk_text_view_new_with_buffer (pBuffer);
	gtk_container_add (GTK_CONTAINER (myData.pTaskWindow), pTextView);
	
	g_signal_connect (myData.pTaskWindow, "delete-event", G_CALLBACK (on_delete_task_window), myApplet);
	
	gtk_window_set_keep_above (GTK_WINDOW (myData.pTaskWindow), TRUE);
	gtk_window_set_modal (GTK_WINDOW (myData.pTaskWindow), TRUE);
	gtk_widget_show_all (myData.pTaskWindow);
}
static void _on_month_changed (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	gtk_calendar_clear_marks (pCalendar);
	_mark_days (pCalendar, myApplet);
}
static void _on_year_changed (GtkCalendar *pCalendar, CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	gtk_calendar_clear_marks (pCalendar);
	_mark_days (pCalendar, myApplet);
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
	
	gtk_calendar_set_detail_func (GTK_CALENDAR (pCalendar),
		(GtkCalendarDetailFunc) _on_display_task_detail,
		myApplet,
		(GDestroyNotify) NULL);
	
	return pCalendar;
}

void cd_clock_show_hide_calendar (CairoDockModuleInstance *myApplet)
{
	if (myData.pCalendarDialog != NULL)
	{
		cairo_dock_dialog_unreference (myData.pCalendarDialog);
		myData.pCalendarDialog = NULL;
		gtk_widget_destroy (myData.pTaskWindow);
		myData.pTaskWindow = NULL;
	}
	else
	{
		GtkWidget *pCalendar = cd_clock_build_calendar (myApplet);
		myData.pCalendarDialog = cairo_dock_show_dialog_full (D_("Calendar"),
			myIcon, myContainer,
			0,
			MY_APPLET_SHARE_DATA_DIR"/dates.svg",
			pCalendar,
			NULL, NULL, NULL);
	}
}


GList *cd_clock_list_tasks (CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	gchar *cFile = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "clock", "tasks.conf");
	GKeyFile*pKeyFile = cairo_dock_open_key_file (cFile);
	g_free (cFile);
	
	gsize length=0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	if (pGroupList == NULL)
	{
		g_key_file_free (pKeyFile);
		return NULL;
	}
	
	CDClockTask *pTask;
	gchar *cTaskID;
	GList *pTaskList = NULL;
	guint i;
	for (i = 0; i < length; i ++)
	{
		cTaskID = pGroupList[i];
		pTask = g_new0 (CDClockTask, 1);
		g_print ("+ task %s\n", cTaskID);
		
		pTask->cText = g_key_file_get_string (pKeyFile, cTaskID, "text", NULL);
		pTask->iHour = g_key_file_get_integer (pKeyFile, cTaskID, "hour", NULL);
		pTask->iMinute = g_key_file_get_integer (pKeyFile, cTaskID, "minute", NULL);
		pTask->iDay = g_key_file_get_integer (pKeyFile, cTaskID, "day", NULL);
		pTask->iMonth = g_key_file_get_integer (pKeyFile, cTaskID, "month", NULL);
		pTask->iYear = g_key_file_get_integer (pKeyFile, cTaskID, "year", NULL);
		
		pTaskList = g_list_prepend (pTaskList, pTask);
		g_free (cTaskID);
	}
	
	g_free (pGroupList);
	g_key_file_free (pKeyFile);
	return pTaskList;
}

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
#include <signal.h>

#include "applet-struct.h"
#include "applet-task-editor.h"
#include "applet-calendar.h"

#define _cd_task_matches_month(pTask, iMonth, iYear) (((pTask)->iMonth == iMonth && ((pTask)->iYear == iYear || (pTask)->iFrequency == CD_TASK_EACH_YEAR)) || (pTask)->iFrequency == CD_TASK_EACH_MONTH)
#define _cd_task_matches_day(pTask, iDay, iMonth, iYear) ((pTask)->iDay == iDay && _cd_task_matches_month (pTask, iMonth, iYear))


  /////////////
 // BACKEND //
/////////////

void cd_clock_register_backend (GldiModuleInstance *myApplet, const gchar *cBackendName, CDClockTaskBackend *pBackend)
{
	if (myData.pBackends == NULL)
		myData.pBackends = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_free);
	g_hash_table_insert (myData.pBackends, g_strdup (cBackendName), pBackend);
}

CDClockTaskBackend *cd_clock_get_backend (GldiModuleInstance *myApplet, const gchar *cBackendName)
{
	CDClockTaskBackend *pBackend = NULL;
	if (cBackendName != NULL)
		pBackend = g_hash_table_lookup (myData.pBackends, cBackendName);
	
	return pBackend;
}

void cd_clock_set_current_backend (GldiModuleInstance *myApplet)
{
	if (myData.pBackend && myData.pBackend->stop)
		myData.pBackend->stop (myApplet);
	myData.pBackend = cd_clock_get_backend (myApplet, myConfig.cTaskMgrName);
	if (myData.pBackend == NULL)
		myData.pBackend = cd_clock_get_backend (myApplet, "Default");
	if (myData.pBackend->init)
		myData.pBackend->init (myApplet);
}


  ///////////
 // TASKS //
///////////

static int _compare_task (CDClockTask *pTask1, CDClockTask *pTask2, gpointer data)
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
void cd_clock_list_tasks (GldiModuleInstance *myApplet)
{
	cd_message ("%s ()", __func__);
	if (myData.pTasks != NULL)
		cd_clock_reset_tasks_list (myApplet);
	
	myData.pTasks = myData.pBackend->get_tasks (myApplet);
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		pTask->pApplet = myApplet;
	}
	myData.pTasks = g_list_sort_with_data (myData.pTasks,
		(GCompareDataFunc) _compare_task,
		NULL);
	myData.pNextTask = cd_clock_get_next_scheduled_task (myApplet);
	myData.pNextAnniversary = cd_clock_get_next_anniversary (myApplet);
}

void cd_clock_add_task_to_list (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	pTask->pApplet = myApplet;
	myData.pTasks = g_list_insert_sorted (myData.pTasks, pTask, (GCompareFunc)_compare_task);
	myData.pNextTask = cd_clock_get_next_scheduled_task (myApplet);
	myData.pNextAnniversary = cd_clock_get_next_anniversary (myApplet);
}

void cd_clock_remove_task_from_list (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	myData.pTasks = g_list_remove (myData.pTasks, pTask);
	myData.pMissedTasks = g_list_remove (myData.pMissedTasks, pTask);
	myData.pNextTask = cd_clock_get_next_scheduled_task (myApplet);
	myData.pNextAnniversary = cd_clock_get_next_anniversary (myApplet);
}

void cd_clock_free_task (CDClockTask *pTask)
{
	if (pTask == NULL)
		return;
	if (pTask->iSidWarning != 0)
		g_source_remove (pTask->iSidWarning);
	gldi_object_unref (GLDI_OBJECT(pTask->pWarningDialog));
	g_free (pTask->cTitle);
	g_free (pTask->cText);
	g_free (pTask->cTags);
	g_free (pTask->cID);
	g_free (pTask);
}

void cd_clock_reset_tasks_list (GldiModuleInstance *myApplet)
{
	g_list_foreach (myData.pTasks, (GFunc)cd_clock_free_task, NULL);
	g_list_free (myData.pTasks);
	g_list_free (myData.pMissedTasks);
	myData.pTasks = NULL;
	myData.pNextTask = NULL;
	myData.pMissedTasks = NULL;
}

CDClockTask *cd_clock_get_task_by_id (const gchar *cID, GldiModuleInstance *myApplet)
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

gchar *cd_clock_get_tasks_for_today (GldiModuleInstance *myApplet)
{
	guint iDay = myData.currentTime.tm_mday, iMonth = myData.currentTime.tm_mon, iYear = myData.currentTime.tm_year + 1900;
	
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
			g_string_append_printf (sTaskString, "<b><u>%s</u></b>\n <i>at %d:%02d</i>\n %s\n", pTask->cTitle ? pTask->cTitle : D_("No title"), pTask->iHour, pTask->iMinute, pTask->cText?pTask->cText:"");
		}
	}
	
	if (sTaskString == NULL)
		return NULL;
	
	gchar *cTasks = sTaskString->str;
	g_string_free (sTaskString, FALSE);
	return cTasks;
}

gchar *cd_clock_get_tasks_for_this_week (GldiModuleInstance *myApplet)
{
	guint iDay = myData.currentTime.tm_mday, iMonth = myData.currentTime.tm_mon, iYear = myData.currentTime.tm_year + 1900;
	
	GDate* pCurrentDate = g_date_new_dmy (iDay, iMonth + 1, iYear);
	GDate* pDate = g_date_new ();
	guint d, m, y;
	int iDelta;
	GString *sTaskString = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		switch (pTask->iFrequency)
		{
			case CD_TASK_DONT_REPEAT:
			default:
				d = pTask->iDay;
				m = pTask->iMonth+1;
				y = pTask->iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
			break;
			
			case CD_TASK_EACH_MONTH:
				d = pTask->iDay;
				m = iMonth+1;
				y = iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
				if (iDelta < 0)  // pDate est avant pCurrentDate => on teste le mois d'apres.
				{
					if (iMonth < 11)
					{
						m = iMonth+2;
						g_date_set_dmy (pDate, d, m, y);
					}
					else
					{
						m = 1;
						y = pTask->iYear + 1;
						g_date_set_dmy (pDate, d, m, y);
					}
					iDelta = g_date_days_between (pCurrentDate, pDate);
				}
			break;
			
			case CD_TASK_EACH_YEAR:
				d = pTask->iDay;
				m = pTask->iMonth+1;
				y = iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
				//g_print ("iDelta : %d/%d/%d -> %d (%s)\n", d, m, y, iDelta, pTask->cTitle);
				if (iDelta < 0)  // pDate est avant pCurrentDate => on teste l'annee d'apres.
				{
					y = iYear + 1;
					g_date_set_dmy (pDate, d, m, y);
					iDelta = g_date_days_between (pCurrentDate, pDate);
				}
			break;
		}
		
		if (iDelta >= 0 && iDelta < 7)
		{
			if (sTaskString == NULL)
				sTaskString = g_string_new ("");
			g_string_append_printf (sTaskString, "<b><u>%s</u></b>\n <i>%d/%d/%d at %d:%02d</i>\n %s\n",
				pTask->cTitle ? pTask->cTitle : D_("No title"),
				(myConfig.bNormalDate ? d : y), m, (myConfig.bNormalDate ? y : d),
				pTask->iHour, pTask->iMinute,
				pTask->cText?pTask->cText:"");
		}  // on n'arrete pas le parcours si iDelta > 7 pour prendre en compte aussi les anniv.
	}
	g_date_free (pCurrentDate);
	g_date_free (pDate);
	
	if (sTaskString == NULL)
		return NULL;
	
	gchar *cTasks = sTaskString->str;
	g_string_free (sTaskString, FALSE);
	return cTasks;
}

#define _compute_index(y,m,d,h,mi) ((((y*12+m)*32+d)*24+h)*60+mi)
CDClockTask *cd_clock_get_next_scheduled_task (GldiModuleInstance *myApplet)
{
	if (myData.pTasks == NULL)
		return NULL;
	
	guint iDay = myData.currentTime.tm_mday;
	guint iMonth = myData.currentTime.tm_mon;
	guint iYear = myData.currentTime.tm_year + 1900;
	guint iHour = myData.currentTime.tm_hour;
	guint iMinute = myData.currentTime.tm_min;
	gulong iIndex = _compute_index (iYear, iMonth, iDay, iHour, iMinute);
	gulong i, iNextIndex=0;
	//g_print ("%s (%d/%d/%d -> %ld)\n", __func__, iDay, iMonth, iYear, iIndex);
	
	CDClockTask *pNextTask = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		//g_print ("test de %s (%d/%d/%d)\n", pTask->cTitle, pTask->iDay, pTask->iMonth, pTask->iYear);
		switch (pTask->iFrequency)
		{
			case CD_TASK_DONT_REPEAT:
			default:
				i = _compute_index (pTask->iYear, pTask->iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);
				//g_print (" normal : %ld\n", i);
			break;
			
			case CD_TASK_EACH_MONTH:
				i = _compute_index (iYear, iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);  // index pour le mois courant.
				if (i < iIndex)  // on tombe avant, on calcule l'index pour le mois suivant.
				{
					if (iMonth < 11)
						i = _compute_index (iYear, iMonth+1, pTask->iDay, pTask->iHour, pTask->iMinute);
					else
						i = _compute_index (iYear+1, 0, pTask->iDay, pTask->iHour, pTask->iMinute);
				}
				//g_print (" mensuel : %ld\n", i);
			break;
			
			case CD_TASK_EACH_YEAR:
				i = _compute_index (iYear, pTask->iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);
				if (i < iIndex)  // on tombe avant, on calcule l'index pour l'annee suivante.
					i = _compute_index (iYear+1, pTask->iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);
				//g_print (" annuel : %ld\n", i);
			break;
		}
		if (i >= iIndex && (iNextIndex == 0 || i < iNextIndex))
		{
			iNextIndex = i;
			pNextTask = pTask;
			//g_print ("pNextTask <- %s, index <- %ld\n", pNextTask->cTitle, iNextIndex);
		}
	}
	return pNextTask;
}

CDClockTask *cd_clock_get_next_anniversary (GldiModuleInstance *myApplet)
{
	if (myData.pTasks == NULL)
		return NULL;
	
	guint iDay = myData.currentTime.tm_mday;
	guint iMonth = myData.currentTime.tm_mon;
	guint iYear = myData.currentTime.tm_year + 1900;
	guint iHour = myData.currentTime.tm_hour;
	guint iMinute = myData.currentTime.tm_min;
	gulong iIndex = _compute_index (iYear, iMonth, iDay, iHour, iMinute);
	gulong i, iNextIndex=0;
	//g_print ("%s (%d/%d/%d -> %ld)\n", __func__, iDay, iMonth, iYear, iIndex);
	
	CDClockTask *pNextAnniversary = NULL;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (pTask->iFrequency != CD_TASK_EACH_YEAR)
			continue;
		//g_print ("test de %s (%d/%d/%d)\n", pTask->cTitle, pTask->iDay, pTask->iMonth, pTask->iYear);
		
		i = _compute_index (iYear, pTask->iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);
		if (i < iIndex)  // on tombe avant, on calcule l'index pour l'annee suivante.
			i = _compute_index (iYear+1, pTask->iMonth, pTask->iDay, pTask->iHour, pTask->iMinute);
		//g_print (" annuel : %ld\n", i);
		
		if (i > iIndex && (iNextIndex == 0 || i < iNextIndex))
		{
			iNextIndex = i;
			pNextAnniversary = pTask;
			//g_print ("pNextTask <- %s, index <- %ld\n", pNextTask->cTitle, iNextIndex);
		}
	}
	return pNextAnniversary;
}


GList *cd_clock_get_missed_tasks (GldiModuleInstance *myApplet)
{
	GList *pTaskList = NULL;
	guint iDay = myData.currentTime.tm_mday;
	guint iMonth = myData.currentTime.tm_mon;
	guint iYear = myData.currentTime.tm_year + 1900;
	guint iHour = myData.currentTime.tm_hour;
	guint iMinute = myData.currentTime.tm_min;
	
	GDate* pCurrentDate = g_date_new_dmy (iDay, iMonth + 1, iYear);
	GDate* pDate = g_date_new ();
	guint d, m, y;
	int iDelta;
	CDClockTask *pTask;
	GList *t;
	for (t = myData.pTasks; t != NULL; t = t->next)
	{
		pTask = t->data;
		if (pTask->bAcknowledged)
			continue;
		
		switch (pTask->iFrequency)
		{
			case CD_TASK_DONT_REPEAT:
			default:
				d = pTask->iDay;
				m = pTask->iMonth+1;
				y = pTask->iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
			break;
			
			case CD_TASK_EACH_MONTH:
				d = pTask->iDay;
				m = iMonth+1;
				y = iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
				if (iDelta > 0)  // pDate est apres pCurrentDate => on teste le mois d'avant.
				{
					if (iMonth > 0)
					{
						m = iMonth;
						g_date_set_dmy (pDate, d, m, y);
					}
					else
					{
						m = 12;
						y = pTask->iYear - 1;
						g_date_set_dmy (pDate, d, m, y);
					}
					iDelta = g_date_days_between (pCurrentDate, pDate);
				}
			break;
			
			case CD_TASK_EACH_YEAR:
				d = pTask->iDay;
				m = pTask->iMonth+1;
				y = iYear;
				g_date_set_dmy (pDate, d, m, y);
				iDelta = g_date_days_between (pCurrentDate, pDate);
				//g_print ("iDelta : %d/%d/%d -> %d (%s)\n", d, m, y, iDelta, pTask->cTitle);
				if (iDelta > 0)  // pDate est apres pCurrentDate => on teste l'annee d'avant.
				{
					y = iYear - 1;
					g_date_set_dmy (pDate, d, m, y);
					iDelta = g_date_days_between (pCurrentDate, pDate);
				}
			break;
		}
		
		if (iDelta <= 0 && iDelta > -7)
		{
			if (iDelta == 0)  // today's task, check time
			{
				if (pTask->iHour > iHour || (pTask->iHour == iHour && pTask->iMinute > iMinute))  // it's in the future, skip it.
					continue;
			}
			pTaskList = g_list_prepend (pTaskList, pTask);
		}  // on n'arrete pas le parcours si iDelta > 7 pour prendre en compte aussi les anniv.
	}
	g_date_free (pCurrentDate);
	g_date_free (pDate);
	
	return pTaskList;
}


  //////////////
 // CALENDAR //
//////////////

static void _mark_days (GtkCalendar *pCalendar, GldiModuleInstance *myApplet)
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
void cd_clock_update_calendar_marks (GldiModuleInstance *myApplet)
{
	if (myData.pCalendarDialog != NULL)
	{
		gtk_calendar_clear_marks (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget));
		_mark_days (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget), myApplet);
	}
}

static gchar * _on_display_task_detail (GtkCalendar *calendar, guint iYear, guint iMonth, guint iDay, GldiModuleInstance *myApplet)
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
			if (pTask->iFrequency == CD_TASK_EACH_YEAR && iYear > pTask->iYear)
				g_string_append_printf (sDetail, "<b><u>%s</u> (%d %s)</b>\n <i>at %d:%02d</i>\n %s\n",
					pTask->cTitle ? pTask->cTitle : D_("No title"),
					iYear - pTask->iYear, D_("years"),
					pTask->iHour, pTask->iMinute,
					pTask->cText?pTask->cText:"");
			else
				g_string_append_printf (sDetail, "<b><u>%s</u></b>\n <i>at %d:%02d</i>\n %s\n", pTask->cTitle ? pTask->cTitle : D_("No title"),
				pTask->iHour, pTask->iMinute,
				pTask->cText?pTask->cText:"");
		}
	}
	
	if (sDetail == NULL)
		return NULL;
	gchar *cDetail= sDetail->str;
	g_string_free (sDetail, FALSE);
	//g_print ("* detail : %s\n", cDetail);
	return cDetail;
}

static void _on_day_selected_double_click (GtkCalendar *pCalendar, GldiModuleInstance *myApplet)
{
	guint iDay, iMonth, iYear;
	gtk_calendar_get_date (pCalendar,
		&iYear,
		&iMonth,
		&iDay);
	cd_clock_build_task_editor (iDay, iMonth, iYear, myApplet);
}

static void _on_date_changed (GtkCalendar *pCalendar, GldiModuleInstance *myApplet)
{
	gtk_calendar_clear_marks (pCalendar);
	_mark_days (pCalendar, myApplet);
}

static void _on_add_task (GtkWidget *pMenuItem, GldiModuleInstance *myApplet)
{
	guint iDay, iMonth, iYear;
	gtk_calendar_get_date (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget),
		&iYear,
		&iMonth,
		&iDay);
	
	CDClockTask *pTask = g_new0 (CDClockTask, 1);
	pTask->iDay = iDay;
	pTask->iMonth = iMonth;
	pTask->iYear = iYear;
	pTask->cTitle = g_strdup (D_("No title"));
	pTask->iHour = 12;
	gboolean bCreated = myData.pBackend->create_task (pTask, myApplet);
	if (bCreated)
	{
		cd_clock_add_task_to_list (pTask, myApplet);
		
		cd_clock_update_calendar_marks (myApplet);
	}
	
	cd_clock_build_task_editor (iDay, iMonth, iYear, myApplet);
}
static void _on_edit_tasks (GtkWidget *pMenuItem, GldiModuleInstance *myApplet)
{
	guint iDay, iMonth, iYear;
	gtk_calendar_get_date (GTK_CALENDAR (myData.pCalendarDialog->pInteractiveWidget),
		&iYear,
		&iMonth,
		&iDay);
	cd_clock_build_task_editor (iDay, iMonth, iYear, myApplet);
}
static gboolean on_button_released_calendar (GtkWidget *widget,
	GdkEventButton *pButton,
	GldiModuleInstance *myApplet)
{
	if (pButton->button == 3)  // right-click
	{
		GtkWidget *pMenu = gldi_menu_new (NULL);
		
		// add a task
		cairo_dock_add_in_menu_with_stock_and_data (D_("Add a new task"), GLDI_ICON_NAME_ADD, G_CALLBACK (_on_add_task), pMenu, myApplet);
		
		// edit tasks
		gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Edit tasks"), D_("double-click"));
		cairo_dock_add_in_menu_with_stock_and_data (cLabel, GLDI_ICON_NAME_EDIT, G_CALLBACK (_on_edit_tasks), pMenu, myApplet);
		g_free (cLabel);
		
		gtk_widget_show_all (GTK_WIDGET (pMenu));
		gtk_menu_popup (GTK_MENU (pMenu),
			NULL,
			NULL,
			NULL,
			NULL,  // data
			1,
			gtk_get_current_event_time ());
	}
	return FALSE;
}

static GtkWidget *cd_clock_build_calendar (GldiModuleInstance *myApplet)
{
	cd_message ("%s ()", __func__);
	GtkWidget *pCalendar = gtk_calendar_new ();
	g_object_set (G_OBJECT (pCalendar), "show-details", FALSE, NULL);
	
	_mark_days (GTK_CALENDAR (pCalendar), myApplet);
	
	// reload the marks when the month/year changes
	g_signal_connect (G_OBJECT (pCalendar), "prev-month" , G_CALLBACK (_on_date_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "next-month" , G_CALLBACK (_on_date_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "prev-year" , G_CALLBACK (_on_date_changed), myApplet);
	g_signal_connect (G_OBJECT (pCalendar), "next-year" , G_CALLBACK (_on_date_changed), myApplet);
	// edit tasks on double-click or right-click
	g_signal_connect (G_OBJECT (pCalendar), "day-selected-double-click" , G_CALLBACK (_on_day_selected_double_click), myApplet);  // it's not a good idea to show the task-editor on left-click, because we can receve the 'click' event when clicking on the month/year buttons, and the 'day-selected' event when we change the month/year.
	g_signal_connect (G_OBJECT (pCalendar),
		"button-release-event",
		G_CALLBACK (on_button_released_calendar),
		myApplet);

	gtk_calendar_set_detail_func (GTK_CALENDAR (pCalendar),
		(GtkCalendarDetailFunc) _on_display_task_detail,
		myApplet,
		(GDestroyNotify) NULL);
	return pCalendar;
}

void cd_clock_hide_dialogs (GldiModuleInstance *myApplet)
{
	gldi_dialogs_remove_on_icon (myIcon);
	myData.pCalendarDialog = NULL;
}


static void _on_dialog_destroyed (GldiModuleInstance *myApplet)
{
	myData.pCalendarDialog = NULL;
}
void cd_clock_show_hide_calendar (GldiModuleInstance *myApplet)
{
	cd_debug ("%s (%x)", __func__, myData.pCalendarDialog);
	if (myData.pCalendarDialog != NULL)
	{
		gldi_object_unref (GLDI_OBJECT(myData.pCalendarDialog));
		myData.pCalendarDialog = NULL;
		if (myData.pTaskWindow != NULL)
		{
			gtk_widget_destroy (myData.pTaskWindow);
			myData.pTaskWindow = NULL;
			myData.pModel = NULL;
		}
	}
	else
	{
		gldi_dialogs_remove_on_icon (myIcon);
		GtkWidget *pCalendar = cd_clock_build_calendar (myApplet);
		myData.pCalendarDialog = gldi_dialog_show (D_("Calendar and tasks"),
			myIcon, myContainer,
			0,
			MY_APPLET_SHARE_DATA_DIR"/dates.svg",
			pCalendar,
			NULL,
			myApplet,
			(GFreeFunc)_on_dialog_destroyed);
	}
}

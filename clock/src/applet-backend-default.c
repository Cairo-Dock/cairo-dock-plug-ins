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
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-calendar.h"
#include "applet-backend-default.h"

static int s_iCounter = 0;

static GList *get_tasks (GldiModuleInstance *myApplet)
{
	gchar *cDirPath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, "clock");
	if (! g_file_test (cDirPath, G_FILE_TEST_EXISTS))
	{
		if (g_mkdir (cDirPath, 7*8*8+7*8+5) != 0)
		{
			cd_warning ("couldn't create directory %s", cDirPath);
			g_free (cDirPath);
			return NULL;
		}
		g_free (cDirPath);
		return NULL;
	}
	gchar *cFile = g_strdup_printf ("%s/%s", cDirPath, "tasks.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cFile);
	
	g_free (cDirPath);
	if (pKeyFile == NULL)  // encore aucune taches.
	{
		g_free (cFile);
		return NULL;
	}
	
	gsize length=0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	if (pGroupList == NULL)
	{
		g_key_file_free (pKeyFile);
		g_free (cFile);
		return NULL;
	}
	
	CDClockTask *pTask;
	gchar *cTaskID;
	GList *pTaskList = NULL;
	guint iMonth = myData.currentTime.tm_mon;
	guint iYear = myData.currentTime.tm_year + 1900;
	guint y, m, freq;
	gboolean bUpdateFile = FALSE;
	guint i;
	for (i = 0; i < length; i ++)
	{
		cTaskID = pGroupList[i];
		
		// discard tasks older than 1 year (clean the file too).
		y = g_key_file_get_integer (pKeyFile, cTaskID, "year", NULL);
		m = g_key_file_get_integer (pKeyFile, cTaskID, "month", NULL);
		freq = g_key_file_get_integer (pKeyFile, cTaskID, "freq", NULL);
		if (freq == CD_TASK_DONT_REPEAT && (iYear < y - 1 || (iYear == y - 1 && iMonth < m)))
		{
			g_key_file_remove_group (pKeyFile, cTaskID, NULL);
			bUpdateFile = TRUE;
			g_free (cTaskID);
			continue;
		}
		
		// append the task.
		pTask = g_new0 (CDClockTask, 1);
		pTask->cID = cTaskID;
		pTask->iDay = g_key_file_get_integer (pKeyFile, cTaskID, "day", NULL);
		pTask->iMonth = m;
		pTask->iYear = y;
		pTask->cTitle = g_key_file_get_string (pKeyFile, cTaskID, "title", NULL);
		pTask->cText = g_key_file_get_string (pKeyFile, cTaskID, "text", NULL);
		pTask->cTags = g_key_file_get_string (pKeyFile, cTaskID, "tags", NULL);
		pTask->iHour = g_key_file_get_integer (pKeyFile, cTaskID, "hour", NULL);
		pTask->iMinute = g_key_file_get_integer (pKeyFile, cTaskID, "minute", NULL);
		pTask->iFrequency = freq;
		pTask->bAcknowledged = g_key_file_get_boolean (pKeyFile, cTaskID, "ack", NULL);
		
		pTaskList = g_list_prepend (pTaskList, pTask);
		s_iCounter = MAX (s_iCounter, atoi (cTaskID));
	}
	
	g_free (pGroupList);  // les elements sont les IDs et sont integres dans les taches.
	
	if (bUpdateFile)
		cairo_dock_write_keys_to_file (pKeyFile, cFile);
	
	g_key_file_free (pKeyFile);
	g_free (cFile);
	return pTaskList;
}

static gboolean create_task (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	//g_print ("%s (%d/%d/%d)\n", __func__, pTask->iDay, pTask->iMonth, pTask->iYear);
	
	gchar *cFile = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "clock", "tasks.conf");
	GKeyFile *pKeyFile = g_key_file_new ();  // on veut un key_file meme s'il n'existe pas encore.
	g_key_file_load_from_file (pKeyFile, cFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
	
	pTask->cID = g_strdup_printf ("%d", ++s_iCounter);
	
	g_key_file_set_integer (pKeyFile, pTask->cID, "day", pTask->iDay);
	g_key_file_set_integer (pKeyFile, pTask->cID, "month", pTask->iMonth);
	g_key_file_set_integer (pKeyFile, pTask->cID, "year", pTask->iYear);
	g_key_file_set_string (pKeyFile, pTask->cID, "title", pTask->cTitle?pTask->cTitle:"");
	g_key_file_set_string (pKeyFile, pTask->cID, "text", pTask->cText?pTask->cText:"");
	g_key_file_set_string (pKeyFile, pTask->cID, "tags", pTask->cTags?pTask->cTags:"");
	g_key_file_set_integer (pKeyFile, pTask->cID, "hour", pTask->iHour);
	g_key_file_set_integer (pKeyFile, pTask->cID, "minute", pTask->iMinute);
	g_key_file_set_integer (pKeyFile, pTask->cID, "freq", pTask->iFrequency);
	g_key_file_set_boolean (pKeyFile, pTask->cID, "ack", pTask->bAcknowledged);
	
	cairo_dock_write_keys_to_file (pKeyFile, cFile);
	g_free (cFile);
	
	return TRUE;
}

static gboolean delete_task (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	cd_debug ("%s (%s)", __func__, pTask->cTitle);
	
	gchar *cFile = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "clock", "tasks.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cFile);
	g_return_val_if_fail (pKeyFile != NULL, FALSE);
	
	g_key_file_remove_group (pKeyFile, pTask->cID, NULL);
	cairo_dock_write_keys_to_file (pKeyFile, cFile);
	g_free (cFile);
	
	return TRUE;
}

static gboolean update_task (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	cd_debug ("%s (%s, '%s')", __func__, pTask->cTitle, pTask->cText);
	
	gchar *cFile = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "clock", "tasks.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cFile);
	g_return_val_if_fail (pKeyFile != NULL, FALSE);
	
	g_key_file_set_string (pKeyFile, pTask->cID, "title", pTask->cTitle?pTask->cTitle:"");
	g_key_file_set_string (pKeyFile, pTask->cID, "text", pTask->cText?pTask->cText:"");
	g_key_file_set_string (pKeyFile, pTask->cID, "tags", pTask->cTags?pTask->cTags:"");
	g_key_file_set_integer (pKeyFile, pTask->cID, "hour", pTask->iHour);
	g_key_file_set_integer (pKeyFile, pTask->cID, "minute", pTask->iMinute);
	g_key_file_set_integer (pKeyFile, pTask->cID, "freq", pTask->iFrequency);
	g_key_file_set_boolean (pKeyFile, pTask->cID, "ack", pTask->bAcknowledged);
	
	cairo_dock_write_keys_to_file (pKeyFile, cFile);
	g_free (cFile);
	
	return TRUE;
}

void cd_clock_register_backend_default (GldiModuleInstance *myApplet)
{
	CDClockTaskBackend *pBackend = g_new0 (CDClockTaskBackend, 1);
	pBackend->get_tasks = get_tasks;
	pBackend->create_task = create_task;
	pBackend->delete_task = delete_task;
	pBackend->update_task = update_task;
	cd_clock_register_backend (myApplet, "Default", pBackend);
}

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

#include <zeitgeist.h>

#include "applet-struct.h"
#include "applet-search.h"


static void on_related_events_received (ZeitgeistLog  *log, GAsyncResult *res, gpointer *user_data)
{
	ZeitgeistResultSet *events;
	GError *error = NULL;
	CDOnGetEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	events = zeitgeist_log_find_events_finish (log, res, &error);
	if (error)
	{
		cd_warning ("Error reading results: %s", error->message);
		g_error_free (error);
		return;
	}
	g_print ("Got %i events:\n", zeitgeist_result_set_size (events));
	
	if (zeitgeist_result_set_has_next (events))
		pCallback (events, data);
	g_object_unref (events);
}
void cd_find_recent_related_files (const gchar **cMimeTypes, CDOnGetEventsFunc pCallback, gpointer data)  // right-click on a launcher/appli
{
	g_print ("%s ()\n", __func__);
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	GPtrArray* zg_templates = g_ptr_array_sized_new (10);
	int i;
	for (i = 0; cMimeTypes[i] != NULL; i ++)
	{
		ZeitgeistSubject *subj = zeitgeist_subject_new_full ("file:*",  // uri, application://* for apps
			"",  // interpretation
			"",  // manifestation
			cMimeTypes[i],  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		ZeitgeistEvent* ev = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (type ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj, NULL);  // a list of subjects, terminated with NULL
		g_ptr_array_add (zg_templates, ev);
	}
	
	ZeitgeistLog *log = zeitgeist_log_new ();
	
	zeitgeist_log_find_events (log,
		zeitgeist_time_range_new_to_now (),
		zg_templates,
		ZEITGEIST_STORAGE_STATE_ANY,
		20,
		ZEITGEIST_RESULT_TYPE_MOST_RECENT_EVENTS,
		(GCancellable *)NULL,
		(GAsyncReadyCallback)on_related_events_received,
		s_data);
}


static void on_recent_events_received (ZeitgeistLog  *log, GAsyncResult *res, gpointer *user_data)
{
	ZeitgeistResultSet *events;
	GError *error = NULL;
	CDOnGetEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	events = zeitgeist_log_find_events_finish (log, res, &error);
	if (error)
	{
		g_warning ("Error reading results: %s", error->message);
		g_error_free (error);
		return;
	}
	
	cd_message ("Got %i events:", zeitgeist_result_set_size (events));
	
	if (zeitgeist_result_set_has_next (events))
		pCallback (events, data);
	g_object_unref (events);	
}
void cd_find_recent_events (int iSortType, CDOnGetEventsFunc pCallback, gpointer data)  // sorted by date or frequency, click on the icon
{
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	GPtrArray* zg_templates = g_ptr_array_sized_new (1);
	ZeitgeistSubject *subj = zeitgeist_subject_new_full ("file:*",  // uri, application://* for apps
		"",  // interpretation
		"",  // manifestation
		"",  // mimetype
		"",  // origin
		"",  // text
		"");  // storage
	ZeitgeistEvent* ev = zeitgeist_event_new_full (
		ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (type ZEITGEIST_ZG_EVENT_INTERPRETATION)
		ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
		"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
		subj,  // a list of subjects
		NULL);  // terminated with NULL
	g_ptr_array_add (zg_templates, ev);
	
	/// define more templates with different groups ...
	
	
	ZeitgeistLog *log = zeitgeist_log_new ();
	
	zeitgeist_log_find_events (log,
		zeitgeist_time_range_new_to_now (),
		zg_templates,
		ZEITGEIST_STORAGE_STATE_ANY,
		10,
		iSortType == 0 ? ZEITGEIST_RESULT_TYPE_MOST_RECENT_EVENTS : ZEITGEIST_RESULT_TYPE_MOST_POPULAR_SUBJECTS,
		(GCancellable *)NULL,
		(GAsyncReadyCallback)on_recent_events_received,
		s_data);
}


static void on_events_received (ZeitgeistIndex *index, GAsyncResult *res, gpointer *user_data)
{
	ZeitgeistResultSet *events;
	GError *error = NULL;
	CDOnGetEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	events = zeitgeist_index_search_finish (index, res, &error);
	if (error)
	{
		g_warning ("Error reading results: %s", error->message);
		g_error_free (error);
		return;
	}
	
	cd_message ("Got %i events:", zeitgeist_result_set_size (events));
	
	if (zeitgeist_result_set_has_next (events))
		pCallback (events, data);
	g_object_unref (events);
}
void cd_search_events (const gchar *cQuery, CDOnGetEventsFunc pCallback, gpointer data)  // dialog box on middle-click
{
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	ZeitgeistIndex *index = zeitgeist_index_new ();
	
	g_print ("Searching for '%s'...\n", cQuery);
	
	GPtrArray* event_templates = g_ptr_array_new ();
	zeitgeist_index_search (index,
		cQuery,
		zeitgeist_time_range_new_anytime (),
		event_templates,
		0,  // offset
		20,  // number of events
		ZEITGEIST_RESULT_TYPE_RELEVANCY,  // sorting type
		(GCancellable*)NULL,
		(GAsyncReadyCallback)on_events_received,
		s_data);  // data
}


static void on_delete_events (ZeitgeistLog *log, GAsyncResult *res, gpointer *user_data)
{
	CDOnDeleteEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	GError *error = NULL;
	gboolean bSuccess = zeitgeist_log_delete_events_finish (log, res, &error);
	if (error)
	{
		cd_warning ("Error deleting log: %s", error->message);
		g_error_free (error);
	}
}
static void on_deleting_event_received (ZeitgeistLog *log, GAsyncResult *res, gpointer user_data)
{
	GError *error = NULL;
	GArray *event_ids = zeitgeist_log_find_event_ids_finish (log, res, &error);
	if (error)
	{
		cd_warning ("Error finding in log: %s", error->message);
		g_error_free (error);
		return;
	}
	// delete events IDs
	zeitgeist_log_delete_events (log,
		event_ids,
		(GCancellable *)NULL,
		(GAsyncReadyCallback)on_delete_events,
		user_data);
}
static void on_delete_whole_log (ZeitgeistLog *log, GAsyncResult *res, gpointer *user_data)
{
	CDOnDeleteEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	GError *error = NULL;
	gboolean bSuccess = zeitgeist_log_delete_log_finish (log, res, &error);
	if (error)
	{
		cd_warning ("Error deleting log: %s", error->message);
		g_error_free (error);
	}
}
void cd_get_delete_recent_events (int iNbDays, CDOnDeleteEventsFunc pCallback, gpointer data)  // entry in the menu
{
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	ZeitgeistLog *log = zeitgeist_log_new ();
	
	if (iNbDays > 0)
	{
		// find events IDs of less than 'iNbDays' days
		GArray *event_ids;
		time_t now = (time_t) time (NULL);
		ZeitgeistTimeRange *time_range = zeitgeist_time_range_new (now - 7*24*3600, now);
		
		GPtrArray* event_templates = g_ptr_array_new ();
		
		zeitgeist_log_find_event_ids (log,
			time_range,
			event_templates,
			ZEITGEIST_STORAGE_STATE_ANY,
			1000,
			ZEITGEIST_RESULT_TYPE_MOST_RECENT_EVENTS,
			(GCancellable *)NULL,
			(GAsyncReadyCallback)on_deleting_event_received,
			s_data);
	}
	else  // delete the whole log
	{
		zeitgeist_log_delete_log (log,
			(GCancellable *)NULL,
			(GAsyncReadyCallback)on_delete_whole_log,
			s_data);
	}
	
}

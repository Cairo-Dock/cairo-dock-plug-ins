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


static ZeitgeistEvent *_get_event_template_for_category (CDEventType iCategory)
{
	if (myData.pEvents == NULL)
	{
		myData.pEvents = g_new0 (ZeitgeistEvent*, CD_NB_EVENT_TYPES);
		
		ZeitgeistSubject *subj;
		subj = zeitgeist_subject_new_full ("",  // any type of uri.
			"",  // interpretation
			"",  // manifestation (ZEITGEIST_NFO_FILE_DATA_OBJECT/ZEITGEIST_NFO_REMOTE_DATA_OBJECT)
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_ALL] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			ZEITGEIST_NFO_DOCUMENT,  // interpretation
			ZEITGEIST_NFO_FILE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_DOCUMENT] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		/**subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			ZEITGEIST_NFO_FOLDER,  // interpretation
			ZEITGEIST_NFO_FILE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_FOLDER] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL*/
		
		subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			ZEITGEIST_NFO_IMAGE,  // interpretation
			ZEITGEIST_NFO_FILE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_IMAGE] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			ZEITGEIST_NFO_AUDIO,  // interpretation
			ZEITGEIST_NFO_FILE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_AUDIO] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			ZEITGEIST_NFO_VIDEO,  // interpretation
			ZEITGEIST_NFO_FILE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_VIDEO] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		subj = zeitgeist_subject_new_full ("",  // url
			ZEITGEIST_NFO_WEBSITE,  // interpretation
			ZEITGEIST_NFO_REMOTE_DATA_OBJECT,  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_WEB] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		
		subj = zeitgeist_subject_new_full ("file://*",  // uri, application://* for apps
			"!"ZEITGEIST_NFO_DOCUMENT,  // interpretation
			"",  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		myData.pEvents[CD_EVENT_OTHER] = zeitgeist_event_new_full (
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj,  // a list of subjects
			NULL);  // terminated with NULL
		/**subj = zeitgeist_subject_new_full ("",  // uri, application://* for apps
			"!"ZEITGEIST_NFO_FOLDER,  // interpretation
			"",  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		zeitgeist_event_add_subject (myData.pEvents[CD_EVENT_OTHER], subj);*/
		subj = zeitgeist_subject_new_full ("",  // uri, application://* for apps
			"!"ZEITGEIST_NFO_IMAGE,  // interpretation
			"",  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		zeitgeist_event_add_subject (myData.pEvents[CD_EVENT_OTHER], subj);
		subj = zeitgeist_subject_new_full ("",  // uri, application://* for apps
			"!"ZEITGEIST_NFO_AUDIO,  // interpretation
			"",  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		zeitgeist_event_add_subject (myData.pEvents[CD_EVENT_OTHER], subj);
		subj = zeitgeist_subject_new_full ("",  // uri, application://* for apps
			"!"ZEITGEIST_NFO_VIDEO,  // interpretation
			"",  // manifestation
			"",  // mimetype
			"",  // origin
			"",  // text
			"");  // storage
		zeitgeist_event_add_subject (myData.pEvents[CD_EVENT_OTHER], subj);
	}
	
	g_return_val_if_fail (iCategory < CD_NB_EVENT_TYPES, NULL);
	g_object_ref (myData.pEvents[iCategory]);
	return myData.pEvents[iCategory];
}


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
			ZEITGEIST_ZG_ACCESS_EVENT,  // interpretation type of the event (ZEITGEIST_ZG_EVENT_INTERPRETATION)
			ZEITGEIST_ZG_USER_ACTIVITY,  // manifestation type of the event (ZEITGEIST_ZG_EVENT_MANIFESTATION)
			"",  // actor (the party responsible for triggering the event, eg: app://firefox.desktop)
			subj, NULL);  // a list of subjects, terminated with NULL
		g_ptr_array_add (zg_templates, ev);
	}
	
	if (myData.pLog == NULL)
		myData.pLog = zeitgeist_log_new ();
	
	zeitgeist_log_find_events (myData.pLog,
		zeitgeist_time_range_new_to_now (),
		zg_templates,
		ZEITGEIST_STORAGE_STATE_ANY,
		myConfig.iNbRelatedFilesMax,
		ZEITGEIST_RESULT_TYPE_MOST_RECENT_EVENTS,  // MOST_RECENT_ORIGIN for folders
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
	
	g_print ("Got %i events:\n", zeitgeist_result_set_size (events));
	
	if (zeitgeist_result_set_has_next (events))
		pCallback (events, data);
	g_object_unref (events);	
}
void cd_find_recent_events (CDEventType iEventType, int iSortType, CDOnGetEventsFunc pCallback, gpointer data)  // sorted by date or frequency, click on the icon
{
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	ZeitgeistEvent *ev;
	ZeitgeistSubject *subj;
	ev = _get_event_template_for_category (iEventType);
	
	GPtrArray* zg_templates = g_ptr_array_sized_new (1);
	g_ptr_array_add (zg_templates, ev);
	
	if (myData.pLog == NULL)
		myData.pLog = zeitgeist_log_new ();
	
	zeitgeist_log_find_events (myData.pLog,
		zeitgeist_time_range_new_to_now (),
		zg_templates,
		ZEITGEIST_STORAGE_STATE_ANY,
		myConfig.iNbResultsMax,
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
void cd_search_events (const gchar *cQuery, CDEventType iEventType, CDOnGetEventsFunc pCallback, gpointer data)  // dialog box on middle-click
{
	static gpointer s_data[2];
	s_data[0] = pCallback;
	s_data[1] = data;
	
	if (myData.pIndex == NULL)
		myData.pIndex = zeitgeist_index_new ();
	
	g_print ("Searching for '%s'...\n", cQuery);
	
	GPtrArray* zg_templates = g_ptr_array_sized_new (1);
	ZeitgeistEvent *ev = _get_event_template_for_category (iEventType);
	g_ptr_array_add (zg_templates, ev);
	zeitgeist_index_search (myData.pIndex,
		cQuery,
		zeitgeist_time_range_new_anytime (),
		zg_templates,
		0,  // offset
		myConfig.iNbResultsMax,  // number of events
		ZEITGEIST_RESULT_TYPE_RELEVANCY,  // sorting type
		(GCancellable*)NULL,
		(GAsyncReadyCallback)on_events_received,
		s_data);  // data
}


static void on_delete_events (ZeitgeistLog *log, GAsyncResult *res, gpointer *user_data)
{
	g_print ("events deleted\n");
	CDOnDeleteEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	int iNbEvents = GPOINTER_TO_INT (user_data[2]);
	
	GError *error = NULL;
	gboolean bSuccess = zeitgeist_log_delete_events_finish (log, res, &error);
	if (error)
	{
		cd_warning ("Error deleting log: %s", error->message);
		g_error_free (error);
		pCallback (0, data);
	}
	else
	{
		pCallback (iNbEvents, data);
	}
}
static void on_deleting_event_received (ZeitgeistLog *log, GAsyncResult *res, gpointer *user_data)
{
	CDOnDeleteEventsFunc pCallback = user_data[0];
	gpointer data = user_data[1];
	
	GError *error = NULL;
	GArray *event_ids = zeitgeist_log_find_event_ids_finish (log, res, &error);
	g_print ("got %d events\n", event_ids->len);
	if (error)
	{
		cd_warning ("Error finding in log: %s", error->message);
		g_error_free (error);
		pCallback (0, data);
		return;
	}
	// delete events IDs
	user_data[2] = GINT_TO_POINTER (event_ids->len);
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
		pCallback (0, data);
	}
	else
	{
		pCallback (-1, data);
	}
}
void cd_delete_recent_events (int iNbDays, CDOnDeleteEventsFunc pCallback, gpointer data)  // entry in the menu
{
	static gpointer s_data[3];
	s_data[0] = pCallback;
	s_data[1] = data;
	s_data[2] = 0;
	
	if (myData.pLog == NULL)
		myData.pLog = zeitgeist_log_new ();
	
	if (iNbDays > 0)
	{
		// find events IDs of less than 'iNbDays' days
		GArray *event_ids;
		time_t t = (time_t) time (NULL);
		gint64 now = t * 1e3;  // msec
		ZeitgeistTimeRange *time_range = zeitgeist_time_range_new (now - iNbDays*24*3600*1e3, now);
		
		GPtrArray* event_templates = g_ptr_array_new ();
		
		zeitgeist_log_find_event_ids (myData.pLog,
			time_range,
			event_templates,
			ZEITGEIST_STORAGE_STATE_ANY,
			999,  // 999 is the max number of events available in zeitgeist 0.2, and 1499 is the max in sqlite3 :-/
			ZEITGEIST_RESULT_TYPE_MOST_RECENT_EVENTS,
			(GCancellable *)NULL,
			(GAsyncReadyCallback)on_deleting_event_received,
			s_data);
	}
	else  // delete the whole log
	{
		zeitgeist_log_delete_log (myData.pLog,
			(GCancellable *)NULL,
			(GAsyncReadyCallback)on_delete_whole_log,
			s_data);
	}
}


void cd_delete_event (guint32 id, CDOnDeleteEventsFunc pCallback, gpointer data)
{
	static gpointer s_data[3];
	s_data[0] = pCallback;
	s_data[1] = data;
	s_data[2] = GINT_TO_POINTER (1);
	
	GArray *event_ids = g_array_sized_new (TRUE,
		TRUE,
		sizeof (guint32),
		1);
	g_array_append_val (event_ids, id);
	
	zeitgeist_log_delete_events (myData.pLog,
		event_ids,
		(GCancellable *)NULL,
		(GAsyncReadyCallback)on_delete_events,
		s_data);  // this function unrefs the array
}

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
#include <dbus/dbus-glib.h>
#include <time.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "applet-notes.h"
#include "applet-backend-tomboy.h"

static DBusGProxy *dbus_proxy_tomboy = NULL;

extern struct tm *localtime_r (const time_t *timer, struct tm *tp);

#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)

  ////////////////////
 /// DBUS METHODS ///
////////////////////

static gchar *getNoteContents (const gchar *cNoteID)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, NULL);
	gchar *cNoteContent = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteContents", NULL,
		G_TYPE_STRING, cNoteID,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cNoteContent,
		G_TYPE_INVALID);
	return cNoteContent;
}

/*
static gchar **get_note_names_with_tag (const gchar *cTag)
{
	gchar **cNoteNames = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetAllNotesWithTag", NULL,
		G_TYPE_STRING, cTag,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cNoteNames,
		G_TYPE_INVALID);
	return cNoteNames;
}*/

static gchar *getNoteTitle (const gchar *cNoteURI)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, NULL);
	gchar *note_title = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteTitle", NULL,
		G_TYPE_STRING, cNoteURI,
		G_TYPE_INVALID,
		G_TYPE_STRING, &note_title,
		G_TYPE_INVALID);
	
	return note_title;
}

static int getNoteCreateDate (const gchar *cNoteURI)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, 0);
	long int iDate = 0;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteCreateDate", NULL,
		G_TYPE_STRING, cNoteURI,
		G_TYPE_INVALID,
		G_TYPE_LONG, &iDate,
		G_TYPE_INVALID);
	
	return iDate;
}

static int getNoteChangeDate (const gchar *cNoteURI)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, 0);
	long int iDate = 0;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteChangeDate", NULL,
		G_TYPE_STRING, cNoteURI,
		G_TYPE_INVALID,
		G_TYPE_LONG, &iDate,
		G_TYPE_INVALID);
	
	return iDate;
}

/* Not used
static gchar **getNoteTags (const gchar *note_name)
{
	gchar **cTags = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetTagsForNote", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cTags,
		G_TYPE_INVALID);
	return cTags;
}
*/


  ////////////////////
 /// DBUS SIGNALS ///
////////////////////

static void onNoteDeleted (DBusGProxy *proxy, const gchar *note_uri, const gchar *note_title, gpointer data)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, note_uri);
	
	cd_notes_store_remove_note (note_uri);
	CD_APPLET_LEAVE ();
}

static void onNoteAdded (DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, note_uri);
	
	CDNote *pNote = g_new0 (CDNote, 1);
	pNote->cID = g_strdup (note_uri);
	pNote->cTitle = getNoteTitle (note_uri);
	pNote->iCreationDate = getNoteCreateDate (note_uri);
	pNote->iLastChangeDate = pNote->iCreationDate;
	pNote->cContent = NULL;
	
	cd_notes_store_add_note (pNote);
	CD_APPLET_LEAVE ();
}

static void onNoteSaved (DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, note_uri);
	
	CDNote *pNote = g_new0 (CDNote, 1);
	pNote->cID = g_strdup (note_uri);
	pNote->cTitle = getNoteTitle (note_uri);
	pNote->iCreationDate = 0;
	pNote->iLastChangeDate = getNoteChangeDate (note_uri);
	pNote->cContent = getNoteContents (note_uri);
	
	cd_notes_store_update_note (pNote);
	CD_APPLET_LEAVE ();
}


  ////////////////////
 /// DBUS SERVICE ///
////////////////////

static void g_cclosure_marshal_VOID__STRING_STRING (GClosure *closure, GValue *return_value G_GNUC_UNUSED, guint n_param_values, const GValue *param_values, gpointer invocation_hint G_GNUC_UNUSED, gpointer marshal_data)
{
	cd_debug ("marshaller");
	typedef void (*GMarshalFunc_VOID__STRING_STRING) (gpointer data1,
		gchar*   arg_1,
		gchar*   arg_2,
		gpointer data2);
	register GMarshalFunc_VOID__STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;

	g_return_if_fail (n_param_values == 3);

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_string (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);

}
void _tomboy_connect_to_service (void)
{
	cd_debug ("");
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_STRING,
	G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);

	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
		default:
			dbus_proxy_tomboy = cairo_dock_create_new_dbus_proxy (
				"org.gnome.Tomboy",
				"/org/gnome/Tomboy/RemoteControl",
				"org.gnome.Tomboy.RemoteControl");
		break;
		case CD_NOTES_GNOTES:
			dbus_proxy_tomboy = cairo_dock_create_new_dbus_proxy (
				"org.gnome.Gnote",
				"/org/gnome/Gnote/RemoteControl",
				"org.gnome.Gnote.RemoteControl");
		break;
	}
	g_return_if_fail (dbus_proxy_tomboy != NULL);

	dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_INVALID);
	dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteAdded",
		G_TYPE_STRING,
		G_TYPE_INVALID);
	dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteSaved",
		G_TYPE_STRING,
		G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_CALLBACK(onNoteDeleted), NULL, NULL);
	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteAdded",
		G_CALLBACK(onNoteAdded), NULL, NULL);
	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteSaved",
		G_CALLBACK(onNoteSaved), NULL, NULL);
}

void _tomboy_disconnect_from_service (void)
{
	cd_debug ("");
	// cancel any pending operations
	if (myData.pDetectTomboyCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pDetectTomboyCall);
		myData.pDetectTomboyCall = NULL;
	}
	if (myData.pGetNotesCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pGetNotesCall);
		myData.pGetNotesCall = NULL;
	}
	
	// disconnect from the service on the bus
	if (dbus_proxy_tomboy != NULL)
	{
		dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteDeleted",
			G_CALLBACK(onNoteDeleted), NULL);
		dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteAdded",
			G_CALLBACK(onNoteAdded), NULL);
		dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteSaved",
			G_CALLBACK(onNoteSaved), NULL);

		g_object_unref (dbus_proxy_tomboy);
		dbus_proxy_tomboy = NULL;
	}
}



  ///////////////
 /// BACKEND ///
///////////////

typedef struct {
	gchar **pNotesURI;
	GList *pNotes;
	} CDSharedMemory;
static void _get_notes_data_async (CDSharedMemory *pSharedMemory)
{
	gchar **note_list = pSharedMemory->pNotesURI;
	GList *pNotes = NULL;
	gchar *cNoteURI;
	int i;
	for (i = 0; note_list[i] != NULL; i ++)
	{
		cNoteURI = note_list[i];
		CDNote *pNote = g_new0 (CDNote, 1);
		pNote->cID = cNoteURI;
		pNote->cTitle = getNoteTitle (cNoteURI);
		pNote->iCreationDate = getNoteCreateDate (cNoteURI);
		pNote->iLastChangeDate = getNoteChangeDate (cNoteURI);
		pNote->cContent = getNoteContents (cNoteURI);

		pNotes = g_list_append (pNotes, pNote);
	}
	
	pNotes = g_list_reverse (pNotes);
	
	g_free (pSharedMemory->pNotesURI);  // elements are inside the list
	pSharedMemory->pNotesURI = NULL;
	pSharedMemory->pNotes = pNotes;
}
static gboolean _build_notes_from_data (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	cd_notes_store_load_notes (pSharedMemory->pNotes);
	
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	CD_APPLET_LEAVE (FALSE);
}
static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_list_foreach (pSharedMemory->pNotes, (GFunc)cd_notes_free_note, NULL);
	g_list_free (pSharedMemory->pNotes);
	if (pSharedMemory->pNotesURI)
		g_strfreev (pSharedMemory->pNotesURI);
	g_free (pSharedMemory);
}

static void _on_get_all_notes (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pGetNotesCall = NULL;
	gchar **note_list = NULL;
	if (dbus_g_proxy_end_call (proxy,
		call_id,
		NULL,
		G_TYPE_STRV,
		&note_list,
		G_TYPE_INVALID))
	{
		cd_message ("got notes list, now get notes content...");
		
		CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
		pSharedMemory->pNotesURI = note_list;
		myData.pTask = cairo_dock_new_task_full (0,  // 1 shot task.
			(CairoDockGetDataAsyncFunc) _get_notes_data_async,
			(CairoDockUpdateSyncFunc) _build_notes_from_data,
			(GFreeFunc) _free_shared_memory,
			pSharedMemory);

		cairo_dock_launch_task (myData.pTask);
	}
	else
	{
		cd_warning ("Couldn't get the notes on the bus.");
		if (myData.iIconState != 1)
		{
			myData.iIconState = 1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "close.svg");
		}
	}
	
	CD_APPLET_LEAVE ();
}
static void getAllNotes_async (void)
{
	if (myData.pGetNotesCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pGetNotesCall);
	}
	myData.pGetNotesCall = dbus_g_proxy_begin_call (dbus_proxy_tomboy, "ListAllNotes",
		(DBusGProxyCallNotify)_on_get_all_notes,
		NULL,  // data
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

static void _on_watcher_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	cd_debug ("=== %s is on the bus (%d)", cName, bOwned);
	CD_APPLET_ENTER;
	
	if (bOwned)
	{
		// connect to the service
		_tomboy_connect_to_service ();
		
		// get the notes asynchronously
		getAllNotes_async ();
		
		myData.bIsRunning = TRUE;
		if (myData.iIconState != 0)
		{
			myData.iIconState = 0;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconDefault, "icon.svg");
		}
	}
	else  // no more service on the bus.
	{
		_tomboy_disconnect_from_service ();
		
		myData.bIsRunning = FALSE;
		if (myData.iIconState != 1)
		{
			myData.iIconState = 1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "close.svg");
		}
	}
	CD_APPLET_LEAVE ();
}
static void _on_detect_tomboy (gboolean bPresent, const gchar *cService)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%s: %d)\n", __func__, cService, bPresent);
	myData.pDetectTomboyCall = NULL;
	myData.bIsRunning = bPresent;
	
	if (bPresent)  // the service is present on the bus -> connect to it and get the notes
	{
		// connect and get the notes.
		_on_watcher_owner_changed (cService, TRUE, NULL);
	}
	else  // no service present -> set the 'closed' image
	{
		if (myData.iIconState != 1)
		{
			myData.iIconState = 1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "close.svg");
		}
	}
	// watch for any futur changes
	cairo_dock_watch_dbus_name_owner (cService,
		(CairoDockDbusNameOwnerChangedFunc) _on_watcher_owner_changed,
		NULL);
	CD_APPLET_LEAVE ();
}
static void start (void)
{
	// detect the service on the bus asynchronously
	g_return_if_fail (myData.pDetectTomboyCall == NULL);
	myData.bIsRunning = FALSE;
	const gchar *cService = "";
	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
			cService = "org.gnome.Tomboy";
		break;
		case CD_NOTES_GNOTES:
			cService = "org.gnome.Gnote";
		break;
		default:
		return;
	}
	myData.pDetectTomboyCall = cairo_dock_dbus_detect_application_async (cService, (CairoDockOnAppliPresentOnDbus) _on_detect_tomboy, (gpointer)cService);  // 'cService' is a constant string, no need to allocate it
}

static void stop (void)
{
	cd_message ("");
	_tomboy_disconnect_from_service ();
}

static void show_note (const gchar *cNoteID)
{
	g_return_if_fail (dbus_proxy_tomboy != NULL);
	dbus_g_proxy_call (dbus_proxy_tomboy, "DisplayNote", NULL,
		G_TYPE_STRING, cNoteID,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}

static void delete_note (const gchar *cNoteID)
{
	g_return_if_fail (dbus_proxy_tomboy != NULL);
	gboolean bDelete = TRUE;
	dbus_g_proxy_call (dbus_proxy_tomboy, "DeleteNote", NULL,
		G_TYPE_STRING, cNoteID,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &bDelete,
		G_TYPE_INVALID);
}

static gchar *create_note (const gchar *cTitle)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, NULL);
	gchar *cNoteID = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "CreateNamedNote", NULL,
		G_TYPE_STRING, cTitle,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cNoteID,
		G_TYPE_INVALID);
	
	return (cNoteID);
}


static void dbus_detect_tomboy (void)
{
	cd_message ("");
	const gchar *cService = "";
	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
		default:
			cService = "org.gnome.Tomboy";
		break;
		case CD_NOTES_GNOTES:
			cService = "org.gnome.Gnote";
		break;
	}
	myData.bIsRunning = cairo_dock_dbus_detect_application (cService);
}
static void run_manager (void)
{
	// check it's really not running
	dbus_detect_tomboy();  // -> updates 'myData.bIsRunning'
	
	// launch it
	if (! myData.bIsRunning)
	{
		const gchar *cName = "";
		const gchar *cCommand = "";
		switch (myConfig.iAppControlled)
		{
			case CD_NOTES_TOMBOY:
			default:
				cName = "Tomboy";
			cCommand = "tomboy &";
			break;
			case CD_NOTES_GNOTES:
				cName = "Gnote";
				cCommand = "gnote &";
			break;
		}
		cairo_dock_show_temporary_dialog_with_icon_printf ("Launching %s...",
			myIcon, myContainer,
			2000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			cName);
		cairo_dock_launch_command (cCommand);
	}
}

void cd_notes_register_tomboy_backend (void)
{
	myData.backend.start = start;
	myData.backend.stop = stop;
	myData.backend.show_note = show_note;
	myData.backend.delete_note = delete_note;
	myData.backend.create_note = create_note;
	myData.backend.run_manager = run_manager;
}

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
#include <time.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "applet-notes.h"
#include "applet-backend-tomboy.h"


static GDBusProxy *dbus_proxy_tomboy = NULL;
static guint s_uWatch = 0;
static GCancellable *s_pCancel = NULL;



  ////////////////////
 /// DBUS SIGNALS ///
////////////////////

typedef struct _CDNoteData {
	CDNote note;
	gboolean bIsUpdate;
} CDNoteData;

static void _got_content_cb (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	CDNoteData *pData = (CDNoteData*)data;
	
	GDBusProxy *pProxy = G_DBUS_PROXY (pObj);
	GError *erreur = NULL;
	GVariant *var = g_dbus_proxy_call_finish (pProxy, pRes, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting note properties: %s", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		if (g_variant_is_of_type (var, G_VARIANT_TYPE ("(s)")))
		{
			g_variant_get (var, "(s)", &pData->note.cContent);
			cd_notes_store_update_note ((CDNote*)pData);
		}
		else cd_warning ("Unexpected result type for note contents: %s", g_variant_get_type_string (var));
		g_variant_unref (var);
	}
	
	g_free (pData->note.cTitle);
	g_free (pData->note.cID);
	g_free (pData);
	
	CD_APPLET_LEAVE ();
}

static void _got_date_cb (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	CDNoteData *pData = (CDNoteData*)data;
	
	GDBusProxy *pProxy = G_DBUS_PROXY (pObj);
	GError *erreur = NULL;
	GVariant *var = g_dbus_proxy_call_finish (pProxy, pRes, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting note properties: %s", erreur->message);
		g_error_free (erreur);
		g_free (pData->note.cTitle);
		g_free (pData->note.cID);
		g_free (pData);
	}
	else
	{
		if (g_variant_is_of_type (var, G_VARIANT_TYPE ("(i)")))
		{
			g_variant_get (var, "(i)", &pData->note.iLastChangeDate);
			if (pData->bIsUpdate) pData->note.iCreationDate = 0;
			else pData->note.iLastChangeDate = pData->note.iCreationDate;
			
			if (pData->bIsUpdate)
			{
				// need to get the contents as well
				g_dbus_proxy_call (pProxy, "GetNoteContents",
					g_variant_new ("(s)", pData->note.cID), G_DBUS_CALL_FLAGS_NONE, -1, s_pCancel,
					_got_content_cb, pData);
			}
			else
			{
				// we have everything, can create the new note
				cd_notes_store_add_note ((CDNote*)pData);
				g_free (pData);
			}
		}
		else
		{
			cd_warning ("Unexpected result type for note update date: %s", g_variant_get_type_string (var));
			g_free (pData->note.cTitle);
			g_free (pData->note.cID);
			g_free (pData); // Cannot continue if cannot get data
		}
		
		g_variant_unref (var);
	}
	
	CD_APPLET_LEAVE ();
}

static void _got_title_cb (GObject *pObj, GAsyncResult *pRes, gpointer data)
{
	CD_APPLET_ENTER;
	CDNoteData *pData = (CDNoteData*)data;
	
	GDBusProxy *pProxy = G_DBUS_PROXY (pObj);
	GError *erreur = NULL;
	GVariant *var = g_dbus_proxy_call_finish (pProxy, pRes, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting note properties: %s", erreur->message);
		g_error_free (erreur);
		g_free (pData->note.cID);
		g_free (pData);
	}
	else
	{
		if (g_variant_is_of_type (var, G_VARIANT_TYPE ("(s)")))
		{
			g_variant_get (var, "(s)", &pData->note.cTitle);
			g_dbus_proxy_call (pProxy, pData->bIsUpdate ? "GetNoteChangeDate" : "GetNoteCreateDate",
				g_variant_new ("(s)", pData->note.cID), G_DBUS_CALL_FLAGS_NONE, -1, s_pCancel,
				_got_date_cb, pData);
		}
		else
		{
			cd_warning ("Unexpected result type for note title: %s", g_variant_get_type_string (var));
			g_free (pData->note.cID);
			g_free (pData); // Cannot continue if cannot get data
		}
		
		g_variant_unref (var);
	}
	
	CD_APPLET_LEAVE ();
}

static void _signal_cb (GDBusProxy *pProxy, char *cSender, char *cSignal, GVariant *pPar, G_GNUC_UNUSED gpointer user_data)
{
	CD_APPLET_ENTER;
	
	CDNoteData *pData = NULL;
	const char *cNoteURI = NULL;
	
	if (! strcmp (cSignal, "NoteDeleted"))
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(s)")))
			g_variant_get (pPar, "(&s)", &cNoteURI);
		else if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(ss)")))
			g_variant_get (pPar, "(&ss)", &cNoteURI, NULL);
		else cd_warning ("Unexpected signal parameter type: %s", g_variant_get_type_string (pPar));
		
		if (cNoteURI) cd_notes_store_remove_note (cNoteURI);
		CD_APPLET_LEAVE ();
	}
	
	if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(s)")))
		g_variant_get (pPar, "(&s)", &cNoteURI);
	else
	{
		cd_warning ("Unexpected signal parameter type: %s", g_variant_get_type_string (pPar));
		CD_APPLET_LEAVE ();
	}
	
	if (! strcmp (cSignal, "NoteAdded"))
	{
		pData = g_new0 (CDNoteData, 1);
	}
	else if (! strcmp (cSignal, "NoteSaved"))
	{
		pData = g_new0 (CDNoteData, 1);
		pData->bIsUpdate = TRUE;
	}
	else cd_warning ("Unknown signal: %s", cSignal);
	
	if (pData) // this is a new or updated note, we need to get its properties
	{
		pData->note.cID = g_strdup (cNoteURI);
		g_dbus_proxy_call (pProxy, "GetNoteTitle", g_variant_new ("(s)", cNoteURI),
			G_DBUS_CALL_FLAGS_NONE, -1, s_pCancel, _got_title_cb, pData);
	}
	
	CD_APPLET_LEAVE ();
}

  ///////////////
 /// BACKEND ///
///////////////

static gboolean _get_string_prop (GDBusProxy* proxy, const char *cMethod, const char *cURI, char **res, GCancellable *pCancel)
{
	GError *erreur = NULL;
	GVariant *var = g_dbus_proxy_call_sync (proxy, cMethod, g_variant_new ("(s)", cURI),
		G_DBUS_CALL_FLAGS_NONE, -1, pCancel, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting note properties: %s", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	
	// return value should be "(s)"
	gboolean ret = FALSE;
	if (g_variant_is_of_type (var, G_VARIANT_TYPE ("(s)")))
	{
		g_variant_get (var, "(s)", res);
		ret = TRUE;
	}
	else cd_warning ("Unexpected note property type: %s", g_variant_get_type_string (var));
	g_variant_unref (var);
	return ret;
}

static gboolean _get_int_prop (GDBusProxy* proxy, const char *cMethod, const char *cURI, guint *iRes, GCancellable *pCancel)
{
	GError *erreur = NULL;
	GVariant *var = g_dbus_proxy_call_sync (proxy, cMethod, g_variant_new ("(s)", cURI),
		G_DBUS_CALL_FLAGS_NONE, -1, pCancel, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting note properties: %s", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	
	// return value should be "(i)"
	gboolean ret = FALSE;
	if (g_variant_is_of_type (var, G_VARIANT_TYPE ("(i)")))
	{
		int x;
		g_variant_get (var, "(i)", &x);
		*iRes = (unsigned int)x;
		ret = TRUE;
	}
	else cd_warning ("Unexpected note property type: %s", g_variant_get_type_string (var));
	g_variant_unref (var);
	return ret;
}


GldiTask *s_pTask = NULL;
typedef struct {
	gchar **pNotesURI;
	GList *pNotes;
	GCancellable *pCancel; // ref to s_pCancel
	GDBusProxy *pProxy; // ref to main proxy
	GldiTask *pTask;
	} CDSharedMemory;
static void _get_notes_data_async (CDSharedMemory *pSharedMemory)
{
	gchar **note_list = pSharedMemory->pNotesURI;
	GDBusProxy *pProxy = pSharedMemory->pProxy;
	GCancellable *pCancel = pSharedMemory->pCancel;
	GList *pNotes = NULL;
	gboolean bError = FALSE;
	gchar *cNoteURI;
	int i;
	for (i = 0; note_list[i] != NULL; i ++)
	{
		cNoteURI = note_list[i];
		CDNote *pNote = g_new0 (CDNote, 1);
		pNote->cID = cNoteURI;
		if (_get_string_prop (pProxy, "GetNoteTitle", cNoteURI, &pNote->cTitle, pCancel) &&
			_get_string_prop (pProxy, "GetNoteContents", cNoteURI, &pNote->cContent, pCancel) &&
			_get_int_prop (pProxy, "GetNoteCreateDate", cNoteURI, &pNote->iCreationDate, pCancel) &&
			_get_int_prop (pProxy, "GetNoteChangeDate", cNoteURI, &pNote->iLastChangeDate, pCancel) &&
			!g_cancellable_is_cancelled (pCancel))
		{
			pNotes = g_list_prepend (pNotes, pNote);
		}
		else
		{
			cd_notes_free_note (pNote);
			bError = TRUE;
			break;
		}
	}
	
	pNotes = g_list_reverse (pNotes);
	
	g_free (pSharedMemory->pNotesURI);  // elements are inside the list
	pSharedMemory->pNotesURI = NULL;
	pSharedMemory->pNotes = pNotes;
	
	if (bError) gldi_task_discard (pSharedMemory->pTask); // will avoid the update below
}
static gboolean _build_notes_from_data (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	cd_notes_store_load_notes (pSharedMemory->pNotes);
	
	gldi_task_discard (s_pTask);
	s_pTask = NULL;
	CD_APPLET_LEAVE (FALSE);
}
static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_list_foreach (pSharedMemory->pNotes, (GFunc)cd_notes_free_note, NULL);
	g_list_free (pSharedMemory->pNotes);
	if (pSharedMemory->pNotesURI)
		g_strfreev (pSharedMemory->pNotesURI);
	g_object_unref (pSharedMemory->pCancel);
	g_object_unref (pSharedMemory->pProxy);
	g_free (pSharedMemory);
}

static void _on_get_all_notes (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	// myData.pGetNotesCall = NULL;
	gchar **note_list = NULL;
	GError *erreur = NULL;
	
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Error getting the list of notes: %s", erreur->message);
			if (myData.iIconState != 1)
			{
				myData.iIconState = 1;
				CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "close.svg");
			}
		}
		g_error_free (erreur);
		erreur = NULL;
	}
	else
	{
		g_variant_get (res, "(^as)", &note_list);
		CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
		pSharedMemory->pNotesURI = note_list;
		pSharedMemory->pCancel = g_object_ref (s_pCancel);
		pSharedMemory->pProxy = G_DBUS_PROXY (g_object_ref (pObj));
		pSharedMemory->pTask = gldi_task_new_full (0,  // 1 shot task.
			(GldiGetDataAsyncFunc) _get_notes_data_async,
			(GldiUpdateSyncFunc) _build_notes_from_data,
			(GFreeFunc) _free_shared_memory,
			pSharedMemory);
		s_pTask = pSharedMemory->pTask;

		gldi_task_launch (s_pTask);
	}
	
	CD_APPLET_LEAVE ();
}
static void getAllNotes_async (void)
{
	g_dbus_proxy_call (dbus_proxy_tomboy, "ListAllNotes",
		NULL, G_DBUS_CALL_FLAGS_NONE, -1,
		s_pCancel, _on_get_all_notes, NULL);
}



static void _proxy_connected (G_GNUC_UNUSED GObject *obj, GAsyncResult *res, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	
	dbus_proxy_tomboy = g_dbus_proxy_new_finish (res, &erreur);
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error connecting to GNote: %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	else
	{
		g_signal_connect (dbus_proxy_tomboy, "g-signal", G_CALLBACK (_signal_cb), NULL);
		
		// get the notes asynchronously
		getAllNotes_async ();
		
		myData.bIsRunning = TRUE;
		if (myData.iIconState != 0)
		{
			myData.iIconState = 0;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconDefault, "icon.svg");
		}
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_name_appeared (GDBusConnection *connection, G_GNUC_UNUSED const gchar *name,
	G_GNUC_UNUSED const gchar *name_owner, G_GNUC_UNUSED gpointer data)
{
	g_return_if_fail (dbus_proxy_tomboy == NULL && s_pCancel == NULL);
	
	CD_APPLET_ENTER;
	
	s_pCancel = g_cancellable_new ();
	g_dbus_proxy_new (connection,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // GDBusInterfaceInfo -- we could supply this, but works anyway
		"org.gnome.Gnote",
		"/org/gnome/Gnote/RemoteControl",
		"org.gnome.Gnote.RemoteControl",
		s_pCancel, // GCancellable
		_proxy_connected,
		NULL);
	
	CD_APPLET_LEAVE ();
}


static void _tomboy_disconnect_from_service (void)
{
	if (s_pCancel)
	{
		g_cancellable_cancel (s_pCancel); // will be freed in _proxy_connected ()
		g_object_unref (s_pCancel);
		s_pCancel = NULL;
	}
	if (s_pTask)
	{
		gldi_task_discard (s_pTask);
		s_pTask = NULL;
	}
	if (dbus_proxy_tomboy)
	{
		g_object_unref (dbus_proxy_tomboy);
		dbus_proxy_tomboy = NULL;
	}
}

static void _on_name_vanished (G_GNUC_UNUSED GDBusConnection *connection, G_GNUC_UNUSED const gchar *name, G_GNUC_UNUSED gpointer user_data)
{
	CD_APPLET_ENTER;
	
	if (s_pCancel || dbus_proxy_tomboy) cd_message ("GNote exited");
	else cd_message ("GNote not started yet?");
	
	_tomboy_disconnect_from_service ();
	
	CD_APPLET_LEAVE ();
}


static void start (void)
{
	g_return_if_fail (s_uWatch == 0);
	
	// detect the service on the bus asynchronously (note: only Gnote is supported)
	s_uWatch = g_bus_watch_name (G_BUS_TYPE_SESSION, "org.gnome.Gnote", G_BUS_NAME_WATCHER_FLAGS_NONE,
		_on_name_appeared, _on_name_vanished, NULL, NULL);
}

static void stop (void)
{
	cd_message ("");
	
	if (s_uWatch)
	{
		g_bus_unwatch_name (s_uWatch);
		s_uWatch = 0;
	}
	_tomboy_disconnect_from_service ();
}

static void show_note (const gchar *cNoteID)
{
	g_return_if_fail (dbus_proxy_tomboy != NULL);
	g_dbus_proxy_call (dbus_proxy_tomboy, "DisplayNote", g_variant_new ("(s)", cNoteID),
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

static void delete_note (const gchar *cNoteID)
{
	g_return_if_fail (dbus_proxy_tomboy != NULL);
	g_dbus_proxy_call (dbus_proxy_tomboy, "DeleteNote", g_variant_new ("(s)", cNoteID),
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

static gchar *create_note (const gchar *cTitle)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, NULL);
	
	GError *erreur = NULL;
	gchar *cNoteID = NULL;
	GVariant *res = g_dbus_proxy_call_sync (dbus_proxy_tomboy,
		"CreateNamedNote", g_variant_new ("(s)", cTitle),
		G_DBUS_CALL_FLAGS_NONE, -1, s_pCancel, &erreur);
	
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error creating new note: %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	else 
	{
		if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(s)")))
			g_variant_get (res, "(s)", &cNoteID);
		else cd_warning ("Unexpected note ID type: %s", g_variant_get_type_string (res));
		g_variant_unref (res);
	}
	
	return cNoteID;
}

static void run_manager (void)
{
	char *cClass = cairo_dock_register_class ("org.gnome.gnote");
	if (!cClass)
	{
		cd_warning ("Cannot find GNote, check that it is installed");
		return;
	}
	
	GldiAppInfo *info = cairo_dock_get_class_app_info (cClass);
	g_free (cClass);
	
	gldi_app_info_launch (info, NULL);
}

static gchar **_get_note_names_with_tag (const gchar *cTag)
{
	g_return_val_if_fail (dbus_proxy_tomboy != NULL, NULL);
	
	GError *erreur = NULL;
	gchar **cNoteNames = NULL;
	GVariant *res = g_dbus_proxy_call_sync (dbus_proxy_tomboy,
		"GetAllNotesWithTag", g_variant_new ("(s)", cTag),
		G_DBUS_CALL_FLAGS_NONE, -1, s_pCancel, &erreur);
	
	if (erreur)
	{
		if (! g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Error getting notes with tag: %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	else 
	{
		if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(as)")))
			g_variant_get (res, "(^as)", &cNoteNames);
		else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (res));
		g_variant_unref (res);
	}
	
	return cNoteNames;
}


void cd_notes_register_tomboy_backend (void)
{
	myData.backend.start = start;
	myData.backend.stop = stop;
	myData.backend.show_note = show_note;
	myData.backend.delete_note = delete_note;
	myData.backend.create_note = create_note;
	myData.backend.run_manager = run_manager;
	myData.backend.get_notes_with_tag = _get_note_names_with_tag;
}

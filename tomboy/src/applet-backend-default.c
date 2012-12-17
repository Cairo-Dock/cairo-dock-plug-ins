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

#include <glib/gstdio.h>

#include "tomboy-struct.h"
#include "applet-notes.h"
#include "applet-backend-default.h"

static GtkWidget *s_pNoteWindow = NULL;


static CDNote *_get_note (const gchar *cNoteID)
{
	CDNote *pNote = g_new0 (CDNote, 1);
	pNote->cID = g_strdup (cNoteID);
	
	// get the content
	gsize length = 0;
	gchar *cContent=NULL;
	g_file_get_contents (cNoteID,
		&cContent,
		&length,
		NULL);
	
	// parse each line: creation date - last change date - tags - title - content
	if (cContent != NULL)
	{
		gchar *str, *ptr;
		
		// line 1
		ptr = cContent;
		str = strchr (ptr, '\n');
		if (str) *str = '\0';
		pNote->iCreationDate = atoi (ptr);
		
		if (str)
		{
			// line 2
			ptr = str + 1;
			str = strchr (ptr, '\n');
			if (str) *str = '\0';
			pNote->iLastChangeDate = atoi (ptr);
			
			if (str)
			{
				// line 3
				ptr = str + 1;
				str = strchr (ptr, '\n');
				if (str) *str = '\0';
				pNote->cTags = g_strdup (ptr);
				
				if (str)
				{
					// line 4
					ptr = str + 1;
					str = strchr (ptr, '\n');
					if (str) *str = '\0';
					pNote->cTitle = g_strdup (ptr);
					
					if (str)
					{
						// line 5+
						ptr = str + 1;
						pNote->cContent = g_strdup (ptr);  // until the end
					}
				}
			}
		}
		
		g_free (cContent);
	}
	return pNote;
}

static void _save_note (CDNote *pNote)
{
	gchar *cContent = g_strdup_printf ("%d\n\
%d\n\
%s\n\
%s\n\
%s",
		pNote->iCreationDate,
		pNote->iLastChangeDate,
		pNote->cTags? pNote->cTags:"",
		pNote->cTitle?pNote->cTitle:"",
		pNote->cContent?pNote->cContent:"");
	g_file_set_contents (pNote->cID,
            cContent,
            -1,
            NULL);
	g_free (cContent);
}
	
static void on_delete_note_window (GtkWidget *pWidget, gchar *cNoteID)
{
	//g_print ("%s ()\n", __func__);
	// save the note.
	CDNote *pNote = g_new0 (CDNote, 1);
	pNote->cID = cNoteID;  // 'cNoteID' was allocated, and has to be destroyed; it will be destroyed with 'pNote'.
	
	GtkWidget *pTitleWidget = g_object_get_data (G_OBJECT (pWidget), "cd-title-widget");
	pNote->cTitle = g_strdup (gtk_entry_get_text (GTK_ENTRY (pTitleWidget)));
	
	GtkWidget *pTextWidget = g_object_get_data (G_OBJECT (pWidget), "cd-text-widget");
	GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pTextWidget));
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter (pBuffer, &start);
	gtk_text_buffer_get_end_iter (pBuffer, &end);
	pNote->cContent = gtk_text_buffer_get_text (pBuffer,
		&start,
		&end,
		FALSE);
	
	_save_note (pNote);
	
	cd_notes_store_update_note (pNote);
	
	cd_notes_free_note (pNote);
	
	s_pNoteWindow = NULL;
	g_object_unref (pTitleWidget);
	g_object_unref (pTextWidget);
}

gboolean _on_key_press (G_GNUC_UNUSED GtkWidget *pWidget,
	GdkEventKey *pKey,
	CairoDockModuleInstance *myApplet)
{
	if (pKey->type == GDK_KEY_PRESS && pKey->keyval == GLDI_KEY(Escape))
	{
		gtk_widget_destroy (s_pNoteWindow);
		return TRUE;
	}
	return FALSE;
}

static void show_note (const gchar *cNoteID)
{
	if (s_pNoteWindow != NULL)
	{
		gtk_widget_destroy (s_pNoteWindow);
	}
	
	CDNote *pNote = _get_note (cNoteID);
	
	s_pNoteWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_modal (GTK_WINDOW (s_pNoteWindow), TRUE);  // set as modal, since the dialog it comes from (the one containing the calendar) is modal (as any interactive dialog).
	
	g_signal_connect (G_OBJECT (s_pNoteWindow),
		"key-press-event",
		G_CALLBACK (_on_key_press),
		myApplet);
	
	g_signal_connect (s_pNoteWindow, "destroy", G_CALLBACK (on_delete_note_window), g_strdup (cNoteID));
	gtk_window_set_keep_above (GTK_WINDOW (s_pNoteWindow), TRUE);
	gtk_window_resize (GTK_WINDOW (s_pNoteWindow), 640, 300);
	
	GtkWidget *vbox = _gtk_vbox_new (3);
	gtk_container_add (GTK_CONTAINER (s_pNoteWindow), vbox);
	
	GtkWidget *hbox = _gtk_hbox_new (3);
	gtk_box_pack_start (GTK_BOX (vbox),
		hbox,
		FALSE,
		FALSE,
		0);
	
	GtkWidget *pLabel = gtk_label_new (D_("Title:"));
	gtk_box_pack_start (GTK_BOX (hbox),
		pLabel,
		FALSE,
		FALSE,
		0);
	
	GtkWidget *pTitleWidget = gtk_entry_new ();
	g_object_set_data (G_OBJECT (s_pNoteWindow), "cd-title-widget", pTitleWidget);
	g_object_ref (pTitleWidget);  // so that the widget stays alive until the top window is destroyed (and the note saved).
	gtk_box_pack_start (GTK_BOX (hbox),
		pTitleWidget,
		TRUE,
		TRUE,
		0);
	gtk_entry_set_text (GTK_ENTRY (pTitleWidget), pNote->cTitle);
	
	/** Lister les tags et les ajouter a la combo ...
	#if (GTK_MAJOR_VERSION < 3 && GTK_MINOR_VERSION < 24)
	#define _combo_box_entry_new gtk_combo_box_entry_new
	#else
	#define _combo_box_entry_new gtk_combo_box_new_with_entry
	#endif
	GtkWidget *pTagsWidget = _combo_box_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox),
		pTagsWidget,
		TRUE,
		TRUE,
		0);*/
	
	GtkWidget *pTextWidget = gtk_text_view_new ();
	g_object_set_data (G_OBJECT (s_pNoteWindow), "cd-text-widget", pTextWidget);
	g_object_ref (pTextWidget);
	GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pTextWidget);
	//g_object_set (pScrolledWindow, "width-request", 600, "height-request", 250, NULL);
	gtk_box_pack_start (GTK_BOX (vbox),
		pScrolledWindow,
		TRUE,
		TRUE,
		0);
	
	if (pNote->cContent != NULL)
	{
		GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pTextWidget));
		gtk_text_buffer_set_text (pBuffer, pNote->cContent, -1);
	}
	
	gtk_widget_show_all (s_pNoteWindow);
}

static void delete_note (const gchar *cNoteID)
{
	g_remove (cNoteID);
	
	cd_notes_store_remove_note (cNoteID);
}

static gchar *create_note (const gchar *cTitle)
{
	// make an unique ID as the path to the note file.
	gchar *cNoteID = NULL;
	
	time_t t = time (NULL);
	cNoteID = g_strdup_printf ("%s/notes/note_%ld", g_cCairoDockDataDir, t);
	if (g_file_test (cNoteID, G_FILE_TEST_EXISTS))
	{
		int i = 1;
		do
		{
			g_free (cNoteID);
			cNoteID = g_strdup_printf ("%s/notes/note_%ld-%d", g_cCairoDockDataDir, t, i++);
		}
		while (g_file_test (cNoteID, G_FILE_TEST_EXISTS));
	}
	
	// create an empty note.
	CDNote *pNote = g_new0 (CDNote, 1);
	pNote->cID = g_strdup (cNoteID);
	pNote->cTitle = g_strdup (cTitle);
	pNote->iCreationDate = t;
	pNote->iLastChangeDate = t;
	
	_save_note (pNote);
	
	// store it.
	cd_notes_store_add_note (pNote);
	
	cd_notes_free_note (pNote);
	return cNoteID;
}


typedef struct {
	gchar *cNotesDir;
	gboolean bError;
	GList *pNotes;
	} CDSharedMemory; 
static void _get_notes_data_async (CDSharedMemory *pSharedMemory)
{
	gchar *cNotesDir = pSharedMemory->cNotesDir;
	if (! g_file_test (cNotesDir, G_FILE_TEST_EXISTS))
	{
		if (g_mkdir (cNotesDir, 7*8*8+5*8+5) != 0)
		{
			cd_warning ("Couldn't make folder %s\n Check permissions.", cNotesDir);
			pSharedMemory->bError = TRUE;
			return;
		}
	}
	
	GDir *dir = g_dir_open (cNotesDir, 0, NULL);
	if (!dir)
	{
		pSharedMemory->bError = TRUE;
		cd_warning ("Couldn't read folder %s\n Check permissions.", cNotesDir);
		return;
	}
	
	GList *pNotes = NULL;
	CDNote *pNote;
	GString *sNotePath = g_string_new ("");
	const gchar *cFileName;
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		g_string_printf (sNotePath, "%s/%s", cNotesDir, cFileName);
		pNote = _get_note (sNotePath->str);
		pNotes = g_list_prepend (pNotes, pNote);
	}
	g_dir_close (dir);
	pNotes = g_list_reverse (pNotes);
	
	pSharedMemory->pNotes = pNotes;
}
static gboolean _build_notes_from_data (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	if (pSharedMemory->bError)
	{
		if (myData.iIconState != 1)
		{
			myData.iIconState = 1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "close.svg");
		}
	}
	else
	{
		cd_notes_store_load_notes (pSharedMemory->pNotes);
	}
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	CD_APPLET_LEAVE (FALSE);
}
static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_list_foreach (pSharedMemory->pNotes, (GFunc)cd_notes_free_note, NULL);
	g_list_free (pSharedMemory->pNotes);
	g_free (pSharedMemory->cNotesDir);
	g_free (pSharedMemory);
}

static void start (void)
{
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cNotesDir = g_strdup_printf ("%s/notes", g_cCairoDockDataDir);
	myData.pTask = cairo_dock_new_task_full (0,  // 1 shot task.
		(CairoDockGetDataAsyncFunc) _get_notes_data_async,
		(CairoDockUpdateSyncFunc) _build_notes_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	
	cairo_dock_launch_task (myData.pTask);
	
	myData.bIsRunning = TRUE;
	if (myData.iIconState != 0)
	{
		myData.iIconState = 0;
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconDefault, "icon.svg");
	}
}

static void stop (void)
{
	if (s_pNoteWindow)
		gtk_widget_destroy (s_pNoteWindow);
}

void cd_notes_register_default_backend (void)
{
	myData.backend.start = start;
	myData.backend.stop = stop;
	myData.backend.show_note = show_note;
	myData.backend.delete_note = delete_note;
	myData.backend.create_note = create_note;
	myData.backend.run_manager = NULL;
}

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

// Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@glx-dock.org)

#include "applet-struct.h"
#include "applet-run-dialog.h"

#define __USE_BSD 1
#include <string.h>
#include <dirent.h>
/* With GLib 2.37.93, dirent.h is now included (in glib/gdir.h) but
 * when '__USE_BSD' is not defined and then DT_DIR and DT_LNK are not defined
 */
#ifndef DT_DIR
	#define DT_DIR 4
#endif
#ifndef DT_LNK
	#define DT_LNK 10
#endif
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>


static GList *
fill_files_from (const char *dirname,
		 const char *dirprefix,
		 char        prefix,
		 GList      *existing_items)
{
	GList         *list;
	DIR           *dir;
	struct dirent *dent;
	
	list = NULL;
	dir = opendir (dirname);
	
	if (!dir)
		return list;
	
	while ((dent = readdir (dir))) {
		char       *file;
		char       *item;
		const char *suffix;
		
		if (!dent->d_name ||
		    dent->d_name [0] != prefix)
			continue;

		file = g_build_filename (dirname, dent->d_name, NULL);
		
		suffix = NULL;
		if (
		/* don't use g_file_test at first so we don't stat() */
		    dent->d_type == DT_DIR ||
		    (dent->d_type == DT_LNK &&
		     g_file_test (file, G_FILE_TEST_IS_DIR))
			//g_file_test (file, G_FILE_TEST_IS_DIR)
		   )
			suffix = "/";
		
		g_free (file);
		
		item = g_build_filename (dirprefix, dent->d_name, suffix, NULL);
		
		list = g_list_prepend (list, item);
	}

	closedir (dir);
	
	return list;
}	

static GList *
fill_possible_executables (void)
{
	GList         *list;
	const char    *path;
	char         **pathv;
	int            i;
	
	list = NULL;
	path = g_getenv ("PATH");

	if (!path || !path [0])
		return list;

	pathv = g_strsplit (path, ":", 0);
	
	for (i = 0; pathv [i]; i++) {
		const char *file;
		char       *filename;
		GDir       *dir;

		dir = g_dir_open (pathv [i], 0, NULL);

		if (!dir)
			continue;

		while ((file = g_dir_read_name (dir))) {
			filename = g_build_filename (pathv [i], file, NULL);
			list = g_list_prepend (list, filename);
		}

		g_dir_close (dir);
	}
	
	g_strfreev (pathv);
	
	return list;
}

static GList *
fill_executables (GList *possible_executables,
		  GList *existing_items,
		  char   prefix)
{
	GList *list;
	GList *l;
	
	list = NULL;	
	
	for (l = possible_executables; l; l = l->next) {
		const char *filename;
		char       *basename;
			
		filename = l->data;
		basename = g_path_get_basename (filename);
			
		if (basename [0] == prefix &&
		    g_file_test (filename, G_FILE_TEST_IS_REGULAR) &&
		    g_file_test (filename, G_FILE_TEST_IS_EXECUTABLE)) {

			if (g_list_find_custom (existing_items, basename,
						(GCompareFunc) strcmp)) {
				g_free (basename);
				return NULL;
			}

			list = g_list_prepend (list, basename);
		 } else {
			g_free (basename);
		 }
	}
	
	return list;
}

static void cd_menu_run_dialog_update_completion (GldiModuleInstance *myApplet,
	const char *text)
{
	GList *list;
	GList *executables;
	char   prefix;
	char  *buf;
	char  *dirname;
	char  *dirprefix;
	char  *key;

	g_assert (text != NULL && *text != '\0' && !g_ascii_isspace (*text));

	list = NULL;
	executables = NULL;

	if (!myData.completion) {
		myData.completion = g_completion_new (NULL);
		myData.possible_executables = fill_possible_executables ();
		myData.dir_hash = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free, NULL);
	}
	
	buf = g_path_get_basename (text);
	prefix = buf[0];
	g_free (buf);
	if (prefix == '/' || prefix == '.')
		return;

	if (text [0] == '/') {
		/* complete against absolute path */
		dirname = g_path_get_dirname (text);
		dirprefix = g_strdup (dirname);
	} else {
		/* complete against relative path and executable name */
		if (!strchr (text, '/')) {
			executables = fill_executables (myData.possible_executables,
							myData.completion_items,
							text [0]);
			dirprefix = g_strdup ("");
		} else {
			dirprefix = g_path_get_dirname (text);
		}

		dirname = g_build_filename (g_get_home_dir (), dirprefix, NULL);
	}

	key = g_strdup_printf ("%s%c%c", dirprefix, G_DIR_SEPARATOR, prefix);

	if (!g_hash_table_lookup (myData.dir_hash, key)) {
		g_hash_table_insert (myData.dir_hash, key, myApplet);

		list = fill_files_from (dirname, dirprefix, prefix,
					myData.completion_items);
	} else {
		g_free (key);
	}

	list = g_list_concat (list, executables);

	g_free (dirname);
	g_free (dirprefix);

	if (list == NULL)
		return;
		
	g_completion_add_items (myData.completion, list);
		
	myData.completion_items = g_list_concat (myData.completion_items, list);
}

static gboolean _entry_event (GtkEditable *entry,
	GdkEventKey *event,
	GldiModuleInstance *myApplet)
{
	char             *prefix;
	char             *nospace_prefix;
	char             *nprefix;
	char             *temp;
	gint               pos, tmp;

	if (event->type != GDK_KEY_PRESS)
		return FALSE;

	/* tab completion */
	if (event->keyval == GDK_KEY_Tab) {
		gtk_editable_get_selection_bounds (entry, &pos, &tmp);

		if (myData.completion_started &&
		    pos != tmp &&
		    pos != 1 &&
		    tmp == strlen (gtk_entry_get_text (GTK_ENTRY (entry)))) {
			gtk_editable_select_region (entry, 0, 0);
			gtk_editable_set_position (entry, -1);
			
			return TRUE;
		}
	} else if (event->length > 0) {
			   
		gtk_editable_get_selection_bounds (entry, &pos, &tmp);

		if (myData.completion_started &&
		    pos != tmp &&
		    pos != 0 &&
		    tmp == strlen (gtk_entry_get_text (GTK_ENTRY (entry)))) {
			temp = gtk_editable_get_chars (entry, 0, pos);
			prefix = g_strconcat (temp, event->string, NULL);
			g_free (temp);
		} else if (pos == tmp &&
			   tmp == strlen (gtk_entry_get_text (GTK_ENTRY (entry)))) {
			prefix = g_strconcat (gtk_entry_get_text (GTK_ENTRY (entry)),
					      event->string, NULL);
		} else {
			return FALSE;
		}
		
		nospace_prefix = prefix;
		while (g_ascii_isspace (*nospace_prefix))
			nospace_prefix ++;
		if (*nospace_prefix == '\0')
			return FALSE;

		cd_menu_run_dialog_update_completion (myApplet, nospace_prefix);
		
		if (!myData.completion) {
			g_free (prefix);
			return FALSE;
		}
		
		pos = strlen (prefix);
		nprefix = NULL;

		g_completion_complete_utf8 (myData.completion,
			nospace_prefix,
			&nprefix);

		if (nprefix) {
			int insertpos;
			insertpos = 0;

			temp = g_strndup (prefix, nospace_prefix - prefix);
			g_free (prefix);

			prefix = g_strconcat (temp, nprefix, NULL);

			gtk_editable_delete_text (entry, 0, -1);

			gtk_editable_insert_text (entry,
						  prefix, strlen (prefix),
						  &insertpos);

 			gtk_editable_set_position (entry, pos);
			gtk_editable_select_region (entry, pos, -1);
			
			myData.completion_started = TRUE;

			g_free (temp);
			g_free (nprefix);
			g_free (prefix);
			
			return TRUE;
		}
		
		g_free (prefix);
	}
	
	return FALSE;
}


static void _cd_menu_on_quick_launch (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok ou entree.
	{
		const gchar *cCommand = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cCommand != NULL && *cCommand != '\0')
			cairo_dock_launch_command_full (cCommand, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY (pInteractiveWidget), "");
	}
	gldi_object_ref (GLDI_OBJECT(myData.pQuickLaunchDialog));
	gldi_dialog_hide (myData.pQuickLaunchDialog);
}
static void _dialog_closed (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	myData.pQuickLaunchDialog = NULL;
	CD_APPLET_LEAVE();
}
void cd_run_dialog_show_hide (GldiModuleInstance *myApplet)
{
	if (myData.pQuickLaunchDialog == NULL)
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (GLDI_ICON_NAME_EXECUTE, myData.iPanelDefaultMenuIconSize);
		myData.pQuickLaunchDialog = gldi_dialog_show_with_entry (D_("Enter a command to launch:"),
			myIcon, myContainer,
			cIconPath ? cIconPath : "same icon",
			NULL,
			(CairoDockActionOnAnswerFunc) _cd_menu_on_quick_launch, myApplet, (GFreeFunc) _dialog_closed);
		g_free (cIconPath);
		
		GtkWidget *pEntry = myData.pQuickLaunchDialog->pInteractiveWidget;
		g_signal_connect (pEntry, "key-press-event",
			G_CALLBACK (_entry_event),
			myApplet);
	}
	else
	{
		gldi_dialog_toggle_visibility (myData.pQuickLaunchDialog);
	}
}

void cd_run_dialog_free (void)
{
	if (myData.pQuickLaunchDialog)
		gldi_object_unref (GLDI_OBJECT(myData.pQuickLaunchDialog));
	
	if (myData.dir_hash)
		g_hash_table_destroy (myData.dir_hash);
	
	GList *l;
	for (l = myData.possible_executables; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.possible_executables);
	
	for (l = myData.completion_items; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.completion_items);
	
	if (myData.completion)
		g_completion_free (myData.completion);
}

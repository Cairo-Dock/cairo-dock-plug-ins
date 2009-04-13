/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#define __USE_BSD 1
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "applet-struct.h"
#include "applet-command-finder.h"
#include "applet-session.h"


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

void cd_do_update_completion (const char *text)
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

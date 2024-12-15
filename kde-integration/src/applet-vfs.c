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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gio.h>

#include "applet-utils.h"
#include "applet-vfs.h"

static gchar *s_kioclient = NULL;
static const gchar *_get_kioclient (void)
{
	if (!s_kioclient) s_kioclient = g_strdup_printf ("kioclient%s", get_kioclient_number());
	return s_kioclient;
}

void vfs_backend_launch_uri (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);

	cd_debug ("%s (%s)", __func__, cURI);
	const gchar * const args[] = {_get_kioclient (), "exec", cURI, NULL};
	cairo_dock_launch_command_argv_full (args, NULL, TRUE);

	/// tester ca :
	//KURL url(cURI);
	//new KRun(url);
}


gboolean vfs_backend_delete_file (const gchar *cURI, gboolean bNoTrash)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	
	if (bNoTrash)
	{
		GError *erreur = NULL;
		gchar *cFilePath = g_filename_from_uri (cURI, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("%s", erreur->message);
			g_error_free (erreur);
			return FALSE;
		}
		const gchar * const args[] = {"rm", "-rf", cFilePath, NULL};
		cairo_dock_launch_command_argv (args);
		g_free (cFilePath);
	}
	else
	{
		const gchar * const args[] = {_get_kioclient (), "move", cURI, "trash:/", NULL};
		cairo_dock_launch_command_argv (args);
	}
	return TRUE;
}

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	g_return_val_if_fail (cOldURI != NULL, FALSE);
	
	gboolean bSuccess = FALSE;
	gchar *cPath = g_path_get_dirname (cOldURI);
	if (cPath)
	{
		gchar *cNewURI = g_strdup_printf ("%s/%s", cPath, cNewName);
		const gchar * const args[] = {_get_kioclient (), "move", cOldURI, cNewURI, NULL};
		cairo_dock_launch_command_argv (args);
		g_free (cNewURI);
		bSuccess = TRUE;
	}
	g_free (cPath);
	return bSuccess;
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	cd_message (" %s -> %s", cURI, cDirectoryURI);
	
	gchar *cFileName = g_path_get_basename (cURI);
	gchar *cNewFileURI = g_strconcat (cDirectoryURI, "/", cFileName, NULL);
	const gchar * const args[] = {_get_kioclient (), "move", cURI, cNewFileURI, NULL};
	cairo_dock_launch_command_argv (args);
	g_free (cNewFileURI);
	g_free (cFileName);
	return TRUE;
}

void vfs_backend_empty_trash (void)
{
	static char *ktrash = NULL;
	if (!ktrash)
	{
		// ktrash gets a postfix with version number similarly to kioclient
		ktrash = g_strdup_printf ("ktrash%s", get_kioclient_number ());
	}
	const gchar * const args[] = {ktrash, "--empty", NULL};
	cairo_dock_launch_command_argv (args);
}


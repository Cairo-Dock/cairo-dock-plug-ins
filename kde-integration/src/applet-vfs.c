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

void vfs_backend_launch_uri (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);

	cd_debug ("%s (%s)", __func__, cURI);
	gchar *cCommand = g_strdup_printf ("kioclient%s exec \"%s\"", get_kioclient_number(), cURI);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);

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
		gchar *cCommand = g_strdup_printf ("rm -rf \"%s\"", cFilePath);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
		g_free (cFilePath);
	}
	else
	{
		gchar *cCommand = g_strdup_printf ("kioclient%s move \"%s\" trash:/", get_kioclient_number(), cURI);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
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
		gchar *cCommand = g_strdup_printf ("kioclient%s move \"%s\" \"%s\"", get_kioclient_number(), cOldURI, cNewURI);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
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
	gchar *cCommand = g_strdup_printf ("kioclient%s move \"%s\" \"%s\"", get_kioclient_number(), cURI, cNewFileURI);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
	g_free (cNewFileURI);
	g_free (cFileName);
	return TRUE;
}

void vfs_backend_empty_trash (void)
{
	static char *cmd = NULL;
	if (!cmd)
	{
		// ktrash gets a postfix with version number similarly to kioclient
		cmd = g_strdup_printf ("ktrash%s --empty", get_kioclient_number());
	}
	cairo_dock_launch_command (cmd);
}


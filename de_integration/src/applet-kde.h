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


#ifndef __KDE_INTEGRATION__
#define __KDE_INTEGRATION__

#include <cairo-dock.h>

void vfs_backend_launch_uri (const gchar *cURI);

gboolean vfs_backend_delete_file (const gchar *cURI, gboolean bNoTrash);

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName);

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI);

void vfs_backend_empty_trash (void);

void env_backend_logout (CairoDockFMConfirmationFunc cb_confirm, gpointer data);

void env_backend_shutdown (CairoDockFMConfirmationFunc cb_confirm, gpointer data);

void env_backend_reboot (CairoDockFMConfirmationFunc cb_confirm, gpointer data);

void env_backend_lock_screen (void);

void env_backend_setup_time (void);

void env_backend_show_system_monitor (void);

int get_kde_version (void);

const gchar *get_kioclient_number (void);

#endif


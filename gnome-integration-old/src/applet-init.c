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

#include "stdlib.h"

#include "applet-gnome-vfs.h"
#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("gnome integration old",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet provides functions for a better integration into a GNOME environnement.\n"
	"It is auto-activated, so you don't need to activate it.\n"
	"It is designed for old Gnome version (prior to 2.22)."),
	"Fabounet (Fabrice Rey)")
	//if (g_iDesktopEnv == CAIRO_DOCK_GNOME)  ///  && glib_major_version == 2 && glib_minor_version < 16  <--- quand il y'aura les 2...
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME && glib_major_version == 2 && glib_minor_version < 16)
	{
		if (init_vfs_backend ())
		{
			CairoDockDesktopEnvBackend *pVFSBackend = g_new0 (CairoDockDesktopEnvBackend, 1);
			pVFSBackend->get_file_info = vfs_backend_get_file_info;
			pVFSBackend->get_file_properties = vfs_backend_get_file_properties;
			pVFSBackend->list_directory = vfs_backend_list_directory;
			pVFSBackend->launch_uri = vfs_backend_launch_uri;
			pVFSBackend->is_mounted = vfs_backend_is_mounted;
			pVFSBackend->mount = vfs_backend_mount;
			pVFSBackend->unmount = vfs_backend_unmount;
			pVFSBackend->add_monitor = vfs_backend_add_monitor;
			pVFSBackend->remove_monitor = vfs_backend_remove_monitor;
			pVFSBackend->delete_file = vfs_backend_delete_file;
			pVFSBackend->rename = vfs_backend_rename_file;
			pVFSBackend->move = vfs_backend_move_file;
			pVFSBackend->get_trash_path = vfs_backend_get_trash_path;
			pVFSBackend->get_desktop_path = vfs_backend_get_desktop_path;
			pVFSBackend->logout = env_backend_logout;
			pVFSBackend->shutdown = env_backend_logout;
			pVFSBackend->setup_time = env_backend_setup_time;
			pVFSBackend->show_system_monitor = env_backend_show_system_monitor;
			cairo_dock_fm_register_vfs_backend (pVFSBackend);
		}
	}
	else
		return FALSE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END

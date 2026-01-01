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

#include "applet-vfs.h"
#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN ("kde integration",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet provides functions for a better integration into a KDE environnement.\n"
	"It is auto-activated, so you don't need to activate it.\n"),
	"Fabounet (Fabrice Rey)")
	if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		CairoDockDesktopEnvBackend VFSBackend = { NULL };
		
		// here, we provide / override KDE-specific stuff (basics are provided by the GIO VFS backend in core)
		VFSBackend.launch_uri = vfs_backend_launch_uri;
		VFSBackend.delete_file = vfs_backend_delete_file;
		VFSBackend.rename = vfs_backend_rename_file;
		VFSBackend.move = vfs_backend_move_file;
		VFSBackend.empty_trash = vfs_backend_empty_trash;
		VFSBackend.logout = env_backend_logout;
		VFSBackend.shutdown = env_backend_shutdown;
		VFSBackend.reboot = env_backend_reboot;
		VFSBackend.lock_screen = env_backend_lock_screen;
		VFSBackend.setup_time = env_backend_setup_time;
		VFSBackend.show_system_monitor = env_backend_show_system_monitor;
		cairo_dock_fm_register_vfs_backend (&VFSBackend, TRUE); // TRUE: overwrite previously registered functions
	}
	else
		return FALSE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END

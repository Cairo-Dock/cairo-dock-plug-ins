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

#include "cairo-dock-gio-vfs.h"

#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("gnome integration",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	"This applet provides functions for a better integration into a GNOME environnement.\n"
	"It is auto-activated, so you don't need to activate it.\n"
	"It is designed for the a GNOME version >= 2.22",
	"Fabounet (Fabrice Rey)")
	
	CairoDockDesktopEnvBackend *pVFSBackend = NULL;
	if (! cairo_dock_fm_vfs_backend_is_defined ())  // the Gnome backend will register the GVFS functions, even if it's not a Gnome environment. It will not overwrite the other functions of the backend, and if another backend comes later, it can set its own functions.
	{
		if (cairo_dock_gio_vfs_init ())
		{
			cd_debug ("GVFS");
			pVFSBackend = g_new0 (CairoDockDesktopEnvBackend, 1);
			cairo_dock_gio_vfs_fill_backend (pVFSBackend);
		}
	}
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME && (glib_major_version > 2 || glib_minor_version >= 16))
	{
		cd_debug ("GNOME");
		if (pVFSBackend == NULL)
			pVFSBackend = g_new0 (CairoDockDesktopEnvBackend, 1);
			
		pVFSBackend->logout = env_backend_logout;
		pVFSBackend->shutdown = env_backend_shutdown;
		pVFSBackend->reboot = env_backend_shutdown;
		pVFSBackend->lock_screen = env_backend_lock_screen;
		pVFSBackend->setup_time = env_backend_setup_time;
		pVFSBackend->show_system_monitor = env_backend_show_system_monitor;
	}
	
	if (pVFSBackend != NULL)
		cairo_dock_fm_register_vfs_backend/*_if_none*/ (pVFSBackend);
	else
		return FALSE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END

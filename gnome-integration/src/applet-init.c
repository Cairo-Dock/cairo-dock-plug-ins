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

#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN ("gnome integration",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	"This applet provides functions for a better integration into a GNOME environnement.\n"
	"It is auto-activated, so you don't need to activate it.\n"
	"It is designed for the a GNOME version >= 2.22",
	"Fabounet (Fabrice Rey)")
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME && (glib_major_version > 2 || glib_minor_version >= 16))
	{
		cd_debug ("GNOME");
		CairoDockDesktopEnvBackend VFSBackend = { NULL };
		
		/* calling gnome-session-quit will only work if either
		 * gnome-shell or gnome-flashback are running */
		if (cairo_dock_dbus_detect_application ("org.gnome.Shell"))
		{
			VFSBackend.logout = env_backend_logout;
			VFSBackend.shutdown = env_backend_shutdown;
			VFSBackend.reboot = env_backend_shutdown;
		}
		// this calls shared-files/scripts/lock-screen.sh which will not work on Wayland
		if (! gldi_container_is_wayland_backend ())
			VFSBackend.lock_screen = env_backend_lock_screen;
		VFSBackend.setup_time = env_backend_setup_time;
		VFSBackend.show_system_monitor = env_backend_show_system_monitor;
		
		cairo_dock_fm_register_vfs_backend (&VFSBackend, TRUE); // TRUE: overwrite previously registered functions
	}
	else
		return FALSE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END

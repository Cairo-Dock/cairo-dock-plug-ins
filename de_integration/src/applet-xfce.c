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

#include "applet-init.h"


static void _xfce_logout (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	// we ignore cb_confirm as the below command will ask for confirmation
	cairo_dock_launch_command_single ("xfce4-session-logout");
}

static void _xfce_lock_screen (void)
{
	cairo_dock_launch_command_single (MY_APPLET_SHARE_DATA_DIR"/../shared-files/scripts/lock-screen.sh");
}

static void _xfce_setup_time (void)
{
	const char * const args[] = {"gksu", "system-config-date", NULL}; // not installed by default + gksu does not work on newer systems and on Wayland
	cairo_dock_launch_command_argv (args);
}

static void _xfce_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("xfce4-taskmanager");  // not installed by default
}

void cd_xfce_integration_init (void)
{
	CairoDockDesktopEnvBackend VFSBackend = { NULL };
		
	VFSBackend.logout = _xfce_logout;
	VFSBackend.shutdown = _xfce_logout;
	VFSBackend.reboot = _xfce_logout;
	VFSBackend.lock_screen = _xfce_lock_screen;
	VFSBackend.setup_time = _xfce_setup_time;
	VFSBackend.show_system_monitor = _xfce_show_system_monitor;
	cairo_dock_fm_register_de_integration_override_backend (&VFSBackend);
}	


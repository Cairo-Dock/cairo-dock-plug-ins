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

#include "applet-utils.h"


void env_backend_logout (void)
{
	cairo_dock_launch_command_single ("xfce4-session-logout");
}

void env_backend_shutdown (void)
{
	cairo_dock_launch_command_single ("xfce4-session-logout");  // avec les options telles que --halt, la fenetre n'est pas montree.
}

void env_backend_lock_screen (void)
{
	cairo_dock_launch_command_single (MY_APPLET_SHARE_DATA_DIR"/../shared-files/scripts/lock-screen.sh");
}

void env_backend_setup_time (void)
{
	const char * const args[] = {"gksu", "system-config-date", NULL}; // not installed by default + gksu does not work on newer systems and on Wayland
	cairo_dock_launch_command_argv (args);
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("xfce4-taskmanager");  // not installed by default
}


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


void env_backend_logout (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	// we ignore cb_confirm as the below command will ask for confirmation
	const char * const args[] = {"cosmic-osd", "log-out", NULL};
	cairo_dock_launch_command_argv (args); // no need for xdg-activation, will show on top anyway
}

void env_backend_shutdown (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	const char * const args[] = {"cosmic-osd", "shutdown", NULL};
	cairo_dock_launch_command_argv (args);
}

void env_backend_reboot (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	const char * const args[] = {"cosmic-osd", "restart", NULL};
	cairo_dock_launch_command_argv (args);
}

void env_backend_lock_screen (void)
{
	const char * const args[] = {"loginctl", "lock-session", NULL}; // TODO: should use DBus?
	cairo_dock_launch_command_argv (args);
}

void env_backend_setup_time (void)
{
	const char * const args[] = {"cosmic-settings", "date-time", NULL};
	cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
}


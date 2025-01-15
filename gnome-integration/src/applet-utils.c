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
#include <cairo-dock.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	// since Gnome 3, gnome-session-save has been replaced by gnome-session-quit
	const gchar *args[] = {"which", "gnome-session-quit", NULL, NULL};
	gchar *cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
	if (cResult != NULL && *cResult == '/')
	{
		args[2] = "--logout";
		cairo_dock_launch_command_argv_full (args + 1, NULL, GLDI_LAUNCH_GUI); // i.e. gnome-session-quit --logout
	}
	else
	{
		g_free (cResult);
		args[1] = "cinnamon-session-quit";
		// Cinnamon?
		cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
		if (cResult != NULL && *cResult == '/')
		{
			args[2] = "--logout";
			cairo_dock_launch_command_argv_full (args + 1, NULL, GLDI_LAUNCH_GUI);
		}
		else
		{
			args[0] = "gnome-session-save";
			args[1] = "--kill";
			args[2] = "--gui";
			cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI);
		}
	}
	g_free (cResult);
}

void env_backend_shutdown (void)
{
	// since Gnome 3, gnome-session-save has been replaced by gnome-session-quit
	const gchar *args[] = {"which", "gnome-session-quit", NULL, NULL};
	gchar *cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
	if (cResult != NULL && *cResult == '/')
	{
		args[2] = "--power-off";
		cairo_dock_launch_command_argv_full (args + 1, NULL, GLDI_LAUNCH_GUI); // i.e. gnome-session-quit --power-off
	}
	else
	{
		g_free (cResult);
		args[1] = "cinnamon-session-quit";
		// Cinnamon?
		cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
		if (cResult != NULL && *cResult == '/')
		{
			args[2] = "--power-off";
			cairo_dock_launch_command_argv_full (args + 1, NULL, GLDI_LAUNCH_GUI);
		}
		else
		{
			args[0] = "gnome-session-save";
			args[1] = "--shutdown-dialog";
			cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI);
		}
	}
	g_free (cResult);
}

void env_backend_lock_screen (void)
{
	cairo_dock_launch_command_single (MY_APPLET_SHARE_DATA_DIR"/../shared-files/scripts/lock-screen.sh");
}

void env_backend_setup_time (void)
{
	static gboolean bChecked = FALSE;
	static const gchar *args[] = {NULL, NULL, NULL};
	if (!bChecked)
	{
		bChecked = TRUE;
		args[0] = "which";
		args[1] = "gnome-control-center";
		gchar *cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);  // Gnome3
		if (cResult != NULL && *cResult == '/')
		{
			args[0] = "gnome-control-center";
			args[1] = "datetime";
		}
		else
		{
			g_free (cResult);
			args[1] = "time-admin";
			cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);  // Gnome2
			if (cResult != NULL && *cResult == '/')
			{
				args[0] = "time-admin";
				args[1] = NULL;
			}
		}
		g_free (cResult);
	}
	if (args[0])
		cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	else
		cd_warning ("couldn't guess what program to use to setup the time and date.");
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("gnome-system-monitor");
}


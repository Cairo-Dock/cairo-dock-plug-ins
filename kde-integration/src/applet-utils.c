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

/*enum    KWorkSpace::ShutdownConfirm {
	KWorkSpace::ShutdownConfirmDefault = -1,
	KWorkSpace::ShutdownConfirmNo = 0,
	KWorkSpace::ShutdownConfirmYes = 1}

enum    KWorkSpace::ShutdownType {
	KWorkSpace::ShutdownTypeDefault = -1,
	KWorkSpace::ShutdownTypeNone = 0,
	KWorkSpace::ShutdownTypeReboot = 1,
	KWorkSpace::ShutdownTypeHalt = 2,
	KWorkSpace::ShutdownTypeLogout = 3}

enum    KWorkSpace::ShutdownMode {
	KWorkSpace::ShutdownModeDefault = -1,
	KWorkSpace::ShutdownModeSchedule = 0,
	KWorkSpace::ShutdownModeTryNow = 1,
	KWorkSpace::ShutdownModeForceNow = 2,
	KWorkSpace::ShutdownModeInteractive = 3}*/

static const gchar *logout_args[] = {"qdbus", "org.kde.ksmserver", "/KSMServer", "logout",
	// ShutdownConfirm; ShutdownType; ShutdownMode
	"1", "1", "-1", NULL};
// usr/bin/dbus-send --session --type=method_call --dest=org.kde.ksmserver /KSMServer org.kde.KSMServerInterface.logout int32:1 int32:2 int32:0

void env_backend_logout (void)
{
	logout_args[5] = "3";
	cairo_dock_launch_command_argv (logout_args);
}

void env_backend_shutdown (void)
{
	logout_args[5] = "2"; // or should we display other options too? => ShutdownTypeDefault?
	cairo_dock_launch_command_argv (logout_args);
}

void env_backend_reboot (void)
{
	logout_args[5] = "1";
	cairo_dock_launch_command_argv (logout_args);
}

void env_backend_lock_screen (void)
{
	const char * const args[] = {"qdbus", "org.freedesktop.ScreenSaver", "/ScreenSaver", "Lock", NULL};
	cairo_dock_launch_command_argv_full (args, NULL, TRUE);
}

void env_backend_setup_time (void)
{
	char *kcmshell = g_strdup_printf ("kcmshell%d clock", get_kde_version());
	const char * const args[] = {kcmshell, "clock", NULL};
	cairo_dock_launch_command_argv_full (args, NULL, TRUE);
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("ksysguard");
}

int get_kde_version (void)
{
	static int s_iKdeVersion = 0;
	if (s_iKdeVersion == 0)
	{
		const gchar *args[] = {"plasmashell", "--version", NULL};
		gchar *version = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);  // KDE5 or above
		if (! version)
		{
			args[0] = "plasma-desktop";
			version = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);  // KDE4
		}
		if (version)
		{
			gchar *major = version;
			while (! g_ascii_isdigit(*major) && *major != '\0')
				major++;
			s_iKdeVersion = atoi(major);
		}
		
		if (! s_iKdeVersion)  // the commands above didn't work
			s_iKdeVersion = 5;  // KDE5 by default
		cd_debug ("KDE version detected: %d\n", s_iKdeVersion);
		g_free (version);
	}
	return s_iKdeVersion;
}

const gchar *get_kioclient_number (void)
{
	static gchar *s_sNumber = NULL;
	if (! s_sNumber)
	{
		if (get_kde_version() < 5)
			s_sNumber = "";
		else
			s_sNumber = g_strdup_printf("%d", get_kde_version());
	}
	return s_sNumber;
}

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

/* KDE 5 and 6:
 * org.kde.LogoutPrompt well-known name, not running by default (cannot detect easily if it will work), but activatable
 * object: /LogoutPrompt
 * interface: org.kde.LogoutPrompt
 * methods: promptLogout, promptShutDown, promptReboot
 */

void env_backend_logout (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	GDBusConnection *pConn = cairo_dock_dbus_get_session_bus ();
	if (!pConn) cd_warning ("No usable DBus connection, cannot trigger logout dialog");
	// note: this will trigger activating the required component
	else g_dbus_connection_call (pConn, "org.kde.LogoutPrompt", "/LogoutPrompt", "org.kde.LogoutPrompt",
		"promptLogout", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

void env_backend_shutdown (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	GDBusConnection *pConn = cairo_dock_dbus_get_session_bus ();
	if (!pConn) cd_warning ("No usable DBus connection, cannot trigger logout dialog");
	// note: this will trigger activating the required component
	else g_dbus_connection_call (pConn, "org.kde.LogoutPrompt", "/LogoutPrompt", "org.kde.LogoutPrompt",
		"promptShutDown", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

void env_backend_reboot (G_GNUC_UNUSED CairoDockFMConfirmationFunc cb_confirm, G_GNUC_UNUSED gpointer data)
{
	GDBusConnection *pConn = cairo_dock_dbus_get_session_bus ();
	if (!pConn) cd_warning ("No usable DBus connection, cannot trigger logout dialog");
	// note: this will trigger activating the required component
	else g_dbus_connection_call (pConn, "org.kde.LogoutPrompt", "/LogoutPrompt", "org.kde.LogoutPrompt",
		"promptReboot", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

void env_backend_lock_screen (void)
{
	GDBusConnection *pConn = cairo_dock_dbus_get_session_bus ();
	if (!pConn) cd_warning ("No usable DBus connection, cannot trigger screensaver");
	else g_dbus_connection_call (pConn, "org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver",
		"Lock", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}

void env_backend_setup_time (void)
{
	char *kcmshell = g_strdup_printf ("kcmshell%d", get_kde_version());
	const char * const args[] = {kcmshell, "clock", NULL};
	cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	g_free (kcmshell);
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("plasma-systemmonitor");
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
		if (get_kde_version() == 5)
			s_sNumber = "5";
		else
			s_sNumber = "";
	}
	return s_sNumber;
}

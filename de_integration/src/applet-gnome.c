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
#include <glib.h>
#include <gio/gio.h>

#include "applet-init.h"

/*
 * Notes on GNOME vs. Cinnamon:
 * Both have org.gnome.SessionManager with the same functionality for logout, shutdown, reboot, etc.
 * The only difference is in the GSettings schema for the logout confirmation setting.
 */

static GDBusProxy *s_pSessionProxy = NULL;
static GCancellable *s_pCancel = NULL; // cancellable for creating the above proxy
/* based on org.gnome.SessionManager.SessionName property if available (not available on MATE)
 * Full logic:
 * 		s_bIsCinnamon == TRUE => Cinnamon
 * 		s_bIsCinnamon == FALSE => GNOME or MATE, check myData.iCurrentEnv
 */
static gboolean s_bIsCinnamon = FALSE;
static GSettings *s_pSessionSettings = NULL;

static gboolean _need_confirmation (void)
{
	if (!s_pSessionSettings) return TRUE;
	
	return ! g_settings_get_boolean (s_pSessionSettings, "logout-prompt");
}


static void _logout_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Logout", g_variant_new ("(u)", 0), // 0 -- normal
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _env_backend_logout (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm && _need_confirmation ())
		cb_confirm (data, _logout_real);
	else _logout_real ();
}

static void _shutdown_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Shutdown", NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _env_backend_shutdown (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm && _need_confirmation ())
		cb_confirm (data, _shutdown_real);
	else _shutdown_real ();
}

static void _reboot_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy,
		(myData.iCurrentEnv == CAIRO_DOCK_MATE) ? "RequestReboot" : "Reboot",
		NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}
static void _env_backend_reboot (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm && (_need_confirmation () || myData.iCurrentEnv == CAIRO_DOCK_MATE)) // MATE never asks for confirmation for RequestReboot
		cb_confirm (data, _reboot_real);
	else _reboot_real ();
}


static void _env_backend_setup_time (void)
{
	const gchar * args[] = {NULL, NULL, NULL};
	if (s_bIsCinnamon)
	{
		args[0] = "cinnamon-settings";
		args[1] = "calendar";
	}
	else
	{
		if (myData.iCurrentEnv == CAIRO_DOCK_MATE)
			args[0] = "mate-time-admin";
		else // GNOME
		{
			args[0] = "gnome-control-center";
			args[1] = "datetime";
		}
	}
	
	cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
}

static void _env_backend_show_system_monitor (void)
{
	//!! TODO: is there a Cinnamon variant?
	cairo_dock_launch_command_single_gui ("gnome-system-monitor");
}

static gboolean s_bRegistered = FALSE;

static void _register_backend (void)
{
	if (s_bRegistered) return;
	s_bRegistered = TRUE;
	
	CairoDockDesktopEnvBackend VFSBackend = { NULL };
	
	if (s_pSessionProxy)
	{
		VFSBackend.logout              = _env_backend_logout;
		VFSBackend.shutdown            = _env_backend_shutdown;
		VFSBackend.reboot              = _env_backend_reboot;
	}
	VFSBackend.setup_time          = _env_backend_setup_time;
	if (myData.iCurrentEnv == CAIRO_DOCK_GNOME)
		VFSBackend.show_system_monitor = _env_backend_show_system_monitor;
	
	cairo_dock_fm_register_de_integration_override_backend (&VFSBackend);
}

static void _proxy_connected (G_GNUC_UNUSED GObject *obj, GAsyncResult *res, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	
	s_pSessionProxy = g_dbus_proxy_new_finish (res, &erreur);
	if (erreur)
	{
		cd_warning ("Error creating gnome-session proxy: %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	else 
	{
		GVariant *prop = g_dbus_proxy_get_cached_property (s_pSessionProxy, "SessionName");
		if (prop)
		{
			if (g_variant_is_of_type (prop, G_VARIANT_TYPE ("s")))
			{
				gsize len = 0;
				const gchar *name = g_variant_get_string (prop, &len);
				if (len == 8 && ! strncmp (name, "cinnamon", 8))
					s_bIsCinnamon = TRUE;
			}
			g_variant_unref (prop);
		}
		
		// set up our settings proxy
		const char *cSchema;
		if (s_bIsCinnamon) cSchema = "org.cinnamon.SessionManager";
		else if (myData.iCurrentEnv == CAIRO_DOCK_MATE) cSchema = "org.mate.session";
		else cSchema = "org.gnome.SessionManager";
		GSettingsSchema *schema = g_settings_schema_source_lookup (
			g_settings_schema_source_get_default (),
			cSchema,
			TRUE);
		if (schema)
		{
			if (s_pSessionSettings) g_object_unref (s_pSessionSettings); // should not happen
			s_pSessionSettings = g_settings_new_full (schema, NULL, NULL);
			g_settings_schema_unref (schema);
		}
		else cd_warning ("Cannot find GSettings schema for the session manager");
	}
	_register_backend ();
	
	CD_APPLET_LEAVE ();
}

static void _on_name_appeared (GDBusConnection *connection, G_GNUC_UNUSED const gchar *name,
	G_GNUC_UNUSED const gchar *name_owner, G_GNUC_UNUSED gpointer data)
{
	g_return_if_fail (s_pSessionProxy == NULL && s_pCancel == NULL);
	
	CD_APPLET_ENTER;
	
	s_pCancel = g_cancellable_new ();
	g_dbus_proxy_new (connection,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
		NULL, // GDBusInterfaceInfo -- we could supply this, but work anyway
		"org.gnome.SessionManager",
		"/org/gnome/SessionManager",
		"org.gnome.SessionManager",
		s_pCancel, // GCancellable
		_proxy_connected,
		NULL);
	
	CD_APPLET_LEAVE ();
}

static void _on_name_vanished (G_GNUC_UNUSED GDBusConnection *connection, G_GNUC_UNUSED const gchar *name, G_GNUC_UNUSED gpointer user_data)
{
	CD_APPLET_ENTER;
	
	if (s_pCancel || s_pSessionProxy) cd_warning ("gnome-session proxy disappeared");
	else
	{
		// this can happen in other DEs if XDG_CURRENT_DESKTOP includes GNOME or if the user requested GNOME
		cd_message ("Cannot connect to session manager, not a GNOME, MATE or Cinnamon session?");
		// we should still register the supported functions
		_register_backend ();
	}
	cd_gnome_integration_stop (); // to clean up
	
	CD_APPLET_RELOAD_CONFIG_PANEL; // in case the config panel is being displayed (e.g. the user just selected GNOME)
	
	CD_APPLET_LEAVE ();
}


void cd_gnome_integration_init (void)
{
	// Note: org.gnome.SessionManager is used by both GNOME and Cinnamon
	g_bus_watch_name (G_BUS_TYPE_SESSION, "org.gnome.SessionManager", G_BUS_NAME_WATCHER_FLAGS_NONE,
		_on_name_appeared, _on_name_vanished, NULL, NULL);
}

void cd_gnome_integration_stop (void)
{
	if (s_pCancel)
	{
		g_cancellable_cancel (s_pCancel);
		g_object_unref (s_pCancel);
		s_pCancel = NULL;
	}
	if (s_pSessionProxy)
	{
		g_object_unref (s_pSessionProxy);
		s_pSessionProxy = NULL;
	}
	if (s_pSessionSettings)
	{
		g_object_unref (s_pSessionSettings);
		s_pSessionSettings = NULL;
	}
	
	s_bRegistered = FALSE; // the caller will unregister if needed
}

const char *cd_gnome_integration_get_status (void)
{
	return s_pSessionProxy ? NULL : "Error: cannot connect to org.gnome.SessionManager";
}


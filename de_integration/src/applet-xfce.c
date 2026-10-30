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

static GDBusProxy *s_pSessionProxy = NULL;
static GCancellable *s_pCancel = NULL; // cancellable for creating the above proxy
static unsigned int s_uWatch = 0; // ID for watching the bus name

typedef enum {
	CD_XFCE_CAN_SHUTDOWN=0,
	CD_XFCE_CAN_REBOOT,
	CD_XFCE_CAN_SUSPEND,
	CD_XFCE_CAN_HIBERNATE,
	CD_XFCE_CAN_HYBRIDSLEEP,
	CD_XFCE_NB_CAP
	} CDXfceCapType;

static gboolean s_bCapabilities[CD_XFCE_NB_CAP] = { FALSE };
static gboolean s_bCapChecked[CD_XFCE_NB_CAP] = { FALSE };

static void _logout_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Logout", g_variant_new ("(bb)", FALSE, TRUE), // don't show the dialog, allow save
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_logout (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm) cb_confirm (data, _logout_real);
	else _logout_real ();
}

static void _shutdown_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Shutdown", g_variant_new ("(b)", TRUE), // allow save
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_shutdown (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm) cb_confirm (data, _shutdown_real);
	else _shutdown_real ();
}

static void _reboot_real (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Restart", g_variant_new ("(b)", TRUE), // allow save
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}
static void _xfce_reboot (CairoDockFMConfirmationFunc cb_confirm, gpointer data)
{
	if (! s_pSessionProxy) return;
	if (cb_confirm) cb_confirm (data, _reboot_real);
	else _reboot_real ();
}

static void _xfce_suspend (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Suspend", NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_hibernate (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Hibernate", NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_hybridsleep (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "HybridSleep", NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_lock_screen (void)
{
	if (! s_pSessionProxy) return;
	
	g_dbus_proxy_call (s_pSessionProxy, "Lock", NULL, // no parameters
		G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout (-1 : default)
		NULL, // GCancellable
		NULL, // callback
		NULL // callback data
	);
}

static void _xfce_show_system_monitor (void)
{
	cairo_dock_launch_command_single_gui ("xfce4-taskmanager");  // not installed by default
}



static gboolean s_bRegistered = FALSE;

static void _register_backend (void)
{
	if (s_bRegistered) return;
	s_bRegistered = TRUE;
	
	CairoDockDesktopEnvBackend VFSBackend = { NULL };
	
	if (s_pSessionProxy)
	{
		if (s_bCapabilities[CD_XFCE_CAN_SHUTDOWN])    VFSBackend.shutdown     = _xfce_shutdown;
		if (s_bCapabilities[CD_XFCE_CAN_REBOOT])      VFSBackend.reboot       = _xfce_reboot;
		if (s_bCapabilities[CD_XFCE_CAN_SUSPEND])     VFSBackend.suspend      = _xfce_suspend;
		if (s_bCapabilities[CD_XFCE_CAN_HIBERNATE])   VFSBackend.hibernate    = _xfce_hibernate;
		if (s_bCapabilities[CD_XFCE_CAN_HYBRIDSLEEP]) VFSBackend.hybrid_sleep = _xfce_hybridsleep;
		VFSBackend.lock_screen = _xfce_lock_screen; // no CanLock method
		VFSBackend.logout      = _xfce_logout; // also no CanLogout
	}
	VFSBackend.show_system_monitor = _xfce_show_system_monitor;
	
	cairo_dock_fm_register_de_integration_override_backend (&VFSBackend);
}

static void _cap_check_cb (GObject *obj, GAsyncResult *res, gpointer data)
{
	CD_APPLET_ENTER;
	int cap_ix = GPOINTER_TO_INT (data);
	GError *err = NULL;
	GVariant *x = g_dbus_proxy_call_finish (G_DBUS_PROXY (obj), res, &err);
	if (!x)
	{
		cd_warning ("Cannot get xfce-session capabilities: %s", err->message);
		g_error_free (err);
	}
	else
	{
		if (g_variant_is_of_type (x, G_VARIANT_TYPE ("(b)")))
			g_variant_get_child (x, 0, "b", s_bCapabilities + cap_ix);
		else cd_warning ("Unexpected return type from xfce-session capabilities: %s", g_variant_get_type_string (x));
		g_variant_unref (x);
	}
	s_bCapChecked[cap_ix] = TRUE; // in any case, we mark that we tried, no point in checking again later
	
	if (s_pSessionProxy)
	{
		// only register if we still have a proxy (_stop() was not called before)
		int i;
		gboolean all_checked = TRUE;
		for (i = 0; i < CD_XFCE_NB_CAP; i++) if (!s_bCapChecked[i])
		{
			all_checked = FALSE;
			break;
		}
		
		if (all_checked) _register_backend ();
	}
	
	CD_APPLET_LEAVE ();
}


static void _proxy_connected (G_GNUC_UNUSED GObject *obj, GAsyncResult *res, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	
	s_pSessionProxy = g_dbus_proxy_new_finish (res, &erreur);
	if (erreur)
	{
		cd_warning ("Error creating xfce-session proxy: %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	else if (!s_bCapChecked[0])
	{
		// check whether things are supported
		g_dbus_proxy_call (s_pSessionProxy, "CanShutdown", NULL, // no parameters
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL,
			_cap_check_cb, // callback
			GINT_TO_POINTER ((int)CD_XFCE_CAN_SHUTDOWN) // callback data
		);
		g_dbus_proxy_call (s_pSessionProxy, "CanRestart", NULL, // no parameters
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL,
			_cap_check_cb, // callback
			GINT_TO_POINTER ((int)CD_XFCE_CAN_REBOOT) // callback data
		);
		g_dbus_proxy_call (s_pSessionProxy, "CanSuspend", NULL, // no parameters
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL,
			_cap_check_cb, // callback
			GINT_TO_POINTER ((int)CD_XFCE_CAN_SUSPEND) // callback data
		);
		g_dbus_proxy_call (s_pSessionProxy, "CanHibernate", NULL, // no parameters
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL,
			_cap_check_cb, // callback
			GINT_TO_POINTER ((int)CD_XFCE_CAN_HIBERNATE) // callback data
		);
		g_dbus_proxy_call (s_pSessionProxy, "CanHybridSleep", NULL, // no parameters
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL,
			_cap_check_cb, // callback
			GINT_TO_POINTER ((int)CD_XFCE_CAN_HYBRIDSLEEP) // callback data
		);
	}
	else _register_backend (); // already checked, can register now
	
	CD_APPLET_LEAVE ();
}

static void _on_name_appeared (GDBusConnection *connection, G_GNUC_UNUSED const gchar *name,
	G_GNUC_UNUSED const gchar *name_owner, G_GNUC_UNUSED gpointer data)
{
	cd_return_if_fail (s_pSessionProxy == NULL && s_pCancel == NULL);
	
	CD_APPLET_ENTER;
	
	s_pCancel = g_cancellable_new ();
	g_dbus_proxy_new (connection,
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
		NULL, // GDBusInterfaceInfo -- we could supply this, but works anyway
		"org.xfce.SessionManager",
		"/org/xfce/SessionManager",
		"org.xfce.Session.Manager",
		s_pCancel, // GCancellable
		_proxy_connected,
		NULL);
	
	CD_APPLET_LEAVE ();
}

static void _on_name_vanished (G_GNUC_UNUSED GDBusConnection *connection, G_GNUC_UNUSED const gchar *name, G_GNUC_UNUSED gpointer user_data)
{
	CD_APPLET_ENTER;
	
	if (s_pCancel || s_pSessionProxy) cd_warning ("xfce-session proxy disappeared");
	else
	{
		// this can happen in other DEs if XDG_CURRENT_DESKTOP includes GNOME or if the user requested GNOME
		cd_message ("Cannot connect to session manager, not an Xfce session?");
		int i;
		for (i = 0; i < CD_XFCE_NB_CAP; i++)
		{
			s_bCapChecked[i] = FALSE; // we can try if the proxy appears later
			s_bCapabilities[i] = FALSE;
		}
		// we should still register the supported functions
		_register_backend ();
	}
	cd_xfce_integration_stop (); // to clean up
	
	CD_APPLET_RELOAD_CONFIG_PANEL; // in case the config panel is being displayed (e.g. the user just selected GNOME)
	
	CD_APPLET_LEAVE ();
}


void cd_xfce_integration_init (void)
{
	if (s_uWatch) return; // already started
	
	s_uWatch = g_bus_watch_name (G_BUS_TYPE_SESSION, "org.xfce.SessionManager", G_BUS_NAME_WATCHER_FLAGS_NONE,
		_on_name_appeared, _on_name_vanished, NULL, NULL);
}

void cd_xfce_integration_stop (void)
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
	if (s_uWatch)
	{
		g_bus_unwatch_name (s_uWatch);
		s_uWatch = 0;
	}
	s_bRegistered = FALSE; // the caller will unregister if needed
}

const char *cd_xfce_integration_get_status (void)
{
	return s_pSessionProxy ? NULL : "Error: cannot connect to org.xfce.SessionManager";
}


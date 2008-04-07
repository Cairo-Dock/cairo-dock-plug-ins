/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Necropotame (for any bug report, please mail me to adrien.pilleboue@gmail.com)

******************************************************************************/
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "applet-struct.h"
#include "applet-dbus-spec.h"
#include "applet-dbus.h"

CD_APPLET_INCLUDE_MY_VARS

static gboolean dbus_deskletVisible = FALSE;
static guint dbus_xLastActiveWindow;
static dbusCallback *server = NULL;

G_DEFINE_TYPE(dbusCallback, cd_dbus_callback, G_TYPE_OBJECT);


static void cd_dbus_callback_class_init(dbusCallbackClass *class)
{
	cd_message("");
	// Nothing here
}
static void cd_dbus_callback_init(dbusCallback *server)
{
	cd_message("");
	g_return_if_fail (server->connection == NULL);
	
	// Initialise the DBus connection
	server->connection = cairo_dock_get_dbus_connection ();
	
	dbus_g_object_type_install_info(cd_dbus_callback_get_type(), &dbus_glib_cd_dbus_callback_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/org/cairodock/CairoDock", G_OBJECT(server));

	// Register the service name
	cairo_dock_register_service_name ("org.cairodock.CairoDock");
}
void cd_dbus_launch_service (void)
{
	g_return_if_fail (server == NULL);
	cd_message("dbus : Lancement du service");
	server = g_object_new(cd_dbus_callback_get_type(), NULL);  // -> appelle cd_dbus_callback_class_init() et cd_dbus_callback_init().
}

void cd_dbus_stop_service (void)
{
	if (server != NULL)
		g_object_unref (server);
	server = NULL;
}



gboolean cd_dbus_callback_hello(dbusCallback *pDbusCallback, GError **error)
{
	cairo_dock_show_general_message("Hello !",3000);
	return TRUE;
}

gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error)
{
	if (! myConfig.bEnableDesklets)
		return FALSE;
	if (dbus_deskletVisible)
	{
		cairo_dock_set_desklets_visibility_to_default ();
		cairo_dock_show_appli (dbus_xLastActiveWindow);
	}
	else
	{
		dbus_xLastActiveWindow = cairo_dock_get_active_window ();
		cairo_dock_set_all_desklets_visible (widgetLayer != NULL ? *widgetLayer : FALSE);
	}
	dbus_deskletVisible = !dbus_deskletVisible;
	return TRUE;
}

gboolean cd_dbus_callback_show_dialog(dbusCallback *pDbusCallback, gchar *message, GError **error)
{
	if (! myConfig.bEnablePopUp)
		return FALSE;
	cairo_dock_show_general_message(message,3000);
	return TRUE;
}

gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableReboot)
		return FALSE;
	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);
	return TRUE;
}

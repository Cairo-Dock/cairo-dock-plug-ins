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
	g_type_init();
	
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
		cairo_dock_show_xwindow (dbus_xLastActiveWindow);
	}
	else
	{
		dbus_xLastActiveWindow = cairo_dock_get_current_active_window ();
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

gboolean cd_dbus_callback_reload_module (dbusCallback *pDbusCallback, gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableReloadModule)
		return FALSE;
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_val_if_fail (pModule != NULL, FALSE);
	
	cairo_dock_reload_module (pModule, TRUE);  // TRUE <=> reload module conf file.
	return TRUE;
}

gboolean cd_dbus_callback_show_dock (dbusCallback *pDbusCallback, gboolean bShow, GError **error)
{
	if (! myConfig.bEnableShowDock)
		return FALSE;
	
	if (bShow)
		cairo_dock_stop_quick_hide ();
	else
		cairo_dock_quick_hide_all_docks ();
	return TRUE;
}

gboolean cd_dbus_callback_load_launcher_from_file (dbusCallback *pDbusCallback, gchar *cDesktopFile, GError **error)
{
	if (! myConfig.bEnableLoadLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	Icon *pIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFile, pCairoContext);
	cairo_destroy (pCairoContext);
	
	return TRUE;
}

gboolean cd_dbus_callback_create_launcher_from_scratch (dbusCallback *pDbusCallback, gchar *cIconFile, gchar *cLabel, gchar *cCommand, gchar *cParentDockName, GError **error)
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	
	if (cParentDockName == NULL)
		cParentDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (cParentDockName);
	if (pParentDock == NULL)
		return FALSE;
	
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->iType = CAIRO_DOCK_LAUNCHER;
	pIcon->acFileName = g_strdup (cIconFile);
	pIcon->acName = g_strdup (cLabel);
	pIcon->acCommand = g_strdup (cCommand);
	pIcon->cParentDockName = g_strdup (cParentDockName);
	pIcon->acDesktopFileName = g_strdup ("none");
	pIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pParentDock));
	cairo_dock_fill_icon_buffers_for_dock (pIcon, pCairoContext, pParentDock);
	cairo_destroy (pCairoContext);
	
	cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, myIcons.bUseSeparator);
	
	return TRUE;
}

static void _find_icon_in_dock (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cIconName = data[0];
	gchar *cIconCommand = data[1];
	Icon **pFoundIcon = data[2];
	if ((cIconName == NULL || (pIcon->acName && strcmp (cIconName, pIcon->acName) == 0)) &&
		(cIconCommand == NULL || (pIcon->acCommand && strcmp (cIconCommand, pIcon->acCommand) == 0)))
	{
		*pFoundIcon = pIcon;
	}
}
Icon *cd_dbus_find_icon (gchar *cIconName, gchar *cIconCommand, gchar *cModuleName)
{
	Icon *pIcon = NULL;
	if (cModuleName != NULL)  // c'est une icone d'un des modules.
	{
		CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
		g_return_val_if_fail (pModule != NULL, FALSE);
		
		if (pModule->pInstancesList != NULL)
		{
			CairoDockModuleInstance *pModuleInstance = pModule->pInstancesList->data;
			if (pModuleInstance != NULL)
				pIcon = pModuleInstance->pIcon;
		}
	}
	else  // on cherche une icone de lanceur.
	{
		gpointer data[3];
		data[0] = cIconName;
		data[1] = cIconCommand;
		data[2] = &pIcon;
		cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_icon_in_dock, data);
	}
	return pIcon;
}
gboolean cd_dbus_callback_set_quick_info (dbusCallback *pDbusCallback, gchar *cQuickInfo, gchar *cIconName, gchar *cIconCommand, gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetQuickInfo)
		return FALSE;
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	if (pContainer == NULL)
		return FALSE;
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_quick_info (pCairoContext, cQuickInfo, pIcon, fMaxScale);
	cairo_destroy (pCairoContext);
	return TRUE;
}

gboolean cd_dbus_callback_set_label (dbusCallback *pDbusCallback, gchar *cLabel, gchar *cIconName, gchar *cIconCommand, gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetLabel)
		return FALSE;
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	if (pContainer == NULL)
		return FALSE;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_icon_name (pCairoContext, cLabel, pIcon, pContainer);
	cairo_destroy (pCairoContext);
	return TRUE;
}

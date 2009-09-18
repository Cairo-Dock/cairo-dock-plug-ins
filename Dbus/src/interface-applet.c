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

/******************************************************************************
exemples : 
----------

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/my_applet org.cairodock.CairoDock.applet.SetLabel string:new_label

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.AddDataRenderer string:gauge int32:2 string:Turbo-night-fuel

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.RenderValues array:double:.7,.2

******************************************************************************/

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet.h"
#include "dbus-applet-spec.h"

static guint s_iSignals[NB_SIGNALS] = { 0 };

G_DEFINE_TYPE(dbusApplet, cd_dbus_applet, G_TYPE_OBJECT);

static void _init_signals_once (dbusAppletClass *klass)
{
	static gboolean bFirst = TRUE;
	if (! bFirst)
		return;
	bFirst = FALSE;
	
	// on definit les signaux dont on aura besoin.
	s_iSignals[CLIC] =
		g_signal_new("on_click_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__INT,
				G_TYPE_NONE, 1, G_TYPE_INT);
	s_iSignals[MIDDLE_CLIC] =
		g_signal_new("on_middle_click_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[SCROLL] =
		g_signal_new("on_scroll_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__BOOLEAN,
				G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	s_iSignals[BUILD_MENU] =
		g_signal_new("on_build_menu",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[MENU_SELECT] =
		g_signal_new("on_menu_select",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__INT,
				G_TYPE_NONE, 1, G_TYPE_INT);
	s_iSignals[DROP_DATA] =
		g_signal_new("on_drop_data",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSignals[RELOAD_MODULE] =
		g_signal_new("on_reload_module",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__BOOLEAN,
				G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	s_iSignals[INIT_MODULE] =
		g_signal_new("on_init_module",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[STOP_MODULE] =
		g_signal_new("on_stop_module",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, G_TYPE_NONE);
	
	// Add signals
	DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
    if (pProxy != NULL)
    {
		dbus_g_proxy_add_signal(pProxy, "on_click_icon",
			G_TYPE_INT, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_middle_click_icon",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_scroll_icon",
			G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_build_menu",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_drop_data",
			G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_init_module",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_stop_module",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_reload_module",
			G_TYPE_BOOLEAN, G_TYPE_INVALID);
	}
}
static void cd_dbus_applet_class_init(dbusAppletClass *klass)
{
	cd_message("");
	
	_init_signals_once (klass);
}
static void cd_dbus_applet_init (dbusApplet *pDbusApplet)
{
	cd_message("");
	
	pDbusApplet->connection = cairo_dock_get_session_connection ();
	pDbusApplet->proxy = cairo_dock_get_main_proxy ();
	
	dbus_g_object_type_install_info (cd_dbus_applet_get_type(), &dbus_glib_cd_dbus_applet_object_info);
}



static dbusApplet * _get_dbus_applet (CairoDockModuleInstance *pModuleInstance)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	dbusApplet *pDbusApplet = NULL;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		if (strcmp (cModuleName, pDbusApplet->cModuleName) == 0)
			break ;
	}
	return (a ? pDbusApplet : NULL);
}

static dbusApplet * _remove_dbus_applet_from_list (CairoDockModuleInstance *pModuleInstance)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	dbusApplet *pDbusApplet;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		if (strcmp (cModuleName, pDbusApplet->cModuleName) == 0)
			break ;
	}
	myData.pAppletList = g_list_delete_link (myData.pAppletList, a);
	return (a ? pDbusApplet : NULL);
}

#define _applet_list_is_empty() (myData.pAppletList == NULL)

dbusApplet *cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	g_return_val_if_fail (pModuleInstance != NULL && myData.pMainObject != NULL, NULL);  // l'interface principale a deja enregistre notre domaine, etc.
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	g_print ("%s (%s)\n", __func__, cModuleName);
	
	/// tester l'unicite...
	dbusApplet *server = _get_dbus_applet (pModuleInstance);
	if (server != NULL)
	{
		cd_warning ("this applet (%s) already has a remote object on the bus", cModuleName);
		return server;
	}
	
	// on cree l'objet DBus correspondant a l'applet.
	server = g_object_new (cd_dbus_applet_get_type(), NULL);  // appelle cd_dbus_applet_class_init() et cd_dbus_applet_init().
	server->cModuleName = g_strdup (cModuleName);
	server->pModuleInstance = pModuleInstance;
	
	// on l'enregistre sous un chemin propre a l'applet.
	gchar *cPath = g_strconcat ("/org/cairodock/CairoDock/", cModuleName, NULL);
	dbus_g_connection_register_g_object(server->connection, cPath, G_OBJECT(server));
	g_free (cPath);
	
	// on s'abonne aux notifications qu'on voudra propager sur le bus.
	if (server->proxy != NULL && _applet_list_is_empty ())  // 1ere applet Dbus.
	{
		cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_SCROLL_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
			CAIRO_DOCK_RUN_FIRST,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
	}
	
	myData.pAppletList = g_list_prepend (myData.pAppletList, server);
	return server;
}

void cd_dbus_unregister_notifications (void)
{
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_MIDDLE_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCROLL_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
		NULL);
}

void cd_dbus_delete_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	dbusApplet *pDbusApplet = _remove_dbus_applet_from_list (pModuleInstance);
	
	if (_applet_list_is_empty ())
	{
		cd_dbus_unregister_notifications ();
	}
	
	if (pDbusApplet != NULL)
		g_object_unref (pDbusApplet);
}



gboolean cd_dbus_applet_is_used (const gchar *cModuleName)
{
	//g_print ("%s (%s in %s)\n", __func__, cModuleName, myData.cActiveModules);
	if (myData.cActiveModules == NULL)
		return FALSE;
	gchar *str = g_strstr_len (myData.cActiveModules, -1, cModuleName);
	return (str && (str[strlen(cModuleName)] == ';' || str[strlen(cModuleName)] == '\0'));
}

void cd_dbus_launch_distant_applet_in_dir (const gchar *cModuleName, const gchar *cDirPath)
{
	gchar *cCommand = g_strdup_printf ("cd \"%s/%s/%s\" && ./\"%s\"", cDirPath, "third-party", cModuleName, cModuleName);
	g_print ("on lance une applet distante : '%s'\n", cCommand);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}


void cd_dbus_launch_distant_applet (const gchar *cModuleName)
{
	gchar *cFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR"/third-party", cModuleName);
	if (g_file_test (cFilePath, G_FILE_TEST_EXISTS))
	{
		cd_dbus_launch_distant_applet_in_dir (cModuleName, MY_APPLET_SHARE_DATA_DIR);
	}
	else
	{
		g_free (cFilePath);
		cFilePath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "third-party", cModuleName);
		if (g_file_test (cFilePath, G_FILE_TEST_EXISTS))
		{
			cd_dbus_launch_distant_applet_in_dir (cModuleName, g_cCairoDockDataDir);
		}
	}
	g_free (cFilePath);
}

void cd_dbus_action_on_init_module (CairoDockModuleInstance *pModuleInstance)
{
	CairoDockVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
	if (pModuleInstance->pDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
			"Simple",
			NULL,
			CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET,
			(CairoDeskletRendererConfigPtr) NULL);
	}
	
	Icon *pIcon = pModuleInstance->pIcon;
	if (pIcon && pIcon->cFileName == NULL && pIcon->pIconBuffer)
	{
		cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
	}
}

void cd_dbus_emit_on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	dbusApplet *pDbusApplet = _get_dbus_applet (pModuleInstance);
	if (pDbusApplet == NULL)
		pDbusApplet = cd_dbus_create_remote_applet_object (pModuleInstance);
	
	g_signal_emit (pDbusApplet,
		s_iSignals[INIT_MODULE],
		0,
		NULL);
	
	cd_dbus_action_on_init_module (pModuleInstance);
	
	if (! cd_dbus_applet_is_used (pModuleInstance->pModule->pVisitCard->cModuleName))
	{
		gchar *str = myData.cActiveModules;
		if (myData.cActiveModules)
			myData.cActiveModules = g_strdup_printf ("%s;%s", myData.cActiveModules, pModuleInstance->pModule->pVisitCard->cModuleName);
		else
			myData.cActiveModules = g_strdup (pModuleInstance->pModule->pVisitCard->cModuleName);
		g_free (str);
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING, "Configuration", "modules", myData.cActiveModules,
			G_TYPE_INVALID);
	}
	
	if (pModuleInstance->pModule->fLastLoadingTime != -1)  // activation faite par le dock, on verifie que le programme est lance.
	{
		gchar *cCommand = g_strdup_printf ("pgrep -f \"./%s\"", pModuleInstance->pModule->pVisitCard->cModuleName);  // -f : match command line and not process name, which is limited to 15 characters; -x : match exactly.
		gchar *cResult = cairo_dock_launch_command_sync (cCommand);
		if (cResult != NULL)
		{
			g_print ("l'applet est deja lancee\n");
			g_free (cResult);
		}
		else
		{
			g_print ("l'applet '%s' n'est pas en cours d'execution (d'apers la commande '%s'\n", pModuleInstance->pModule->pVisitCard->cModuleName, cCommand);
			cd_dbus_launch_distant_applet (pModuleInstance->pModule->pVisitCard->cModuleName);
		}
		g_free (cCommand);
	}
}

void cd_dbus_action_on_stop_module (CairoDockModuleInstance *pModuleInstance)
{
	if (pModuleInstance->pIcon->pSubDock != NULL)
	{
		cairo_dock_destroy_dock (pModuleInstance->pIcon->pSubDock, pModuleInstance->pIcon->cName, NULL, NULL);
		pModuleInstance->pIcon->pSubDock = NULL;
	}
	
	cairo_dock_remove_data_renderer_on_icon (pModuleInstance->pIcon);
	
	if (pModuleInstance->pDesklet != NULL && pModuleInstance->pDesklet->icons != NULL)  // idem, version desklet.
	{
		g_list_foreach (pModuleInstance->pDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (pModuleInstance->pDesklet->icons);
		pModuleInstance->pDesklet->icons = NULL;
	}
}

void cd_dbus_emit_on_stop_module (CairoDockModuleInstance *pModuleInstance)
{
	g_print ("%s ()\n", __func__);
	dbusApplet *pDbusApplet = _get_dbus_applet (pModuleInstance);
	if (pDbusApplet != NULL)
		g_signal_emit (pDbusApplet,
			s_iSignals[STOP_MODULE],
			0,
			NULL);
		
	cd_dbus_action_on_stop_module (pModuleInstance);
	
	/// enlever le nom de myData.cActiveModules ...
	if (myData.cActiveModules != NULL)
	{
		gchar *str = g_strstr_len (myData.cActiveModules, -1, pModuleInstance->pModule->pVisitCard->cModuleName);
		if (str)
		{
			*str = '\0';
			gchar *ptr = myData.cActiveModules;
			myData.cActiveModules = g_strdup_printf ("%s%s", myData.cActiveModules, str + strlen (pModuleInstance->pModule->pVisitCard->cModuleName));
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_STRING, "Configuration", "modules", myData.cActiveModules,
				G_TYPE_INVALID);
			
			g_free (ptr);
		}
	}
	
	cd_dbus_delete_remote_applet_object (pModuleInstance);
}

gboolean cd_dbus_emit_on_reload_module (CairoDockModuleInstance *pModuleInstance, CairoContainer *pOldContainer, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	CairoDockVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
	dbusApplet *pDbusApplet = _get_dbus_applet (pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, FALSE);
	g_signal_emit (pDbusApplet,
		s_iSignals[RELOAD_MODULE],
		0,
		pKeyFile != NULL);
	
	if (pModuleInstance->pDesklet)
	{
		if (pModuleInstance->pDesklet->icons == NULL)
		{
			cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
				"Simple",
				NULL,
				CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET,
				(CairoDeskletRendererConfigPtr) NULL);
		}
		else
		{
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
				"Caroussel",
				NULL,
				CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET,
				(CairoDeskletRendererConfigPtr) data);
		}
	}
	
	Icon *pIcon = pModuleInstance->pIcon;
	if (pIcon && pIcon->cFileName == NULL && pIcon->pIconBuffer && pIcon->pDataRenderer == NULL)
	{
		cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
	}
	
	if (pKeyFile == NULL)
	{
		if (pIcon && pIcon->pDataRenderer != NULL)
		{
			cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
			cairo_dock_reload_data_renderer_on_icon (pIcon, pModuleInstance->pContainer, pDrawContext, NULL);
			
			CairoDataRenderer *pRenderer = pIcon->pDataRenderer;
			CairoDataToRenderer *pData = cairo_data_renderer_get_data (pRenderer);
			g_print ("actuellement %d valeurs dans l'historique\n", pData->iMemorySize);
			if (pData->iMemorySize > 2)
				cairo_dock_resize_data_renderer_history (pIcon, pIcon->fWidth);
			
			cairo_dock_refresh_data_renderer (pIcon, pModuleInstance->pContainer, pDrawContext);
			cairo_destroy (pDrawContext);
		}
	}
	
	return TRUE;
}



static inline CairoDockModuleInstance *_get_module_instance_from_dbus_applet (dbusApplet *pDbusApplet)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (pDbusApplet->cModuleName);
	g_return_val_if_fail (pModule != NULL && pModule->pInstancesList != NULL, NULL);
	
	return pModule->pInstancesList->data;
}

gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_quick_info (pCairoContext, cQuickInfo, pIcon, fMaxScale);
	cairo_destroy (pCairoContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}


gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_icon_name (pCairoContext, cLabel, pIcon, pContainer);
	cairo_destroy (pCairoContext);
	return TRUE;
}

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && cAnimation != NULL)
	{
		cairo_dock_request_icon_animation (pIcon, CAIRO_DOCK (pContainer), cAnimation, iNbRounds);
		return TRUE;
	}
	return FALSE;
}

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}

void cd_dbus_emit_on_menu_select (GtkMenuShell *menu, gpointer data)
{
	int iNumEntry = GPOINTER_TO_INT (data);
	g_signal_emit ((dbusApplet *)myData.pCurrentMenuDbusApplet, s_iSignals[MENU_SELECT], 0, iNumEntry);
}

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	if (myData.pModuleSubMenu == NULL)
	{
		cd_warning ("the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet");
		return FALSE;
	}
	
	if (pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet");
		return FALSE;
	}
	
	int i;
	for (i = 0; pLabels[i] != NULL; i ++)
	{
		if (*pLabels[i] == '\0')
		{
			gtk_menu_shell_append (GTK_MENU_SHELL (myData.pModuleSubMenu), gtk_separator_menu_item_new ());
		}
		else
		{
			cairo_dock_add_in_menu_with_stock_and_data (pLabels[i],
				NULL,
				(GFunc) cd_dbus_emit_on_menu_select,
				myData.pModuleSubMenu,
				GINT_TO_POINTER (i));
		}
	}
	gtk_widget_show_all (myData.pModuleSubMenu);
	
	return TRUE;
}


gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
	if (strcmp (cType, "gauge") == 0)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = cairo_dock_get_gauge_theme_path (cTheme);
	}
	else if (strcmp (cType, "gauge") == 0)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (pIcon->fWidth > 1 ? pIcon->fWidth : 32);  // fWidht peut etre <= 1 en mode desklet au chargement.
		// Line;Plain;Bar;Circle;Plain Circle
		if (cTheme == NULL || strcmp (cTheme, "Line") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_LINE;
		else if (strcmp (cTheme, "Plain") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_PLAIN;
		else if (strcmp (cTheme, "Bar") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_BAR;
		else if (strcmp (cTheme, "Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_CIRCLE;
		else if (strcmp (cTheme, "Plain Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_CIRCLE_PLAIN;
		attr.iRadius = 10;
		attr.bMixGraphs = FALSE;
		double *fHighColor = g_new (double, iNbValues*3);
		double *fLowColor = g_new (double, iNbValues*3);
		int i;
		for (i = 0; i < iNbValues; i ++)
		{
			fHighColor[3*i] = 1;
			fHighColor[3*i+1] = 0;
			fHighColor[3*i+2] = 0;
			fLowColor[3*i] = 0;
			fLowColor[3*i+1] = 1;
			fLowColor[3*i+2] = 1;
		}
		attr.fHighColor = fHighColor;
		attr.fLowColor = fLowColor;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 1;
		attr.fBackGroundColor[0] = .4;
	}
	else if (strcmp (cType, "bar") == 0)
	{
		/// A FAIRE...
	}
	
	if (pRenderAttr == NULL)
		return FALSE;
	
	pRenderAttr->iLatencyTime = 500;  // 1/2s
	pRenderAttr->iNbValues = iNbValues;
	//pRenderAttr->bUpdateMinMax = TRUE;
	//pRenderAttr->bWriteValues = TRUE;
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
	if (pIcon->pDataRenderer == NULL)
		cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pDrawContext, pRenderAttr);
	else
		cairo_dock_reload_data_renderer_on_icon (pIcon, pContainer, pDrawContext, pRenderAttr);
	cairo_destroy (pDrawContext);
	
	return TRUE;
}

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, myDrawContext, (double *)pValues->data);
	cairo_destroy (pDrawContext);
	
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}


gboolean cd_dbus_applet_add_sub_icons (dbusApplet *pDbusApplet, const gchar **pIconFields, GError **error)
{
	g_print ("%s ()\n", __func__);
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	GList *pCurrentIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pCurrentIconsList);
	int n = (pLastIcon ? pLastIcon->fOrder + 1 : 0);
	
	GList *pIconsList = NULL;
	Icon *pOneIcon;
	int i;
	for (i = 0; pIconFields[3*i] && pIconFields[3*i+1] && pIconFields[3*i+2]; i ++)
	{
		pOneIcon = g_new0 (Icon, 1);
		pOneIcon->cName = g_strdup (pIconFields[3*i]);
		pOneIcon->cFileName = g_strdup (pIconFields[3*i+1]);
		pOneIcon->fOrder = i + n;
		pOneIcon->fScale = 1.;
		pOneIcon->fAlpha = 1.;
		pOneIcon->fWidthFactor = 1.;
		pOneIcon->fHeightFactor = 1.;
		pOneIcon->cCommand = g_strdup (pIconFields[3*i+2]);
		pIconsList = g_list_append (pIconsList, pOneIcon);
	}
	if (pIconFields[3*i] != NULL)
	{
		cd_warning ("the number of argument is incorrect\nThis may result in an incorrect number of loaded icons.");
	}
	
	if (pInstance->pDock)
	{
		if (pIcon->pSubDock == NULL)
		{
			cairo_t *pDrawContext = cairo_dock_create_context_from_container (pContainer);
			if (pIcon->cName == NULL)
				cairo_dock_set_icon_name (pDrawContext, pInstance->pModule->pVisitCard->cModuleName, pIcon, pContainer);
			if (cairo_dock_check_unique_subdock_name (pIcon))
				cairo_dock_set_icon_name (pDrawContext, pIcon->cName, pIcon, pContainer);
			cairo_destroy (pDrawContext);
			pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconsList, pIcon->cName, pInstance->pDock);
			//cairo_dock_set_renderer (pIcon->pSubDock, cRenderer);
			cairo_dock_update_dock_size (pIcon->pSubDock);
		}
		else
		{
			GList *ic;
			for (ic = pIconsList; ic != NULL; ic = ic->next)
			{
				pOneIcon = ic->data;
				cairo_dock_load_one_icon_from_scratch (pOneIcon, CAIRO_CONTAINER (pIcon->pSubDock));
				cairo_dock_insert_icon_in_dock (pOneIcon, pIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
			}
			cairo_dock_update_dock_size (pIcon->pSubDock);
			g_list_free (pIconsList);
		}
	}
	else
	{
		if (pIcon->pSubDock != NULL)  // precaution.
		{
			cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName, NULL, NULL);
			pIcon->pSubDock = NULL;
		}
		pInstance->pDesklet->icons = g_list_concat (pInstance->pDesklet->icons, pIconsList);
		gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (pInstance->pDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, (CairoDeskletRendererConfigPtr) data);
	}
	
	return TRUE;
}

gboolean cd_dbus_applet_remove_sub_icon (dbusApplet *pDbusApplet, const gchar *cIconID, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (cIconID == NULL || strcmp (cIconID, "any") == 0)  // remove all
	{
		if (pInstance->pDesklet && pInstance->pDesklet->icons != NULL)
		{
			g_list_foreach (pInstance->pDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (pInstance->pDesklet->icons);
			pInstance->pDesklet->icons = NULL;
		}
		if (pIcon->pSubDock != NULL)
		{
			if (pInstance->pDesklet)
			{
				cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName, NULL, NULL);
				pIcon->pSubDock = NULL;
			}
			else
			{
				g_list_foreach (pIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
				g_list_free (pIcon->pSubDock->icons);
				pIcon->pSubDock->icons = NULL;
				pIcon->pSubDock->pFirstDrawnElement = NULL;
			}
		}
	}
	else
	{
		GList *pIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		Icon *pOneIcon = cairo_dock_get_icon_with_command (pIconsList, cIconID);
		if (pInstance->pDock)
		{
			cairo_dock_detach_icon_from_dock (pOneIcon, pIcon->pSubDock, FALSE);
			cairo_dock_update_dock_size (pIcon->pSubDock);
		}
		else
		{
			pInstance->pDesklet->icons = g_list_remove (pInstance->pDesklet->icons, pOneIcon);
			gtk_widget_queue_draw (pInstance->pDesklet->container.pWidget);
		}
		cairo_dock_free_icon (pOneIcon);
	}
	
	return TRUE;
}



#define CAIRO_DOCK_IS_MANUAL_APPLET(pIcon) (CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->pModule->cSoFilePath == NULL)

static inline Icon *_get_main_icon_from_clicked_icon (Icon *pIcon, CairoContainer *pContainer)
{
	Icon *pMainIcon = NULL;
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		pMainIcon = CAIRO_DESKLET (pContainer)->pIcon;
	}
	else if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		if (CAIRO_DOCK (pContainer)->iRefCount == 0)
		{
			pMainIcon = pIcon;
		}
		else
		{
			pMainIcon = cairo_dock_search_icon_pointing_on_dock (CAIRO_DOCK (pContainer), NULL);
		}
	}
	return pMainIcon;
}

gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iButtonState);
	dbusApplet *pDbusApplet = _get_dbus_applet (pAppletIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[CLIC], 0, iButtonState);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = _get_dbus_applet (pAppletIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[MIDDLE_CLIC], 0, NULL);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iDirection);
	dbusApplet *pDbusApplet = _get_dbus_applet (pAppletIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[SCROLL], 0, (iDirection == GDK_SCROLL_UP));
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

static void _delete_menu (GtkMenuShell *menu, CairoDockModuleInstance *myApplet)
{
	myData.pModuleSubMenu = NULL;
}
gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	myData.pModuleSubMenu = cairo_dock_create_sub_menu (pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
		pAppletMenu,
		pAppletIcon->pModuleInstance->pModule->pVisitCard->cIconFilePath);
	
	cairo_dock_add_in_menu_with_stock_and_data (_("About this applet"),
		GTK_STOCK_ABOUT,
		(GFunc) cairo_dock_pop_up_about_applet,
		myData.pModuleSubMenu,
		pAppletIcon->pModuleInstance);
	
	g_signal_connect (G_OBJECT (pAppletMenu),
		"deactivate",
		G_CALLBACK (_delete_menu),
		myApplet);
	
	g_print ("%s (%s)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = _get_dbus_applet (pAppletIcon->pModuleInstance);
	myData.pCurrentMenuDbusApplet = pDbusApplet;
	g_signal_emit (pDbusApplet, s_iSignals[BUILD_MENU], 0, NULL);
	return (pClickedIcon == pAppletIcon ? CAIRO_DOCK_LET_PASS_NOTIFICATION : CAIRO_DOCK_INTERCEPT_NOTIFICATION);
}

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cd_message (" %s --> sur le bus !", cReceivedData);
	dbusApplet *pDbusApplet = _get_dbus_applet (pAppletIcon->pModuleInstance);
	g_signal_emit (pDbusApplet, s_iSignals[DROP_DATA], 0, cReceivedData);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

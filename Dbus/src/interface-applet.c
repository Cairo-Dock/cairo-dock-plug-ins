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

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.CreateLauncherFromScratch string:inkscape string:yep string:rien string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetLabel string:new_label string:icon_name string:any string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:none string:none string:dustbin

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:default int32:2 string:any string:firefox string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetIcon string:firefox-3.0 string:any string:nautilus string:none

******************************************************************************/

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet.h"
#include "applet-dbus-applet-spec.h"

static guint s_iSignals[NB_SIGNALS] = { 0 };

G_DEFINE_TYPE(dbusApplet, cd_dbus_applet, G_TYPE_OBJECT);

/*static void g_cclosure_marshal_VOID__INT (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

static void g_cclosure_marshal_VOID__INT (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}*/

static void _init_signals_once (dbusAppletClass *klass)
{
	static gboolean bFirst = TRUE;
	if (! bFirst)
		return;
	bFirst = FALSE;
	
	// Enregistrement des marshaller specifique aux signaux.
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__INT,
		G_TYPE_NONE, G_TYPE_INT ,G_TYPE_INVALID);  // clic
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__BOOLEAN,
		G_TYPE_NONE, G_TYPE_BOOLEAN ,G_TYPE_INVALID);  // scroll
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, G_TYPE_STRING ,G_TYPE_INVALID);  // drop
	
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
	// Register the service name
	//cairo_dock_register_service_name ("org.cairodock.CairoDock.applet");
}

static dbusApplet * _get_dbus_applet (CairoDockModuleInstance *pModuleInstance)
{
	gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
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

static void cd_dbus_applet_class_init(dbusAppletClass *klass)
{
	cd_message("");
	
	_init_signals_once (klass);
}
static void cd_dbus_applet_init (dbusApplet *server)
{
	cd_message("");
	
	server->connection = cairo_dock_get_session_connection ();
	server->proxy = cairo_dock_get_main_proxy ();
	
	dbus_g_object_type_install_info(cd_dbus_applet_get_type(), &dbus_glib_cd_dbus_applet_object_info);
}

void cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	g_return_if_fail (pModuleInstance != NULL && myData.server != NULL);  // l'interface principale a deja enregistre notre domaine, etc.
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	
	// on cree l'objet DBus correspondant a l'applet.
	dbusApplet *server = g_object_new (cd_dbus_applet_get_type(), NULL);  // appelle cd_dbus_applet_class_init() et cd_dbus_applet_init().
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
	
	myData.pAppletList = g_list_prepend (myData.pAppletList, server);  /// tester l'unicite...
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

void cd_dbus_delete_remote_object (CairoDockModuleInstance *pModuleInstance)
{
	dbusApplet *pDbusApplet = _remove_dbus_applet_from_list (pModuleInstance);
	
	if (_applet_list_is_empty ())
	{
		cd_dbus_unregister_notifications ();
	}
	
	if (pDbusApplet != NULL)
		g_object_unref (pDbusApplet);
}


void cd_dbus_emit_on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	dbusApplet *pDbusApplet = _get_dbus_applet (pModuleInstance);
	if (pDbusApplet != NULL)
		g_signal_emit (pDbusApplet,
			s_iSignals[INIT_MODULE],
			0,
			NULL);
	
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
	if (pIcon && pIcon->acFileName == NULL && pIcon->pIconBuffer)
	{
		cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
	}
}

void cd_dbus_emit_on_stop_module (CairoDockModuleInstance *pModuleInstance)
{
	g_print ("%s ()\n", __func__);
	dbusApplet *pDbusApplet = _get_dbus_applet (pModuleInstance);
	g_return_if_fail (pDbusApplet != NULL);
	g_signal_emit (pDbusApplet,
		s_iSignals[STOP_MODULE],
		0,
		NULL);
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
		pVisitCard->cModuleName,
		pKeyFile != NULL);
	
	if (pModuleInstance->pDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
			"Simple",
			NULL,
			CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET,
			(CairoDeskletRendererConfigPtr) NULL);
	}
	
	Icon *pIcon = pModuleInstance->pIcon;
	if (pIcon && pIcon->acFileName == NULL && pIcon->pIconBuffer)
	{
		cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
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
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
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
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
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
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
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
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
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
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}

static void cd_dbus_emit_on_menu_select (GtkMenuShell *menu, gpointer data)
{
	int iNumEntry = GPOINTER_TO_INT (data);
	g_print ("%s (%s, %d)\n", __func__, ((dbusApplet *)myData.pCurrentMenuDbusApplet)->cModuleName, iNumEntry);
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
		g_print (" + %s\n", pLabels[1]);
		cairo_dock_add_in_menu_with_stock_and_data (pLabels[i],
			NULL,
			(GFunc) cd_dbus_emit_on_menu_select,
			myData.pModuleSubMenu,
			GINT_TO_POINTER (i));
	}
	gtk_widget_show_all (myData.pModuleSubMenu);
	
	return TRUE;
}



#define CAIRO_DOCK_IS_MANUAL_APPLET(pIcon) (CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->pModule->cSoFilePath == NULL)


gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iButtonState);
	dbusApplet *pDbusApplet = _get_dbus_applet (pClickedIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[CLIC], 0, iButtonState);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = _get_dbus_applet (pClickedIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[MIDDLE_CLIC], 0, NULL);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iDirection);
	dbusApplet *pDbusApplet = _get_dbus_applet (pClickedIcon->pModuleInstance);
	
	g_signal_emit (pDbusApplet, s_iSignals[SCROLL], 0, iDirection == GDK_SCROLL_UP);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

static void _delete_menu (GtkMenuShell *menu, CairoDockModuleInstance *myApplet)
{
	myData.pModuleSubMenu = NULL;
}
gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	myData.pModuleSubMenu = cairo_dock_create_sub_menu (pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
		pAppletMenu,
		pClickedIcon->pModuleInstance->pModule->pVisitCard->cIconFilePath);
	
	cairo_dock_add_in_menu_with_stock_and_data (_("About this applet"),
		GTK_STOCK_ABOUT,
		(GFunc) cairo_dock_pop_up_about_applet,
		myData.pModuleSubMenu,
		pClickedIcon->pModuleInstance);
	
	g_signal_connect (G_OBJECT (pAppletMenu),
		"deactivate",
		G_CALLBACK (_delete_menu),
		myApplet);
	
	g_print ("%s (%s)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = _get_dbus_applet (pClickedIcon->pModuleInstance);
	myData.pCurrentMenuDbusApplet = pDbusApplet;
	g_signal_emit (pDbusApplet, s_iSignals[BUILD_MENU], 0, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cd_message (" %s --> sur le bus !", cReceivedData);
	dbusApplet *pDbusApplet = _get_dbus_applet (pClickedIcon->pModuleInstance);
	g_signal_emit (pDbusApplet, s_iSignals[DROP_DATA], 0, cReceivedData);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

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

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"
#include "interface-applet-object.h"

static guint s_iSignals[NB_SIGNALS] = { 0 };
static guint s_iSubSignals[NB_SIGNALS] = { 0 };


static void g_cclosure_marshal_VOID__INT_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}
static void g_cclosure_marshal_VOID__BOOLEAN_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}
static void g_cclosure_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}

void cd_dbus_applet_init_signals_once (dbusAppletClass *klass)
{
	static gboolean bFirst = TRUE;
	if (! bFirst)
		return;
	bFirst = FALSE;
	
	// Enregistrement des marshaller specifique aux signaux.
	
	// on definit les signaux dont on aura besoin.
	s_iSignals[CLIC] =
		g_signal_new("on_click",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			g_cclosure_marshal_VOID__INT,
			G_TYPE_NONE, 1, G_TYPE_INT);
	s_iSignals[MIDDLE_CLIC] =
		g_signal_new("on_middle_click",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[SCROLL] =
		g_signal_new("on_scroll",
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
	s_iSignals[RELOAD_MODULE] =
		g_signal_new("on_reload_module",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	
	// Add signals
	DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
	if (pProxy != NULL)
	{
		dbus_g_proxy_add_signal(pProxy, "on_click",
			G_TYPE_INT, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_middle_click",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_scroll",
			G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_build_menu",
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_menu_select",
			G_TYPE_INT, G_TYPE_INVALID);
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

void cd_dbus_sub_applet_init_signals_once (dbusSubAppletClass *klass)
{
	static gboolean bFirst = TRUE;
	if (! bFirst)
		return;
	bFirst = FALSE;
	
	// Enregistrement des marshaller specifique aux signaux.
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__INT_STRING,
		G_TYPE_NONE, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);  // clic
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__BOOLEAN_STRING,
		G_TYPE_NONE, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INVALID);  // scroll
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);  // drop
	
	// on definit les signaux dont on aura besoin.
	s_iSubSignals[CLIC] =
		g_signal_new("on_click_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__INT_STRING,
				G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
	s_iSubSignals[MIDDLE_CLIC] =
		g_signal_new("on_middle_click_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSubSignals[SCROLL] =
		g_signal_new("on_scroll_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__BOOLEAN_STRING,
				G_TYPE_NONE, 2, G_TYPE_BOOLEAN, G_TYPE_STRING);
	s_iSubSignals[BUILD_MENU] =
		g_signal_new("on_build_menu_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSubSignals[MENU_SELECT] =
		g_signal_new("on_menu_select_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__INT_STRING,
				G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
	s_iSubSignals[DROP_DATA] =
		g_signal_new("on_drop_data_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING_STRING,
				G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
	
	// Add signals
	DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
	if (pProxy != NULL)
	{
		dbus_g_proxy_add_signal(pProxy, "on_click_sub_icon",
			G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_middle_click_icon",
			G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_scroll_sub_icon",
			G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_build_menu_sub_icon",
			G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_menu_select_sub_icon",
			G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_drop_data_sub_icon",
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	}
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
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	if (pClickedIcon == pAppletIcon)
	{
		g_print ("emit clic on main icon\n");
		g_signal_emit (pDbusApplet, s_iSignals[CLIC], 0, iButtonState);
	}
	else if (pDbusApplet->pSubApplet != NULL)
	{
		g_print ("emit clic on sub icon\n");
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[CLIC], 0, iButtonState, pClickedIcon->cCommand);
	}
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[MIDDLE_CLIC], 0, NULL);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[MIDDLE_CLIC], 0, pClickedIcon->cCommand);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iDirection);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[SCROLL], 0, (iDirection == GDK_SCROLL_UP));
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[SCROLL], 0, (iDirection == GDK_SCROLL_UP), pClickedIcon->cCommand);
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
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	myData.pCurrentMenuDbusApplet = pDbusApplet;
	myData.pCurrentMenuIcon = pClickedIcon;
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[BUILD_MENU], 0);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[BUILD_MENU], 0, pClickedIcon->cCommand);
	return (pClickedIcon == pAppletIcon ? CAIRO_DOCK_LET_PASS_NOTIFICATION : CAIRO_DOCK_INTERCEPT_NOTIFICATION);
}

void cd_dbus_emit_on_menu_select (GtkMenuShell *menu, gpointer data)
{
	g_return_if_fail (myData.pCurrentMenuIcon != NULL);
	int iNumEntry = GPOINTER_TO_INT (data);
	gchar *cIconID = myData.pCurrentMenuIcon->cCommand;  // NULL si c'est l'icone principale.
	if (cIconID == NULL)
		g_signal_emit (myData.pCurrentMenuDbusApplet, s_iSignals[MENU_SELECT], 0, iNumEntry);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (myData.pCurrentMenuDbusApplet->pSubApplet, s_iSubSignals[MENU_SELECT], 0, iNumEntry, cIconID);
}

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer)
{
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pAppletIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cd_message (" %s --> sur le bus !", cReceivedData);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[DROP_DATA], 0, cReceivedData);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[DROP_DATA], 0, cReceivedData, pClickedIcon->cCommand);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
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

void cd_dbus_emit_init_signal (CairoDockModuleInstance *pModuleInstance)
{
	g_print ("%s ()\n", __func__);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet == NULL)
		pDbusApplet = cd_dbus_create_remote_applet_object (pModuleInstance);
	
	g_signal_emit (pDbusApplet,
		s_iSignals[INIT_MODULE],
		0,
		NULL);
}

void cd_dbus_emit_on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	cd_dbus_emit_init_signal (pModuleInstance);
	
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
	
	if (pModuleInstance->pModule->fLastLoadingTime != -1)  // le registering a ete fait auparavant, donc l'applet n'est probablement pas lancee.
	{
		cd_dbus_launch_distant_applet_in_dir (pModuleInstance->pModule->pVisitCard->cModuleName, pModuleInstance->pModule->pVisitCard->cShareDataDir);
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
	g_print ("%s (%s)\n", __func__, pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet != NULL)
		g_signal_emit (pDbusApplet,
			s_iSignals[STOP_MODULE],
			0,
			NULL);
		
	cd_dbus_action_on_stop_module (pModuleInstance);
	
	// on enleve le nom de myData.cActiveModules si c'est une desactivation par l'utilisateur (plutot que le plug-in DBus qui s'arrete).
	if (myData.cActiveModules != NULL && ! myData.bServiceIsStopping)
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
	
	if (! myData.bServiceIsStopping)
		cd_dbus_delete_remote_applet_object (pModuleInstance);
}

gboolean cd_dbus_emit_on_reload_module (CairoDockModuleInstance *pModuleInstance, CairoContainer *pOldContainer, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	CairoDockVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
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

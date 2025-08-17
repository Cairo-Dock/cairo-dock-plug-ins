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
#include <implementations/cairo-dock-wayland-manager.h> // gldi_wayland_manager_have_layer_shell

#include "applet-dbus.h"
#include "interface-applet-methods.h"
#include "interface-applet-object.h"
#include "interface-applet-signals.h"

static guint s_iSignals[NB_SIGNALS] = { 0 };
static guint s_iSubSignals[NB_SIGNALS] = { 0 };


void cd_dbus_applet_init_signals_once (dbusAppletClass *klass)
{
	static gboolean bFirst = TRUE;
	if (! bFirst)
		return;
	bFirst = FALSE;
	
	// Enregistrement des marshaller specifique aux signaux.
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__VALUE,
		G_TYPE_NONE, G_TYPE_VALUE, G_TYPE_INVALID);  // answer
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__INT_VALUE,
		G_TYPE_NONE, G_TYPE_INT, G_TYPE_VALUE, G_TYPE_INVALID);  // answer_dialog
	
	// on definit les signaux dont on aura besoin.
	s_iSignals[CLIC] =
		g_signal_new("on_click",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__INT,
			G_TYPE_NONE, 1, G_TYPE_INT);
	s_iSignals[MIDDLE_CLIC] =
		g_signal_new("on_middle_click",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__VOID,
			G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[SCROLL] =
		g_signal_new("on_scroll",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__BOOLEAN,
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	s_iSignals[BUILD_MENU] =
		g_signal_new("on_build_menu",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__VOID,
			G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[MENU_SELECT] =
		g_signal_new("on_menu_select",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__INT,
			G_TYPE_NONE, 1, G_TYPE_INT);
	s_iSignals[DROP_DATA] =
		g_signal_new("on_drop_data",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__STRING,
			G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSignals[CHANGE_FOCUS] =
		g_signal_new("on_change_focus",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__BOOLEAN,
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	s_iSignals[ANSWER] =
		g_signal_new("on_answer",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__VALUE,
			G_TYPE_NONE, 1, G_TYPE_VALUE);  // deprecated
	s_iSignals[ANSWER_DIALOG] =
		g_signal_new("on_answer_dialog",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__INT_VALUE,
			G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_VALUE);
	s_iSignals[SHORTKEY] =
		g_signal_new("on_shortkey",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__STRING,
			G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSignals[INIT_MODULE] =
		g_signal_new("on_init_module",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__VOID,
			G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[STOP_MODULE] =
		g_signal_new("on_stop_module",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__VOID,
			G_TYPE_NONE, 0, G_TYPE_NONE);
	s_iSignals[RELOAD_MODULE] =
		g_signal_new("on_reload_module",
			G_OBJECT_CLASS_TYPE(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
			0,
			NULL, NULL,
			cd_dbus_marshal_VOID__BOOLEAN,
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
		dbus_g_proxy_add_signal(pProxy, "on_change_focus",
			G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_answer",
			G_TYPE_VALUE, G_TYPE_INVALID);  // deprecated
		dbus_g_proxy_add_signal(pProxy, "on_answer_dialog",
			G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_shortkey",
			G_TYPE_VALUE, G_TYPE_INVALID);
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
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__INT_STRING,
		G_TYPE_NONE, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);  // clic
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__BOOLEAN_STRING,
		G_TYPE_NONE, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INVALID);  // scroll
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);  // drop
	dbus_g_object_register_marshaller(cd_dbus_marshal_VOID__VALUE_STRING,
		G_TYPE_NONE, G_TYPE_VALUE, G_TYPE_STRING, G_TYPE_INVALID);  // answer
	
	// on definit les signaux dont on aura besoin.
	s_iSubSignals[CLIC] =
		g_signal_new("on_click_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				cd_dbus_marshal_VOID__INT_STRING,
				G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
	s_iSubSignals[MIDDLE_CLIC] =
		g_signal_new("on_middle_click_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				cd_dbus_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSubSignals[SCROLL] =
		g_signal_new("on_scroll_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				cd_dbus_marshal_VOID__BOOLEAN_STRING,
				G_TYPE_NONE, 2, G_TYPE_BOOLEAN, G_TYPE_STRING);
	s_iSubSignals[BUILD_MENU] =
		g_signal_new("on_build_menu_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				cd_dbus_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	s_iSubSignals[DROP_DATA] =
		g_signal_new("on_drop_data_sub_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				cd_dbus_marshal_VOID__STRING_STRING,
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
		dbus_g_proxy_add_signal(pProxy, "on_drop_data_sub_icon",
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	}
}

#define CAIRO_DOCK_IS_EXTERNAL_APPLET(pIcon) (CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->pModule->pInterface->stopModule == cd_dbus_emit_on_stop_module)

static inline Icon *_get_main_icon_from_clicked_icon (Icon *pIcon, GldiContainer *pContainer)
{
	Icon *pMainIcon = NULL;
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		pMainIcon = CAIRO_DESKLET (pContainer)->pIcon;
	}
	else if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		if (CAIRO_DOCK (pContainer)->iRefCount == 0 || CAIRO_DOCK_IS_APPLET (pIcon))  // gere donc le cas ou l'applet est placee dans un sous-dock.
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

gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, guint iButtonState)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	//g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iButtonState);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
	{
		//g_print ("emit clic on main icon\n");
		g_signal_emit (pDbusApplet, s_iSignals[CLIC], 0, iButtonState);
	}
	else if (pDbusApplet->pSubApplet != NULL)
	{
		//g_print ("emit clic on sub icon\n");
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[CLIC], 0, iButtonState, pClickedIcon->cCommand);
	}
	
	// if the applet acts as a launcher, assume it launches the program it controls on click
	// Note: if one day it poses a problem, we can either make a new attribute, or add a dbus method (or even reuse the "Animate" method with a pseudo "launching" animation).
	if (pAppletIcon->pModuleInstance->pModule->pVisitCard->bActAsLauncher
	&& pClickedIcon->pAppli == NULL)  // if the icon already controls a window, don't notify; most probably, the action the applet will take is to show/minimize this window
		gldi_class_startup_notify (pClickedIcon);
	return GLDI_NOTIFICATION_INTERCEPT;
}

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	//g_print ("%s (%s)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[MIDDLE_CLIC], 0, NULL);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[MIDDLE_CLIC], 0, pClickedIcon->cCommand);
	return GLDI_NOTIFICATION_INTERCEPT;
}

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, int iDirection)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	//g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iDirection);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[SCROLL], 0, (iDirection == GDK_SCROLL_UP));
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[SCROLL], 0, (iDirection == GDK_SCROLL_UP), pClickedIcon->cCommand);
	return GLDI_NOTIFICATION_INTERCEPT;
}

static void _delete_menu (GtkMenuShell *menu, GldiModuleInstance *myApplet)
{
	///myData.pModuleSubMenu = NULL;
	myData.pModuleMainMenu = NULL;
}
gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, GtkWidget *pAppletMenu)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	myData.pModuleMainMenu = pAppletMenu;
	/**myData.pModuleSubMenu = cairo_dock_create_sub_menu (pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
		pAppletMenu,
		pAppletIcon->pModuleInstance->pModule->pVisitCard->cIconFilePath);
	
	cairo_dock_add_in_menu_with_stock_and_data (_("Applet's Handbook"),
		GLDI_ICON_NAME_ABOUT,
		G_CALLBACK (cairo_dock_pop_up_about_applet),
		myData.pModuleSubMenu,
		pAppletIcon->pModuleInstance);*/
	
	g_signal_connect (G_OBJECT (pAppletMenu),
		"deactivate",
		G_CALLBACK (_delete_menu),
		myApplet);
	
	//g_print ("%s (%s)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
	myData.pCurrentMenuDbusApplet = pDbusApplet;
	
	GList *pChildren = gtk_container_get_children (GTK_CONTAINER (pAppletMenu));
	myData.iMenuPosition = g_list_length (pChildren);
	g_list_free (pChildren);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[BUILD_MENU], 0);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[BUILD_MENU], 0, pClickedIcon->cCommand);
	return (pClickedIcon == pAppletIcon ? GLDI_NOTIFICATION_LET_PASS : GLDI_NOTIFICATION_INTERCEPT);
}

void cd_dbus_emit_on_menu_select (GtkMenuItem *pMenuItem, gpointer data)
{
	g_return_if_fail (myData.pCurrentMenuDbusApplet != NULL);
	if (GTK_IS_RADIO_MENU_ITEM (pMenuItem))
	{
		if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
			return ;
	}
	
	// need to explicitly disable the tooltip on Wayland to avoid a race condition,
	// see: https://github.com/wmww/gtk-layer-shell/issues/207
	if (gldi_wayland_manager_have_layer_shell ())
		gtk_widget_set_tooltip_text (GTK_WIDGET (pMenuItem), NULL);
	int iNumEntry = GPOINTER_TO_INT (data);
	g_signal_emit (myData.pCurrentMenuDbusApplet, s_iSignals[MENU_SELECT], 0, iNumEntry);  // since there can only be 1 menu at once, and the applet knows when the menu is raised, there is no need to pass the icon in the signal: the applet can remember the clicked icon when it received the 'build-menu' event.
}

gboolean cd_dbus_applet_emit_on_drop_data (G_GNUC_UNUSED gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, GldiContainer *pClickedContainer)
{
	//\________________ On gere le cas d'une applet tierce-partie en provenance de nos depots.
	if (cReceivedData && strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz") && (g_strstr_len (cReceivedData, -1, "glxdock") || g_strstr_len (cReceivedData, -1, "glx-dock")))
	{
		//\________________ On telecharge l'archive de l'applet et on l'installe/la met a jour.
		GError *erreur = NULL;
		//g_print ("dropped a distant applet\n");
		gchar *cExtractTo = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CD_DBUS_APPLETS_FOLDER);
		gchar *cAppletDirPath = cairo_dock_download_archive (cReceivedData, cExtractTo);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}
		if (cAppletDirPath == NULL)
		{
			gldi_dialog_show_general_message (D_("Sorry, this module couldn't be added."), 10000);
		}
		else
		{
			//\________________ On la supprime totalement si elle existe deja (mise a jour).
			gchar *cAppletName = g_path_get_basename (cAppletDirPath);
			gchar *str = strchr (cAppletName, '_');
			if (str)  // on enleve les numeros de version (launchpad, solution temporaire).
			{
				if (g_ascii_isdigit (*(str+1)))
					*str = '\0';
			}
			
			GldiModule *pModule = gldi_module_get (cAppletName);
			gboolean bUpdate = FALSE;
			if (pModule != NULL)  // on va totalement supprimer le module pour le recharger de zero.
			{
				bUpdate = TRUE;
				gldi_object_unref (GLDI_OBJECT(pModule));
				pModule = NULL;
			}
			
			//\________________ On l'enregistre et on la (re)lance.
			cd_dbus_register_module_in_dir (cAppletName, cExtractTo);
			pModule = gldi_module_get (cAppletName);
			
			gldi_module_activate (pModule);
			
			//\________________ On balance un petit message a l'utilisateur.
			if (!pModule)
			{
				gldi_dialog_show_general_message (D_("Sorry, this module couldn't be added."), 10000);
			}
			else if (!pModule->pInstancesList)
			{
				gldi_dialog_show_general_message (D_("The module has been added, but couldn't be launched."), 10000);
			}
			else
			{
				GldiModuleInstance *pInstance = pModule->pInstancesList->data;
				if (!pInstance->pIcon || !pInstance->pContainer)
					gldi_dialog_show_general_message (D_("The module has been added, but couldn't be launched."), 10000);
				else
				{
					gldi_dialog_show_temporary_with_icon_printf (bUpdate ? D_("The applet '%s' has been succefully updated and automatically reloaded") : D_("The applet '%s' has been succefully installed and automatically launched"),
						pInstance->pIcon,
						pInstance->pContainer,
						10000,
						"same icon",
						cAppletName);
				}
			}
			g_free (cAppletName);
		}
		g_free (cExtractTo);
		return GLDI_NOTIFICATION_INTERCEPT;
	}
	
	//\________________ Sinon on notifie l'applet du drop.
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	cd_debug (" %s --> sur le bus !", cReceivedData);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance);
	g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_signal_emit (pDbusApplet, s_iSignals[DROP_DATA], 0, cReceivedData);
	else if (pDbusApplet->pSubApplet != NULL)
		g_signal_emit (pDbusApplet->pSubApplet, s_iSubSignals[DROP_DATA], 0, cReceivedData, pClickedIcon->cCommand);
	return GLDI_NOTIFICATION_INTERCEPT;
}

static gboolean _on_window_destroyed (G_GNUC_UNUSED gpointer data, GldiWindowActor *actor)
{
	if (actor == myData.pActiveWindow)
		myData.pActiveWindow = NULL;
	return GLDI_NOTIFICATION_LET_PASS;
}
gboolean cd_dbus_applet_emit_on_change_focus (G_GNUC_UNUSED gpointer data, GldiWindowActor *pNewActiveWindow)
{
	// on emet le signal sur l'icone qui avait le focus.
	if (myData.pActiveWindow != NULL)
	{
		Icon *pPrevActiveIcon = cairo_dock_get_appli_icon (myData.pActiveWindow);
		if (pPrevActiveIcon && pPrevActiveIcon->cParentDockName == NULL)
			pPrevActiveIcon = cairo_dock_get_inhibitor (pPrevActiveIcon, FALSE);
		if (CAIRO_DOCK_IS_EXTERNAL_APPLET (pPrevActiveIcon))
		{
			dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pPrevActiveIcon->pModuleInstance);
			g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
			g_signal_emit (pDbusApplet, s_iSignals[CHANGE_FOCUS], 0, FALSE);
		}
	}
	
	// on emet le signal sur l'icone qui a desormais le focus.
	//g_print ("DBUS : new active window : %ld\n", *xNewActiveWindow);
	if (pNewActiveWindow != NULL)
	{
		Icon *pNewActiveIcon = cairo_dock_get_appli_icon (pNewActiveWindow);
		if (pNewActiveIcon && pNewActiveIcon->cParentDockName == NULL)
			pNewActiveIcon = cairo_dock_get_inhibitor (pNewActiveIcon, FALSE);
		if (CAIRO_DOCK_IS_EXTERNAL_APPLET (pNewActiveIcon))
		{
			dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pNewActiveIcon->pModuleInstance);
			g_return_val_if_fail (pDbusApplet != NULL, GLDI_NOTIFICATION_LET_PASS);
			g_signal_emit (pDbusApplet, s_iSignals[CHANGE_FOCUS], 0, TRUE);
		}
	}
	
	myData.pActiveWindow = pNewActiveWindow;
	if (pNewActiveWindow)
		gldi_object_register_notification (pNewActiveWindow, NOTIFICATION_DESTROY, (GldiNotificationFunc)_on_window_destroyed, GLDI_RUN_AFTER, NULL);
	return GLDI_NOTIFICATION_LET_PASS;
}


// deprecated
static inline void _emit_answer (dbusApplet *pDbusApplet, CairoDialog *pDialog, GValue *v)
{
	g_signal_emit (pDbusApplet, s_iSignals[ANSWER], 0, v);  // same remark as for the menu: 1 dialog at once, the applet knows when the dialog is raised, so it can just remember the clicked icon; we don't need to tell it.
	
	pDbusApplet->pDialog = NULL;
}
void cd_dbus_applet_emit_on_answer_question (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	gboolean bYes = (iClickedButton == 0 || iClickedButton == -1);
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_BOOLEAN);
	g_value_set_boolean (&v, bYes);
	
	_emit_answer (pDbusApplet, pDialog, &v);
}

void cd_dbus_applet_emit_on_answer_value (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	double fValue = (iClickedButton == 0 || iClickedButton == -1 ? gtk_range_get_value (GTK_RANGE (pInteractiveWidget)) : -1);
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_DOUBLE);
	g_value_set_double (&v, fValue);
	
	_emit_answer (pDbusApplet, pDialog, &v);
}

void cd_dbus_applet_emit_on_answer_text (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	const gchar *cAnswer = (iClickedButton == 0 || iClickedButton == -1 ? gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget)) : NULL);
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_STRING);
	g_value_set_string (&v, cAnswer);
	
	_emit_answer (pDbusApplet, pDialog, &v);
}
// end of deprecated


static inline void _emit_answer_dialog (dbusApplet *pDbusApplet, CairoDialog *pDialog, int iClickedButton, GValue *v)
{
	g_signal_emit (pDbusApplet, s_iSignals[ANSWER_DIALOG], 0, iClickedButton, v);  // same remark as for the menu: 1 dialog at once, the applet knows when the dialog is raised, so it can just remember the clicked icon; we don't need to tell it.
	
	pDbusApplet->pDialog = NULL;
}
void cd_dbus_applet_emit_on_answer_buttons (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_INT);
	g_value_set_int (&v, iClickedButton);
	
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
}

void cd_dbus_applet_emit_on_answer_text_entry (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_STRING);
	
	GtkWidget *pEntry = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pEntry != NULL);
	/**GtkWidget *pEntry;
	if (GTK_IS_ENTRY (pInteractiveWidget))
	{
		pEntry = pInteractiveWidget;
	}
	else
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (pInteractiveWidget));
		g_return_if_fail (children != NULL);
		pEntry = children->data;
	}*/
	
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	//g_print (" -> %s\n", cText);
	g_value_set_string (&v, cText);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
}

void cd_dbus_applet_emit_on_answer_text_view (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_STRING);
	
	GtkWidget *pTextView = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pTextView != NULL);
	/**if (GTK_IS_TEXT_VIEW (pInteractiveWidget))
	{
		pTextView = pInteractiveWidget;
	}
	else
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (pInteractiveWidget));
		g_return_if_fail (children != NULL);
		pTextView = children->data;
	}*/
	GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pTextView));
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter (pBuffer, &start);
	gtk_text_buffer_get_end_iter (pBuffer, &end);
	gchar *cText = gtk_text_buffer_get_text (pBuffer,
		&start,
		&end,
		FALSE);
	
	g_value_set_string (&v, cText);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
	g_free (cText);
}

void cd_dbus_applet_emit_on_answer_scale (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_DOUBLE);
	
	GtkWidget *pScale = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pScale != NULL);
	/**GtkWidget *pScale;
	if (GTK_IS_RANGE (pInteractiveWidget))
	{
		pScale = pInteractiveWidget;
	}
	else
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (pInteractiveWidget));
		g_return_if_fail (children != NULL && children->next != NULL);
		pScale = children->next->data;
	}*/
	
	double x = gtk_range_get_value (GTK_RANGE (pScale));
	g_value_set_double (&v, x);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
}

void cd_dbus_applet_emit_on_answer_combo_entry (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_STRING);
	
	GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pInteractiveWidget));
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	
	g_value_set_string (&v, cText);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
}

void cd_dbus_applet_emit_on_answer_combo (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog)
{
	GValue v = G_VALUE_INIT;
	g_value_init (&v, G_TYPE_INT);
	
	int iSelectedItem = gtk_combo_box_get_active (GTK_COMBO_BOX (pInteractiveWidget));
	
	g_value_set_int (&v, iSelectedItem);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, &v);
}

void cd_dbus_applet_emit_on_shortkey (const gchar *cShortkey, dbusApplet *pDbusApplet)
{
	g_signal_emit (pDbusApplet, s_iSignals[SHORTKEY], 0, cShortkey);
}


void cd_dbus_action_on_init_module (GldiModuleInstance *pModuleInstance)
{
	GldiVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
	if (pModuleInstance->pDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
			"Simple",
			(CairoDeskletRendererConfigPtr) NULL);
	}
	
	Icon *pIcon = pModuleInstance->pIcon;
	if (pIcon && pIcon->cFileName == NULL && pIcon->image.pSurface)
	{
		cairo_t *pDrawContext = cairo_create (pIcon->image.pSurface);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
	}
}


void cd_dbus_action_on_stop_module (GldiModuleInstance *pModuleInstance)
{
	if (pModuleInstance->pIcon->pSubDock != NULL)
	{
		gldi_object_unref (GLDI_OBJECT(pModuleInstance->pIcon->pSubDock));
		pModuleInstance->pIcon->pSubDock = NULL;
	}
	
	cairo_dock_remove_data_renderer_on_icon (pModuleInstance->pIcon);
	
	if (pModuleInstance->pDesklet != NULL && pModuleInstance->pDesklet->icons != NULL)  // idem, version desklet.
	{
		g_list_foreach (pModuleInstance->pDesklet->icons, (GFunc) gldi_object_unref, NULL);
		g_list_free (pModuleInstance->pDesklet->icons);
		pModuleInstance->pDesklet->icons = NULL;
	}
}

void cd_dbus_emit_on_stop_module (GldiModuleInstance *pModuleInstance)
{
	//g_print ("%s (%s)\n", __func__, pModuleInstance->pModule->pVisitCard->cModuleName);
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet != NULL)
		g_signal_emit (pDbusApplet,
			s_iSignals[STOP_MODULE],
			0,
			NULL);
		
	cd_dbus_action_on_stop_module (pModuleInstance);
	
	cd_dbus_delete_remote_applet_object (pDbusApplet);
}

gboolean cd_dbus_emit_on_reload_module (GldiModuleInstance *pModuleInstance, GldiContainer *pOldContainer, GKeyFile *pKeyFile)
{
	//g_print ("%s ()\n", __func__);
	GldiVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
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
				(CairoDeskletRendererConfigPtr) NULL);
		}
		else
		{
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			cairo_dock_set_desklet_renderer_by_name (pModuleInstance->pDesklet,
				"Caroussel",
				(CairoDeskletRendererConfigPtr) data);
		}
	}
	
	Icon *pIcon = pModuleInstance->pIcon;
	if (pIcon && pIcon->cFileName == NULL && pIcon->image.pSurface && (pIcon->pDataRenderer == NULL || pIcon->pDataRenderer->bUseOverlay))
	{
		cairo_t *pDrawContext = cairo_create (pIcon->image.pSurface);
		cairo_dock_set_image_on_icon (pDrawContext, pVisitCard->cIconFilePath, pIcon, pModuleInstance->pContainer);
		cairo_destroy (pDrawContext);
		gtk_widget_queue_draw (pModuleInstance->pContainer->pWidget);
	}
	
	if (pKeyFile == NULL)
	{
		CairoDataRenderer *pDataRenderer = cairo_dock_get_icon_data_renderer (pIcon);
		if (pDataRenderer != NULL)
		{
			CairoDataToRenderer *pData = cairo_data_renderer_get_data (pDataRenderer);
			if (pData->iMemorySize > 2)
				cairo_dock_resize_data_renderer_history (pIcon, pIcon->fWidth);
		}
	}
	
	return TRUE;
}

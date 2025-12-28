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
#include <implementations/cairo-dock-wayland-manager.h> // gldi_wayland_manager_have_layer_shell

#include "applet-dbus.h"
#include "interface-applet-methods.h"
#include "interface-applet-object.h"
#include "interface-applet-signals.h"

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
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance); // note: this never returns NULL
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
			"on_click", g_variant_new ("(i)", iButtonState), NULL); // note: parameter GVariant is consumed with _ref_sink ()
	else g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPathSub, "org.cairodock.CairoDock.subapplet",
			"on_click_sub_icon", g_variant_new ("(is)", iButtonState, pClickedIcon->cCommand), NULL);
	
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
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance); // note: this never returns NULL
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
			"on_middle_click", NULL, NULL);
	else g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPathSub, "org.cairodock.CairoDock.subapplet",
			"on_middle_click_sub_icon", g_variant_new ("(s)", pClickedIcon->cCommand), NULL);
	return GLDI_NOTIFICATION_INTERCEPT;
}

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer,
	int iDirection, G_GNUC_UNUSED gboolean bEmulated)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	Icon *pAppletIcon = _get_main_icon_from_clicked_icon (pClickedIcon, pClickedContainer);
	if (! CAIRO_DOCK_IS_EXTERNAL_APPLET (pAppletIcon))
		return GLDI_NOTIFICATION_LET_PASS;
	
	//g_print ("%s (%s, %d)\n", __func__, pAppletIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iDirection);
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance); // note: this never returns NULL
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
			"on_scroll", g_variant_new ("(b)", (iDirection == GDK_SCROLL_UP)), NULL);
	else g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPathSub, "org.cairodock.CairoDock.subapplet",
			"on_scroll_sub_icon", g_variant_new ("(bs)", (iDirection == GDK_SCROLL_UP), pClickedIcon->cCommand), NULL);
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
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance); // note: this never returns NULL
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
	myData.pCurrentMenuDbusApplet = pDbusApplet;
	
	GList *pChildren = gtk_container_get_children (GTK_CONTAINER (pAppletMenu));
	myData.iMenuPosition = g_list_length (pChildren);
	g_list_free (pChildren);
	
	if (pClickedIcon == pAppletIcon)
		g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
			"on_build_menu", NULL, NULL);
	else g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPathSub, "org.cairodock.CairoDock.subapplet",
			"on_build_menu_sub_icon", g_variant_new ("(s)", pClickedIcon->cCommand), NULL);
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
	g_dbus_connection_emit_signal (myData.pCurrentMenuDbusApplet->connection, NULL, myData.pCurrentMenuDbusApplet->cBusPath,
		"org.cairodock.CairoDock.applet", "on_menu_select", g_variant_new ("(i)", iNumEntry), NULL);
}

gboolean cd_dbus_applet_emit_on_drop_data (G_GNUC_UNUSED gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, GldiContainer *pClickedContainer)
{
	//\________________ On gere le cas d'une applet tierce-partie en provenance de nos depots.
	//!! TODO: update this for applets in the Github repository !!
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
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pAppletIcon->pModuleInstance); // note: this never returns NULL
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
	
	if (pClickedIcon == pAppletIcon)
		g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
			"on_drop_data", g_variant_new ("(s)", cReceivedData), NULL);
	else g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPathSub, "org.cairodock.CairoDock.subapplet",
			"on_drop_data_sub_icon", g_variant_new ("(ss)", cReceivedData, pClickedIcon->cCommand), NULL);
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
			DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pPrevActiveIcon->pModuleInstance); // note: this never returns NULL
			g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
			g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
				"on_change_focus", g_variant_new ("(b)", FALSE), NULL);
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
			DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pNewActiveIcon->pModuleInstance); // note: this never returns NULL
			g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, GLDI_NOTIFICATION_LET_PASS);
			g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
				"on_change_focus", g_variant_new ("(b)", TRUE), NULL);
		}
	}
	
	myData.pActiveWindow = pNewActiveWindow;
	if (pNewActiveWindow)
		gldi_object_register_notification (pNewActiveWindow, NOTIFICATION_DESTROY, (GldiNotificationFunc)_on_window_destroyed, GLDI_RUN_AFTER, NULL);
	return GLDI_NOTIFICATION_LET_PASS;
}


// deprecated
static inline void _emit_answer (DBusAppletData *pDbusApplet, CairoDialog *pDialog, GVariant *v)
{
	g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
		"on_answer", g_variant_new ("(v)", v), NULL);
	
	pDbusApplet->pDialog = NULL;
}
void cd_dbus_applet_emit_on_answer_question (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	gboolean bYes = (iClickedButton == 0 || iClickedButton == -1);
	_emit_answer (pDbusApplet, pDialog, g_variant_new_boolean (bYes));
}

void cd_dbus_applet_emit_on_answer_value (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	double fValue = (iClickedButton == 0 || iClickedButton == -1 ? gtk_range_get_value (GTK_RANGE (pInteractiveWidget)) : -1);
	_emit_answer (pDbusApplet, pDialog, g_variant_new_double (fValue));
}

void cd_dbus_applet_emit_on_answer_text (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	const gchar *cAnswer = (iClickedButton == 0 || iClickedButton == -1 ? gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget)) : "");
	_emit_answer (pDbusApplet, pDialog, g_variant_new_string (cAnswer));
}
// end of deprecated


static inline void _emit_answer_dialog (DBusAppletData *pDbusApplet, CairoDialog *pDialog, int iClickedButton, GVariant *v)
{
	// same remark as for the menu: 1 dialog at once, the applet knows when the dialog is raised, so it can just remember the clicked icon; we don't need to tell it.
	g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
		"on_answer_dialog", g_variant_new ("(iv)", iClickedButton, v), NULL);
	
	pDbusApplet->pDialog = NULL;
}
void cd_dbus_applet_emit_on_answer_buttons (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, g_variant_new_int32 (iClickedButton));
}

void cd_dbus_applet_emit_on_answer_text_entry (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	GtkWidget *pEntry = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pEntry != NULL);
	
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, g_variant_new_string (cText ? cText : ""));
}

void cd_dbus_applet_emit_on_answer_text_view (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	GtkWidget *pTextView = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pTextView != NULL);
	
	GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pTextView));
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter (pBuffer, &start);
	gtk_text_buffer_get_end_iter (pBuffer, &end);
	gchar *cText = gtk_text_buffer_get_text (pBuffer,
		&start,
		&end,
		FALSE);
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton,
		cText ? g_variant_new_take_string (cText) : g_variant_new_string (""));
}

void cd_dbus_applet_emit_on_answer_scale (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	GtkWidget *pScale = g_object_get_data (G_OBJECT (pInteractiveWidget), "cd-widget");
	g_return_if_fail (pScale != NULL);
	
	double x = gtk_range_get_value (GTK_RANGE (pScale));
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, g_variant_new_double (x));
}

void cd_dbus_applet_emit_on_answer_combo_entry (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pInteractiveWidget));
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, g_variant_new_string (cText ? cText : ""));
}

void cd_dbus_applet_emit_on_answer_combo (int iClickedButton, GtkWidget *pInteractiveWidget, DBusAppletData *pDbusApplet, CairoDialog *pDialog)
{
	int iSelectedItem = gtk_combo_box_get_active (GTK_COMBO_BOX (pInteractiveWidget));
	_emit_answer_dialog (pDbusApplet, pDialog, iClickedButton, g_variant_new_int32 (iSelectedItem));
}

void cd_dbus_applet_emit_on_shortkey (const gchar *cShortkey, DBusAppletData *pDbusApplet)
{
	g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet",
		"on_shortkey", g_variant_new ("(s)", cShortkey), NULL);
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
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet->uRegApplet != 0) g_dbus_connection_emit_signal (pDbusApplet->connection, NULL,
		pDbusApplet->cBusPath, "org.cairodock.CairoDock.applet", "on_stop_module", NULL, NULL);
		
	cd_dbus_action_on_stop_module (pModuleInstance);
	
	cd_dbus_delete_remote_applet_object (pDbusApplet);
	
	g_free (pDbusApplet->cBusPath);
	g_free (pDbusApplet->cBusPathSub);
	g_free (pDbusApplet->cModuleName);
}

gboolean cd_dbus_emit_on_reload_module (GldiModuleInstance *pModuleInstance, GldiContainer *pOldContainer, GKeyFile *pKeyFile)
{
	//g_print ("%s ()\n", __func__);
	GldiVisitCard *pVisitCard = pModuleInstance->pModule->pVisitCard;
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	g_return_val_if_fail (pDbusApplet->uRegApplet != 0 && pDbusApplet->uRegSubApplet != 0, FALSE);
	g_dbus_connection_emit_signal (pDbusApplet->connection, NULL, pDbusApplet->cBusPath,
		"org.cairodock.CairoDock.applet", "on_reload_module", 
		g_variant_new ("(b)", (pKeyFile != NULL)), NULL);
	
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


/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/

#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-compiz.h"



static void _compiz_get_version (void) {
	if (myData.iCompizMajor != 0 || myData.iCompizMinor != 0 || myData.iCompizMicro != 0)
		return ;
	
	gchar *cCommand = g_strdup_printf ("compiz.real --version | awk '{print $2}'");
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	
	if (cResult == NULL) {
		cd_warning ("couldn't guess Compiz's version");
		return ;
	}
	cairo_dock_get_version_from_string (cResult, &myData.iCompizMajor, &myData.iCompizMinor, &myData.iCompizMicro);
	g_free (cResult);
	
	cd_message ("Compiz : %d.%d.%d", myData.iCompizMajor, myData.iCompizMinor, myData.iCompizMicro);
}

static void _compiz_dbus_action (const gchar *cCommand) {
	if (! cairo_dock_dbus_detect_application ("org.freedesktop.compiz"))
		cd_warning  ("Dbus plug-in must be activated in Compiz !");
	GError *erreur = NULL;
	gchar *cDbusCommand = g_strdup_printf ("dbus-send --type=method_call --dest=org.freedesktop.compiz /org/freedesktop/compiz/%s org.freedesktop.compiz.activate string:'root' int32:%d", cCommand, cairo_dock_get_root_id ());
	g_spawn_command_line_async (cDbusCommand, &erreur);
	g_free (cDbusCommand);
	if (erreur != NULL) {
		cd_warning ("Compiz-icon : when trying to send '%s' : %s", cCommand, erreur->message);
		g_error_free (erreur);
	}
}

static void _compiz_menu_show_desktop (void) {
	if (myData.iCompizMajor > 0 || (myData.iCompizMajor == 0 && myData.iCompizMinor > 6))
		_compiz_dbus_action ("core/allscreens/show_desktop_button");  /// A verifier ...
	else
		_compiz_dbus_action ("core/allscreens/show_desktop");
}

static void _compiz_menu_activate_expo (void) {
	_compiz_get_version ();
	if (myData.iCompizMajor > 0 || (myData.iCompizMajor == 0 && myData.iCompizMinor > 6))
		_compiz_dbus_action ("expo/allscreens/expo_button");
	else
		_compiz_dbus_action ("expo/allscreens/expo");
}

static void _compiz_menu_toggle_wlayer (void) {
	_compiz_get_version ();
	if (myData.iCompizMajor > 0 || (myData.iCompizMajor == 0 && myData.iCompizMinor > 6))
	_compiz_dbus_action ("widget/allscreens/toggle_button");
	else
		_compiz_dbus_action ("widget/allscreens/toggle");
}

static void _compiz_action_by_id (int k, Icon *pIcon) {
  switch (k) {
    case 0:
      cd_debug ("test de ccsm ...");
      gchar *cResult = cairo_dock_launch_command_sync ("which ccsm");
      if (cResult == NULL || *cResult != '/')
      {
        cairo_dock_show_temporary_dialog_with_icon (_("To configure Compiz, you need to install CCSM\n through your package manager (Synaptic, YasT, etc)"), pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 10000, "same icon");
      }
      else
        cairo_dock_launch_command ("ccsm");
      g_free (cResult);
    break;
    case 1:
      cairo_dock_launch_command ("emerald-theme-manager");
    break;
    case 2:
    	if (myData.bCompizIsRunning)
				cd_compiz_start_compiz ();
			else
				cd_compiz_start_system_wm ();
    break;
    case 3:
      _compiz_menu_activate_expo ();
    break;
    case 4:
      _compiz_menu_toggle_wlayer ();
    break;
    default:
     //Rien a faire
    break;
  }
}

static void _action_on_click (compizAction iAction) {
	switch (iAction) {
		case COMPIZ_NO_ACTION :
			return ;
		break;
		case COMPIZ_SWITCH_WM :
			cd_compiz_switch_manager ();
		break;
		case COMPIZ_LAYER :
			_compiz_menu_toggle_wlayer ();
		break;
		case COMPIZ_EXPO :
			_compiz_menu_activate_expo ();
		break;
		case COMPIZ_SHOW_DESKTOP :
			_compiz_menu_show_desktop ();
		break;
		default :
			cd_warning ("no action defined");
		break;
	}
}
CD_APPLET_ON_CLICK_BEGIN
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock) && pClickedIcon != NULL) {  // clic sur ne icone du sous-dock.
		//cd_debug (" clic sur %s", pClickedIcon->acName);
		///if (pClickedIcon->acCommand != NULL && strcmp (pClickedIcon->acCommand, "none") != 0)
		///	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		_compiz_action_by_id ((int) pClickedIcon->fOrder/2, pClickedIcon);
	}
	else if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL) {  // clic sur une des icones du desklet.
		if (pClickedIcon == myIcon)
			cairo_dock_launch_measure (myData.pMeasureTimer);
		else {
			if (pClickedIcon->acCommand != NULL && strcmp (pClickedIcon->acCommand, "none") != 0)
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			_compiz_action_by_id ((int) pClickedIcon->fOrder/2, pClickedIcon);
		}
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon) {
		_action_on_click (myConfig.iActionOnMiddleClick);
	}
	else if (pClickedIcon != NULL && pClickedIcon->acCommand != NULL && strcmp (pClickedIcon->acCommand, "none") != 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_MIDDLE_CLICK_END


static void cd_compiz_switch_decorator (GtkMenuItem *menu_item, gpointer *data) {
	compizDecorator iDecorator = GPOINTER_TO_INT (data);
	cd_compiz_start_decorator (iDecorator);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (CD_APPLET_CLICKED_ICON != NULL && strcmp(CD_APPLET_CLICKED_ICON->acName,D_("Emerald Manager")) == 0) {
		CD_APPLET_ADD_IN_MENU (D_("Reload Emerald"), cd_compiz_start_favorite_decorator, CD_APPLET_MY_MENU);
	}
	
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_IN_MENU (D_("Switch Windows Manager"), cd_compiz_switch_manager, pSubMenu);
	GtkWidget *pDecoratorSubMenu = CD_APPLET_ADD_SUB_MENU (D_("Switch Windows Decorator"), pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cDecorators[DECORATOR_EMERALD], cd_compiz_switch_decorator, pDecoratorSubMenu, GINT_TO_POINTER (DECORATOR_EMERALD));
		CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cDecorators[DECORATOR_GTK], cd_compiz_switch_decorator, pDecoratorSubMenu, GINT_TO_POINTER (DECORATOR_GTK));
		CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cDecorators[DECORATOR_KDE], cd_compiz_switch_decorator, pDecoratorSubMenu, GINT_TO_POINTER (DECORATOR_KDE));
		CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cDecorators[DECORATOR_HELIODOR], cd_compiz_switch_decorator, pDecoratorSubMenu, GINT_TO_POINTER (DECORATOR_HELIODOR));
		if (myConfig.cDecorators[DECORATOR_USER] != NULL) {
			CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cDecorators[DECORATOR_USER], cd_compiz_switch_decorator, pDecoratorSubMenu, GINT_TO_POINTER (DECORATOR_USER));
		}
	if (!myConfig.bScriptSubDock) {
		CD_APPLET_ADD_IN_MENU (D_("Toggle Exposition Mode"), _compiz_menu_activate_expo, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Toggle Widgets Layer"), _compiz_menu_toggle_wlayer, pSubMenu);
	}
	CD_APPLET_ADD_IN_MENU (D_("Toggle Show Desktop"), _compiz_menu_show_desktop, pSubMenu);
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	if (pClickedIcon != myIcon && (pClickedIcon == NULL || pClickedIcon->acCommand == NULL || strcmp (pClickedIcon->acCommand, "none") == 0 || ! CAIRO_DOCK_IS_APPLI (pClickedIcon)))
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;  // on ne veut pas des autres entrees habituelles du menu.
CD_APPLET_ON_BUILD_MENU_END

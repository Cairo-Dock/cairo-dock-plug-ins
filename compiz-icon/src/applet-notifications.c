/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-compiz.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the compiz-icon applet\n made by ChAnGFu for Cairo-Dock"))


static void _compiz_action_by_id (int k) {
  switch (k) {
    case 0:
      _compiz_cmd("ccsm &");
    break;
    case 1:
      _compiz_cmd("emerald-theme-manager &");
    break;
    case 2:
      cd_compiz_start_wm();
    break;
    default:
     //Rien a faires
    break;
  }
}

static void _compiz_dbus_action (const gchar *cCommand)
{
	GError *erreur = NULL;
	gchar *cDbusCommand = g_strdup_printf ("dbus-send --type=method_call --dest=org.freedesktop.compiz /org/freedesktop/compiz/%s org.freedesktop.compiz.activate string:'root' int32:%d", cCommand, cairo_dock_get_root_id ());
	g_spawn_command_line_async (cDbusCommand, &erreur);
	g_free (cDbusCommand);
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to send '%s' : %s", cCommand, erreur->message);
		g_error_free (erreur);
	}
}
static void _compiz_menu_show_desktop (void) {
	_compiz_dbus_action ("core/allscreens/show_desktop");
}
static void _compiz_menu_activate_expo (void) {
	_compiz_dbus_action ("expo/allscreens/expo");
}
static void _compiz_menu_toggle_wlayer (void) {
	_compiz_dbus_action ("widget/allscreens/toggle");
}

static void _action_on_click (compizAction iAction)
{
	switch (iAction)
	{
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
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock)) {  // clic sur le sous-dock.
		cd_debug (" clic sur %s", pClickedIcon->acName);
		_compiz_action_by_id ((int) pClickedIcon->fOrder/2);
	}
	else if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL) {  // clic sur une des icones du desklet.
		if (pClickedIcon == myIcon)
			cd_compiz_launch_measure();
		else
			_compiz_action_by_id ((int) pClickedIcon->fOrder/2);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon)
	{
		_action_on_click (myConfig.iActionOnMiddleClick);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Compiz Icon", pSubMenu, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU (D_("Switch Windows Manager"), cd_compiz_switch_manager, pSubMenu)
	CD_APPLET_ADD_IN_MENU (D_("Switch Windows Decorator"), cd_compiz_switch_decorator, pSubMenu)
	CD_APPLET_ADD_IN_MENU (D_("Toggle Exposition Mode"), _compiz_menu_activate_expo, pSubMenu)
	CD_APPLET_ADD_IN_MENU (D_("Toggle Widgets Layer"), _compiz_menu_toggle_wlayer, pSubMenu)
	CD_APPLET_ADD_IN_MENU (D_("Toggle Show Desktop"), _compiz_menu_show_desktop, pSubMenu)
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

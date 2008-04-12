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

static void _compiz_menu_script_expo (void) {
  GError *erreur = NULL;
  gchar *cCommand = g_strdup_printf("bash %s/expo", MY_APPLET_SHARE_DATA_DIR);
	g_spawn_command_line_async (cCommand, &erreur);
	g_free (cCommand);
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'expo' : %s", erreur->message);
		g_error_free (erreur);
	}
}
static void _compiz_menu_script_wlayer (void) {
  GError *erreur = NULL;
  gchar *cCommand = g_strdup_printf("bash %s/wlayer", MY_APPLET_SHARE_DATA_DIR);
	g_spawn_command_line_async (cCommand, &erreur);
	g_free (cCommand);
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'wlayer' : %s", erreur->message);
		g_error_free (erreur);
	}
}
CD_APPLET_ON_CLICK_BEGIN

	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock)) {
		cd_debug (" clic sur %s", pClickedIcon->acName);
		_compiz_action_by_id ((int) pClickedIcon->fOrder/2);
	}
	else if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL) {
		if (pClickedIcon == myIcon)
			cd_compiz_launch_measure();
		else
			_compiz_action_by_id ((int) pClickedIcon->fOrder/2);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  cd_compiz_switch();
CD_APPLET_ON_MIDDLE_CLICK_END

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Compiz Icon", pSubMenu, CD_APPLET_MY_MENU)
	  CD_APPLET_ADD_IN_MENU (D_("Switch Windows Decorator"), cd_compiz_switch_decorator, pSubMenu)
	  CD_APPLET_ADD_IN_MENU (D_("Toggle Exposition Mode"), _compiz_menu_script_expo, pSubMenu)
	  CD_APPLET_ADD_IN_MENU (D_("Toggle Widgets Layer"), _compiz_menu_script_wlayer, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

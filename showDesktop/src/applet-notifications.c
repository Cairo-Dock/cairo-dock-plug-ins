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

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


static gboolean _cd_allow_minimize (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, gpointer data)
{
	pDesklet->bAllowMinimize = TRUE;
	return FALSE;
}

static void _cd_show_hide_desktop (gboolean bShowDesklets)
{
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	
	if (! bDesktopIsVisible && ! bShowDesklets)  // on autorise chaque desklet a etre minimise. l'autorisation est annulee lors de leur cachage, donc on n'a pas besoin de faire le contraire apres avoir montre le bureau.
	{
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cd_allow_minimize, NULL);
	}
	
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
}

static void _cd_show_hide_desklet (void)
{
	if (myData.bDeskletsVisible)
	{
		//myData.xLastActiveWindow = cairo_dock_get_current_active_window ();
		cairo_dock_set_all_desklets_visible (myConfig.bShowWidgetLayerDesklet);
	}
	else
	{
		cairo_dock_set_desklets_visibility_to_default ();
		//cairo_dock_show_xwindow (myData.xLastActiveWindow);
	}
	myData.bDeskletsVisible = ! myData.bDeskletsVisible;
}

static void _compiz_dbus_action (const gchar *cCommand)  // taken from the Compiz-Icon applet, thanks ChangFu !
{
	if (! cairo_dock_dbus_detect_application ("org.freedesktop.compiz"))
		cd_warning  ("Dbus plug-in must be activated in Compiz !");
	GError *erreur = NULL;
	gchar *cDbusCommand = g_strdup_printf ("dbus-send --type=method_call --dest=org.freedesktop.compiz /org/freedesktop/compiz/%s org.freedesktop.compiz.activate string:'root' int32:%d", cCommand, cairo_dock_get_root_id ());
	g_spawn_command_line_async (cDbusCommand, &erreur);
	g_free (cDbusCommand);
	if (erreur != NULL)
	{
		cd_warning ("Compiz-icon : when trying to send '%s' : %s", cCommand, erreur->message);
		g_error_free (erreur);
	}
}

static void _cd_show_widget_layer (void)
{
	_compiz_dbus_action ("widget/allscreens/toggle_button");  // toggle avant la 0.7
}

static void _cd_expose (void)
{
	_compiz_dbus_action ("expo/allscreens/expo_button");  // expo avant la 0.7
}

static void _cd_action_on_middle_click (void)
{
	switch (myConfig.iActionOnMiddleClick)
	{
		case CD_SHOW_DESKTOP :
			_cd_show_hide_desktop (TRUE);  // TRUE <=> show the desklets
		break ;
		case CD_SHOW_DESKLETS :
			_cd_show_hide_desklet ();
		break ;
		case CD_SHOW_WIDGET_LAYER :
			_cd_show_widget_layer ();
		break ;
		case CD_EXPOSE :
			_cd_expose ();
		break ;
		default:
		break;
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_cd_show_hide_desktop (myConfig.bShowDesklets);
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_action_on_middle_click ();
CD_APPLET_ON_MIDDLE_CLICK_END



void cd_show_desktop_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	_cd_action_on_middle_click ();
}

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

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-run-dialog.h"
#include "applet-notifications.h"

static const gchar *s_cEditMenuCmd = NULL; // we need to check with 'which' only one time if alacarte or kmenuedit is available

static void cd_menu_show_menu (void)
{
	if (myData.pMenu != NULL)
	{
		CD_APPLET_POPUP_MENU_ON_MY_ICON (myData.pMenu);
	}
}

static void cd_menu_show_hide_quick_launch (void)
{
	if (myData.pQuickLaunchDialog == NULL)
	{
		myData.pQuickLaunchDialog = cd_menu_create_quick_launch_dialog (myApplet);
		cairo_dock_dialog_reference (myData.pQuickLaunchDialog);
	}
	else
	{
		cairo_dock_toggle_dialog_visibility (myData.pQuickLaunchDialog);
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_menu_show_menu ();
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _cd_menu_configure_menu (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	GError *error = NULL;
	if (myConfig.cConfigureMenuCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cConfigureMenuCommand, &error);
	}
	else if (s_cEditMenuCmd != NULL)
	{
		g_spawn_command_line_async (s_cEditMenuCmd, &error);
	}
	
	if (error != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s",
			myConfig.cConfigureMenuCommand ? myConfig.cConfigureMenuCommand : s_cEditMenuCmd,
			error->message);
		g_error_free (error);
	}
	CD_APPLET_LEAVE();
}

static gboolean _cd_check_edit_menu_cmd (const gchar *cWhich)
{
	gchar *cResult = cairo_dock_launch_command_sync (cWhich);  // Gnome (2 + 3(?)) + XFCE(?)
	gboolean bResult = (cResult != NULL && *cResult == '/');
	g_free (cResult);
	return bResult;
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Quick launch"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_EXECUTE, cd_menu_show_hide_quick_launch, CD_APPLET_MY_MENU);
	g_free (cLabel);
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);

	static gboolean bEditMenuCmdChecked = FALSE;
	if (!myConfig.cConfigureMenuCommand && !bEditMenuCmdChecked)
	{
		bEditMenuCmdChecked = TRUE;
		if (_cd_check_edit_menu_cmd ("which alacarte"))
			s_cEditMenuCmd = "alacarte";
		else if (_cd_check_edit_menu_cmd ("which kmenuedit"))
			s_cEditMenuCmd = "kmenuedit";
		else if (_cd_check_edit_menu_cmd ("which menulibre"))
			s_cEditMenuCmd = "menulibre";
		else if (_cd_check_edit_menu_cmd ("which ezame"))
			s_cEditMenuCmd = "ezame";
	}
	if (myConfig.cConfigureMenuCommand || s_cEditMenuCmd)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Configure menu"), GTK_STOCK_PREFERENCES, _cd_menu_configure_menu, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear recent"), GTK_STOCK_CLEAR, cd_menu_clear_recent, CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_menu_show_hide_quick_launch ();
CD_APPLET_ON_MIDDLE_CLICK_END


void cd_menu_on_shortkey_menu (const char *keystring, gpointer data)
{
	cd_menu_show_menu ();
}

void cd_menu_on_shortkey_quick_launch (const char *keystring, gpointer data)
{
	cd_menu_show_hide_quick_launch ();
}

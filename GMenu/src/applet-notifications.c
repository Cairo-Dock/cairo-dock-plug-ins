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
#include "applet-menu.h"
#include "applet-recent.h"
#include "applet-run-dialog.h"
#include "applet-notifications.h"

static const gchar *s_cEditMenuCmd = NULL; // we need to check with 'which' only one time if alacarte or kmenuedit is available


static void cd_menu_show_hide_quick_launch (void)
{
	cd_run_dialog_show_hide (myApplet);
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_menu_show_menu ();
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _cd_menu_configure_menu (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	if (myConfig.cConfigureMenuCommand != NULL)
		cairo_dock_launch_command_full (myConfig.cConfigureMenuCommand, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	else if (s_cEditMenuCmd != NULL)
		cairo_dock_launch_command_single_gui (s_cEditMenuCmd);
	CD_APPLET_LEAVE();
}

static gboolean _cd_check_edit_menu_cmd (const gchar *cCommand)
{
	const char * const args[] = {"which", cCommand, NULL};
	gchar *cResult = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);  // Gnome (2 + 3(?)) + XFCE(?)
	gboolean bResult = (cResult != NULL && *cResult == '/');
	g_free (cResult);
	return bResult;
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Quick launch"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_EXECUTE, cd_menu_show_hide_quick_launch, CD_APPLET_MY_MENU);
	g_free (cLabel);
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);

	static gboolean bEditMenuCmdChecked = FALSE;
	if (!myConfig.cConfigureMenuCommand && !bEditMenuCmdChecked)
	{
		bEditMenuCmdChecked = TRUE;
		const char *cmds[] = {"alacarte", "kmenuedit", "menulibre", "ezame", "cinnamon-menu-editor", NULL};
		const char **tmp;
		for (tmp = cmds; *tmp; ++tmp)
			if (_cd_check_edit_menu_cmd (*tmp))
			{
				s_cEditMenuCmd = *tmp;
				break;
			}
	}

	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Configure menu"),
		GLDI_ICON_NAME_PREFERENCES, _cd_menu_configure_menu, CD_APPLET_MY_MENU);
	if (myConfig.cConfigureMenuCommand == NULL && s_cEditMenuCmd == NULL)
	{
		gchar *cTooltip = g_strdup_printf ("%s %s",
			D_("None of these applications seems available:"),
			"Alacarte, KMenuEdit, MenuLibre, Ezame");
		gtk_widget_set_tooltip_text (pMenuItem, cTooltip);
		g_free (cTooltip);
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	}

	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear recent"), GLDI_ICON_NAME_CLEAR, cd_menu_clear_recent, CD_APPLET_MY_MENU);
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

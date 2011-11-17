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
#include "applet-logout.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN

	cd_logout_display_actions ();

CD_APPLET_ON_CLICK_END

		
static void _logout (void)
{
	if (myConfig.cUserAction != NULL)
	{
		cairo_dock_launch_command (myConfig.cUserAction);
	}
	else
	{
		gboolean bLoggedOut = cairo_dock_fm_logout ();
		if (! bLoggedOut)
		{
			cd_logout_display_actions ();
		}
	}
}
static void _shutdown (void)
{
	if (myConfig.cUserAction2 != NULL)
	{
		cairo_dock_launch_command (myConfig.cUserAction2);
	}
	else
	{
		gboolean bShutdowned = cairo_dock_fm_shutdown ();
		if (! bShutdowned)
		{
			cd_logout_display_actions ();
		}
	}
}
static inline void _execute_action (gint iAction)
{
	switch (iAction)
	{
		case CD_LOGOUT:
			_logout ();
		break;
		case CD_SHUTDOWN:
			_shutdown ();
		break;
		case CD_LOCK_SCREEN:
			cairo_dock_fm_lock_screen ();
		break;
		case CD_POP_UP_MENU:
		default:
			cd_logout_display_actions ();
		break;
	}
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN

	_execute_action (myConfig.iActionOnMiddleClick);

CD_APPLET_ON_MIDDLE_CLICK_END


static void cd_logout_manage_users (GtkMenuItem *menu_item, gchar *cUserName)
{
	GError * error = NULL;
	if (! g_spawn_command_line_async("gnome-control-center user-accounts", &error))  // Gnome3
	{
		cd_warning ("Couldn't launch 'gnome-control-center user-accounts': %s", error->message);
		g_error_free(error);
	}  /// TODO: handle other DE ...
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Manage users"), GTK_STOCK_EDIT, cd_logout_manage_users, CD_APPLET_MY_MENU);
	}
CD_APPLET_ON_BUILD_MENU_END


void cd_logout_on_keybinding_pull (const gchar *keystring, gpointer user_data)
{
	_execute_action (myConfig.iActionOnShortkey);
}

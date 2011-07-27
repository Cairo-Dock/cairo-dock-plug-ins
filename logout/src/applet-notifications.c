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
		default:
			_logout ();
		break;
		case CD_SHUTDOWN:
			_shutdown ();
		break;
		case CD_LOCK_SCREEN:
			cairo_dock_fm_lock_screen ();
		break;
	}
}

CD_APPLET_ON_CLICK_BEGIN
{
	/**if (myIcon->Xid != 0)
	{
		if (cairo_dock_get_current_active_window () == myIcon->Xid && myTaskBar.bMinimizeOnClick)
			cairo_dock_minimize_xwindow (myIcon->Xid);
		else
			cairo_dock_show_xwindow (myIcon->Xid);
	}
	else*/
	{
		// on execute l'action meme si la fenetre est deja ouverte (ca lui redonne le focus), car si on avait deja execute l'autre action, ca empeche de faire celle-ci.
		_execute_action (myConfig.iActionOnClick);
	}
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	_execute_action (myConfig.iActionOnMiddleClick);
}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_logout (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	_logout ();
	CD_APPLET_LEAVE ();
}
static void _cd_shutdown (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	_shutdown ();
	CD_APPLET_LEAVE ();
}
static void _cd_lock_screen (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_lock_screen ();
	CD_APPLET_LEAVE ();
}

static void _cd_logout_guest_session (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	cd_logout_launch_guest_session ();
	CD_APPLET_LEAVE ();
}

static void _cd_logout_program_shutdown (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	cd_logout_program_shutdown ();
	CD_APPLET_LEAVE ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
{
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	gchar *cLabel;
	if (! myData.bCapabilitiesChecked)  // if we're using our own logout methods, they are all accessible from the left-click, so no need to add the following actions in the right-click menu.
	{
		if (myConfig.iActionOnClick != CD_LOGOUT)  // logout action not on click => put it in the menu
		{
			if (myConfig.iActionOnMiddleClick == CD_LOGOUT)  // logout action on middle-click
				cLabel = g_strdup_printf ("%s (%s)", D_("Log out"), D_("middle-click"));
			else
				cLabel = g_strdup (D_("Log out"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", _cd_logout, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		if (myConfig.iActionOnClick != CD_SHUTDOWN)  // shutdown action not on click => put it in the menu
		{
			if (myConfig.iActionOnMiddleClick == CD_SHUTDOWN)  // logout action on middle-click
				cLabel = g_strdup_printf ("%s (%s)", D_("Shut down"), D_("middle-click"));
			else
				cLabel = g_strdup (D_("Shut down"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", _cd_shutdown, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		if (myConfig.iActionOnClick != CD_LOCK_SCREEN)  // lockscreen action not on click => put it in the menu
		{
			if (myConfig.iActionOnMiddleClick == CD_LOCK_SCREEN)  // lockscreen action on middle-click
				cLabel = g_strdup_printf ("%s (%s)", D_("Lock screen"), D_("middle-click"));
			else
				cLabel = g_strdup (D_("Lock screen"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/locked.svg", _cd_lock_screen, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
	}
	if (cd_logout_have_guest_session ()) // Guest Session
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Guest session"), MY_APPLET_SHARE_DATA_DIR"/system-guest.svg", _cd_logout_guest_session, CD_APPLET_MY_MENU);
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shut-down"), MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.svg", _cd_logout_program_shutdown, CD_APPLET_MY_MENU);  // pas beaucoup d'entrees => on le met dans le menu global.
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END

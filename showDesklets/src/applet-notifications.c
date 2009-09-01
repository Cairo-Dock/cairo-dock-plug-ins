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
#include "applet-notifications.h"



static void _hide_unhide_desklets (void)
{
	if (myData.bHide)
	{
		cairo_dock_set_desklets_visibility_to_default ();
		cairo_dock_show_xwindow (myData.xLastActiveWindow);
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage);
	}
	else
	{
		myData.xLastActiveWindow = cairo_dock_get_current_active_window ();
		cairo_dock_set_all_desklets_visible (myConfig.bShowWidgetLayerDesklet);
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHideImage);
	}
	CD_APPLET_REDRAW_MY_ICON;
	myData.bHide = ! myData.bHide;
}

CD_APPLET_ON_CLICK_BEGIN
	_hide_unhide_desklets ();
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


gboolean cd_show_desklet_active_window_changed (Window *pXid)
{
	if (pXid != NULL && myData.bHide)
	{
		if (cairo_dock_get_desklet_by_Xid (*pXid) == NULL)
		{
			myData.xLastActiveWindow = *pXid;
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

void cd_show_desklet_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	_hide_unhide_desklets ();
}

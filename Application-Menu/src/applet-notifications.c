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
#include "gdk/gdkx.h"

#include "applet-struct.h"
#include "applet-app.h"
#include "applet-notifications.h"


static void _show_menu (gboolean bOnMouse)
{
	if (myData.pMenu != NULL)
	{
		if (bOnMouse)
		{
			gtk_widget_show_all (GTK_WIDGET (myData.pMenu));
			gtk_menu_popup (GTK_MENU (myData.pMenu),
				NULL,
				NULL,
				(GtkMenuPositionFunc) NULL,
				NULL,
				0,
				gtk_get_current_event_time ());
		}
		else
		{
			CD_APPLET_POPUP_MENU_ON_MY_ICON (GTK_WIDGET (myData.pMenu));
		}
	}
	else  /// either show a message, or remember the user demand, so that we pop the menu as soon as we get it...
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("The application didn't send its menu to us."), myIcon, myContainer, 4000., "same icon");
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_show_menu (FALSE);
CD_APPLET_ON_CLICK_END


//\___________ Same as ON_CLICK, but with middle-click.
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	// set the window behind all the others...
	if (myData.iCurrentWindow != 0)
		cairo_dock_lower_xwindow (myData.iCurrentWindow);
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Same as ON_CLICK, but with scroll. Moreover, CD_APPLET_SCROLL_UP tels you is the user scrolled up, CD_APPLET_SCROLL_DOWN the opposite.
CD_APPLET_ON_SCROLL_BEGIN
	// minimize...
	if (myData.iCurrentWindow != 0 && CD_APPLET_SCROLL_DOWN)
		cairo_dock_minimize_xwindow (myData.iCurrentWindow);
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	// nothing to do here, since the icon is considered as an appli.
	
CD_APPLET_ON_BUILD_MENU_END

		
CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	if (myData.iCurrentWindow != 0)
	{
		Icon *pAppli = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
		if (pAppli)
			cairo_dock_maximize_xwindow (pAppli->Xid, ! pAppli->bIsMaximized);
	}
CD_APPLET_ON_DOUBLE_CLICK_END


void cd_app_menu_on_keybinding_pull (const gchar *keystring, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_show_menu (myConfig.bMenuOnMouse);
	CD_APPLET_LEAVE();
}


static void _check_dock_is_active (gchar *cDockName, CairoDock *pDock, Window *data)
{
	Window xActiveWindow = data[0];
	if (gldi_container_get_Xid (CAIRO_CONTAINER (pDock)) == xActiveWindow)
		data[1] = 1;
}
gboolean cd_app_menu_on_active_window_changed (gpointer pUserData, Window *XActiveWindow)
{
	if (XActiveWindow == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	// check if a dock has the focus (we don't want to control the dock, it wouldn't make sense anyway).
	/// TODO: check each container...
	Window data[2] = {*XActiveWindow, 0};
	cairo_dock_foreach_docks ((GHFunc) _check_dock_is_active, data);
	
	if (data[1] == 0)  // not a dock, so let's take it.
	{
		// take this new window (possibly 0).
		cd_app_menu_set_current_window (*XActiveWindow);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_app_menu_on_property_changed (gpointer data, Window Xid, Atom aProperty, int iState)
{
	if (Xid != 0 && Xid == myData.iCurrentWindow)
	{
		Display *dpy = cairo_dock_get_Xdisplay();
		Atom aNetWmState = XInternAtom (dpy, "_NET_WM_STATE", False);
		if (aProperty == aNetWmState)
		{
			Icon *icon = cairo_dock_get_icon_with_Xid (Xid);
			if (icon)
				cd_app_menu_set_window_border (Xid, ! icon->bIsMaximized);
		}
	}
}

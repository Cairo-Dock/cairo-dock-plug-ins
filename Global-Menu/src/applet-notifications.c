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
#include "applet-draw.h"
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

//\___________ Action on click: show the menu
CD_APPLET_ON_CLICK_BEGIN
	if (myData.iCurrentWindow == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	if (myConfig.bDisplayControls)
	{
		int iNumButton = cd_app_menu_find_button (myApplet);
		if (iNumButton >= 0)
		{
			switch (iNumButton)
			{
				case CD_BUTTON_MENU:
					_show_menu (FALSE);
				break;
				case CD_BUTTON_MINIMIZE:
					if (myData.bCanMinimize)
						cairo_dock_minimize_xwindow (myData.iCurrentWindow);
				break;
				case CD_BUTTON_MAXIMIZE:
					if (myData.bCanMaximize)
					{
						Icon *pAppli = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
						if (pAppli)
							cairo_dock_maximize_xwindow (pAppli->Xid, ! pAppli->bIsMaximized);
					}
				break;
				case CD_BUTTON_CLOSE:
					if (myData.bCanClose)
						cairo_dock_close_xwindow (myData.iCurrentWindow);
				break;
			}
		}
	}
	else if (myConfig.bDisplayMenu)
		_show_menu (FALSE);
CD_APPLET_ON_CLICK_END


//\___________ Other actions are defined to mime the usual actions available on windows top border
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	// set the window behind all the others.
	if (myData.iCurrentWindow != 0)
		cairo_dock_lower_xwindow (myData.iCurrentWindow);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	// minimize the window (we could also use the scroll to (un)shade the window, but I'm afraid that a maximized shaded window would be too much hidden, users could be confused).
	if (myData.iCurrentWindow != 0 && CD_APPLET_SCROLL_DOWN)
		cairo_dock_minimize_xwindow (myData.iCurrentWindow);
CD_APPLET_ON_SCROLL_END

		
CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	// maximize/restaure the window.
	if (myData.iCurrentWindow != 0)
	{
		Icon *pAppli = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
		if (pAppli)
			cairo_dock_maximize_xwindow (pAppli->Xid, ! pAppli->bIsMaximized);
	}
CD_APPLET_ON_DOUBLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	// nothing to do here, since the icon is considered as an appli, the dock will fill it for free !
CD_APPLET_ON_BUILD_MENU_END


void cd_app_menu_on_keybinding_pull (const gchar *keystring, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_show_menu (myConfig.bMenuOnMouse);
	CD_APPLET_LEAVE();
}


//\___________ Other notifications, that are not from the user (but from the Applications-manager)
static void _check_dock_is_active (gchar *cDockName, CairoDock *pDock, Window *data)
{
	Window xActiveWindow = data[0];
	if (gldi_container_get_Xid (CAIRO_CONTAINER (pDock)) == xActiveWindow)
		data[1] = 1;
}
gboolean cd_app_menu_on_active_window_changed (CairoDockModuleInstance *myApplet, Window *XActiveWindow)
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

gboolean cd_app_menu_on_state_changed (CairoDockModuleInstance *myApplet, Icon *pIcon, gboolean bHiddenChanged, gboolean bMaximizedChanged, gboolean bFullScreenChanged)
{
	if (pIcon && pIcon->Xid == myData.iCurrentWindow)
	{
		if (bMaximizedChanged)
		{
			cd_app_menu_set_window_border (pIcon->Xid, ! pIcon->bIsMaximized);
			cd_app_menu_redraw_buttons ();
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_app_menu_on_name_changed (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	if (pIcon && pIcon->Xid == myData.iCurrentWindow)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (pIcon->cName);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_app_menu_on_new_appli (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	if (pIcon && pIcon->bIsMaximized)
	{
		cd_app_menu_set_window_border (pIcon->Xid, ! pIcon->bIsMaximized);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

/**gboolean cd_app_menu_on_property_changed (CairoDockModuleInstance *myApplet, Window Xid, Atom aProperty, int iState)
{
	if (Xid != 0 && Xid == myData.iCurrentWindow)
	{
		Display *dpy = cairo_dock_get_Xdisplay();
		Atom aNetWmState = XInternAtom (dpy, "_NET_WM_STATE", False);
		Atom s_aNetWmName = XInternAtom (dpy, "_NET_WM_NAME", False);
		Atom s_aWmName = XInternAtom (dpy, "WM_NAME", False);
		if (aProperty == aNetWmState)
		{
			Icon *icon = cairo_dock_get_icon_with_Xid (Xid);
			if (icon)
				cd_app_menu_set_window_border (Xid, ! icon->bIsMaximized);
			// update the icon to reflect the change of state (max/restore button)
			cd_app_menu_redraw_buttons ();
		}
		else if (iState == PropertyNewValue && (aProperty == s_aNetWmName || aProperty == s_aWmName))
		{
			Icon *icon = cairo_dock_get_icon_with_Xid (Xid);
			CD_APPLET_SET_NAME_FOR_MY_ICON (icon ? icon->cName : NULL);
		}  // ignore the change of icon, we just want to use the default application icon.
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}*/


//\___________ Other notifications, for animation of the buttons.

gboolean on_mouse_moved (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	// find the pointed button
	int iNumButton = cd_app_menu_find_button (myApplet);
	if (iNumButton >= 0 && iNumButton < myData.iNbButtons)
	{
		// trigger animation
		*bStartAnimation = TRUE;
	}
	
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}


gboolean cd_app_menu_on_update_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	CD_APPLET_ENTER;
	int iNumButton = 0;
	if (! myIcon->bPointed || ! pContainer->bInside)  // our icon is not pointed, only update it to finish the current button animations.
	{
		if (myData.bButtonAnimating)  // one or more button is animating.
		{
			myData.bButtonAnimating = FALSE;
			
			if (cairo_dock_image_buffer_is_animated (&myData.minimizeButton))
			{
				if (iNumButton == CD_BUTTON_MINIMIZE || myData.minimizeButton.iCurrentFrame != 0)
					cairo_dock_image_buffer_next_frame (&myData.minimizeButton);
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MINIMIZE || myData.minimizeButton.iCurrentFrame != 0);
			}
			else
			{
				if (iNumButton == CD_BUTTON_MINIMIZE || myData.iAnimIterMin != 0)
				{
					myData.iAnimIterMin ++;
					if (myData.iAnimIterMin >= CD_ANIM_STEPS)
						myData.iAnimIterMin = 0;
				}
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MINIMIZE || myData.iAnimIterMin != 0);
			}
			
			if (cairo_dock_image_buffer_is_animated (&myData.maximizeButton))
			{
				if (iNumButton == CD_BUTTON_MAXIMIZE || myData.maximizeButton.iCurrentFrame != 0)
				{
					cairo_dock_image_buffer_next_frame (&myData.maximizeButton);
					cairo_dock_image_buffer_next_frame (&myData.restoreButton);
				}
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MAXIMIZE || myData.maximizeButton.iCurrentFrame != 0);
			}
			else
			{
				if (iNumButton == CD_BUTTON_MAXIMIZE || myData.iAnimIterMax != 0)
				{
					myData.iAnimIterMax ++;
					if (myData.iAnimIterMax >= CD_ANIM_STEPS)
						myData.iAnimIterMax = 0;
				}
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MAXIMIZE || myData.iAnimIterMax != 0);
			}
			
			if (cairo_dock_image_buffer_is_animated (&myData.closeButton))
			{
				if (iNumButton == CD_BUTTON_CLOSE || myData.closeButton.iCurrentFrame != 0)
					cairo_dock_image_buffer_next_frame (&myData.closeButton);
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_CLOSE || myData.closeButton.iCurrentFrame != 0);
			}
			else
			{
				if (iNumButton == CD_BUTTON_CLOSE || myData.iAnimIterClose != 0)
				{
					myData.iAnimIterClose ++;
					if (myData.iAnimIterClose >= CD_ANIM_STEPS)
						myData.iAnimIterClose = 0;
				}
				myData.bButtonAnimating |= (iNumButton == CD_BUTTON_CLOSE || myData.iAnimIterClose != 0);
			}
			
			cd_app_menu_redraw_buttons ();
			*bContinueAnimation = myData.bButtonAnimating;
		}
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	}
	
	iNumButton = cd_app_menu_find_button (myApplet);
	myData.bButtonAnimating = FALSE;
	
	if (cairo_dock_image_buffer_is_animated (&myData.minimizeButton))
	{
		if (iNumButton == CD_BUTTON_MINIMIZE || myData.minimizeButton.iCurrentFrame != 0)
			cairo_dock_image_buffer_next_frame (&myData.minimizeButton);
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MINIMIZE || myData.minimizeButton.iCurrentFrame != 0);
	}
	else
	{
		if (iNumButton == CD_BUTTON_MINIMIZE || myData.iAnimIterMin != 0)
		{
			myData.iAnimIterMin ++;
			if (myData.iAnimIterMin >= CD_ANIM_STEPS)
				myData.iAnimIterMin = 0;
		}
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MINIMIZE || myData.iAnimIterMin != 0);
	}
	
	if (cairo_dock_image_buffer_is_animated (&myData.maximizeButton))
	{
		if (iNumButton == CD_BUTTON_MAXIMIZE || myData.maximizeButton.iCurrentFrame != 0)
		{
			cairo_dock_image_buffer_next_frame (&myData.maximizeButton);
			cairo_dock_image_buffer_next_frame (&myData.restoreButton);
		}
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MAXIMIZE || myData.maximizeButton.iCurrentFrame != 0);
	}
	else
	{
		if (iNumButton == CD_BUTTON_MAXIMIZE || myData.iAnimIterMax != 0)
		{
			myData.iAnimIterMax ++;
			if (myData.iAnimIterMax >= CD_ANIM_STEPS)
				myData.iAnimIterMax = 0;
		}
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_MAXIMIZE || myData.iAnimIterMax != 0);
	}
	
	if (cairo_dock_image_buffer_is_animated (&myData.closeButton))
	{
		if (iNumButton == CD_BUTTON_CLOSE || myData.closeButton.iCurrentFrame != 0)
			cairo_dock_image_buffer_next_frame (&myData.closeButton);
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_CLOSE || myData.closeButton.iCurrentFrame != 0);
	}
	else
	{
		if (iNumButton == CD_BUTTON_CLOSE || myData.iAnimIterClose != 0)
		{
			myData.iAnimIterClose ++;
			if (myData.iAnimIterClose >= CD_ANIM_STEPS)
				myData.iAnimIterClose = 0;
		}
		myData.bButtonAnimating |= (iNumButton == CD_BUTTON_CLOSE || myData.iAnimIterClose != 0);
	}
	
	cd_app_menu_redraw_buttons ();
	*bContinueAnimation = myData.bButtonAnimating;
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-app.h"
#include "applet-draw.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Application Menu"),
	3, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet allows you to control the current active window:\n"
	"  close, minimize, maximize, and display the application menu."
	"To display the menu, applications have to support this feature, which is the case on Ubuntu by default.\n"
	"You can bind a shortkey to this action."),
	"Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_app_menu_on_active_window_changed,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_PROPERTY_CHANGED,
		(CairoDockNotificationFunc) cd_app_menu_on_property_changed,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	
	// start !
	cd_app_menu_start ();
	myData.iNbButtons = myConfig.bDisplayControls * 3 + 1;  // we display the icon even if we don't provide the menu.
	
	if (myConfig.bDisplayControls)  // no animation on mouse hover if the buttons are displayed, it's hard to click
	{
		CD_APPLET_SET_STATIC_ICON;
	}
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DOUBLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	cairo_dock_register_notification_on_object (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(CairoDockNotificationFunc) on_mouse_moved,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	
	// keyboard events
	if (myConfig.bDisplayMenu)
		myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
			D_("Show/hide the current application menu"),
			"Configuration", "shortkey",
			(CDBindkeyHandler) cd_app_menu_on_keybinding_pull);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_app_menu_on_active_window_changed, myApplet);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_PROPERTY_CHANGED,
		(CairoDockNotificationFunc) cd_app_menu_on_property_changed, myApplet);
	
	cd_app_menu_stop ();

	// mouse events
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DOUBLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	if (myConfig.bDisplayMenu)
		cd_keybinder_unbind (myData.pKeyBinding);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	// if they are loaded, reload the controls icons.
	cd_app_menu_load_button_images ();
	cd_app_menu_default_image ();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		// windows borders
		cd_app_menu_set_windows_borders (!myConfig.bDisplayControls);
		
		// registrar
		if (myConfig.bDisplayMenu && !myData.pProxyRegistrar)
			cd_app_detect_registrar ();
		else if (! myConfig.bDisplayMenu)  // even if myData.pProxyRegistrar is NULL, we have to cancel the detection
			cd_app_disconnect_from_registrar ();
		
		// to update any param that could have changed, simply re-set the current window.
		myData.iNbButtons = myConfig.bDisplayControls * 3 + 1;
		Window iActiveWindow = myData.iCurrentWindow;
		myData.iCurrentWindow = 0;
		cd_app_menu_set_current_window (iActiveWindow);
		
		// shortkey
		if (myConfig.bDisplayMenu)
		{
			if (myData.pKeyBinding)
				cd_keybinder_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
			else
				myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
					D_("Show/hide the current application menu"),
					"Configuration", "shortkey",
					(CDBindkeyHandler) cd_app_menu_on_keybinding_pull);
		}
		else if (myData.pKeyBinding)
		{
			cd_keybinder_unbind (myData.pKeyBinding);
		}
		
		cairo_dock_set_icon_static (myIcon, myConfig.bDisplayControls);
	}
	
	if (myConfig.bDisplayControls)
	{
		cd_app_menu_resize ();
	}
	
CD_APPLET_RELOAD_END

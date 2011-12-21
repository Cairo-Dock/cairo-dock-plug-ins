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
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Application Menu"),
	2, 4, 0,
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
		NOTIFICATION_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_app_menu_on_active_window_changed, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_PROPERTY_CHANGED,
		(CairoDockNotificationFunc) cd_app_menu_on_property_changed,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	// detect the registrar (launch it if not present).
	cd_app_detect_registrar ();
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Show/hide the current application menu"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_app_menu_on_keybinding_pull);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_app_menu_on_active_window_changed, NULL);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_PROPERTY_CHANGED,
		(CairoDockNotificationFunc) cd_app_menu_on_property_changed, NULL);
	
	cd_app_disconnect_from_registrar ();

	// mouse events
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	cd_keybinder_unbind (myData.pKeyBinding);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		/// redraw ...
		
		
		cd_keybinder_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
CD_APPLET_RELOAD_END

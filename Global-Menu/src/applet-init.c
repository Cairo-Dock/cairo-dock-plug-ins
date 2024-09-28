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


CD_APPLET_DEFINE2_BEGIN (N_("Global Menu"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("This applet allows you to control the current active window:\n"
	"  close, minimize, maximize, and display the application menu."
	"To display the menu, applications have to support this feature, which is the case on Ubuntu by default.\n"
	"You can bind a shortkey to this action."),
	"Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE2_END

static gboolean _reversed_buttons_order (void)
{	// TRUE: on the left (close, min, max) || FALSE: on the right (min, max, close)
	if (myConfig.iButtonsOrder == CD_GM_BUTTON_ORDER_AUTO
	        && ((myDock && (int) myIcon->fXAtRest < (myDock->container.iWidth / 2))
	        || (myDesklet && myDesklet->container.iWindowPositionX < (g_desktopGeometry.Xscreen.width / 2))))
		return TRUE;
	return (myConfig.iButtonsOrder == CD_GM_BUTTON_ORDER_LEFT);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(GldiNotificationFunc) cd_app_menu_on_active_window_changed,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_STATE_CHANGED,
		(GldiNotificationFunc) cd_app_menu_on_state_changed,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_NAME_CHANGED,
		(GldiNotificationFunc) cd_app_menu_on_name_changed,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_CREATED,
		(GldiNotificationFunc) cd_app_menu_on_new_appli,
		GLDI_RUN_AFTER, myApplet);
	
	gldi_object_register_notification (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) on_mouse_moved,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (myContainer,
		NOTIFICATION_UPDATE_SLOW,
		(GldiNotificationFunc) cd_app_menu_on_update_container,
		GLDI_RUN_AFTER, myApplet);
	
	// start !
	myData.iNbButtons = myConfig.bDisplayControls * 3 + 1;  // we display the icon even if we don't provide the menu.
	cd_app_menu_start ();
	
	if (myConfig.bDisplayControls)  // no animation on mouse hover if the buttons are displayed, it's hard to click
	{
		CD_APPLET_SET_STATIC_ICON;
		myData.bReversedButtonsOrder = _reversed_buttons_order ();
		/** => TODO? check if the position of the icon has changed?
		 * => if we use the 'auto' position, this position of the icon seems to
		 * not be correct if we call this function here... we can check if
		 * something has changed with CD_APPLET_RELOAD but the order doesn't
		 * change if the icon has been moved in the same dock (it works if the
		 * container has changed). Do we have to register to this notifications?
		 * => NOTIFICATION_ICON_MOVED
		 */
	}
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DOUBLE_CLICK_EVENT;
	// CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	if (myConfig.bDisplayMenu)
		myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
			D_("Show/hide the current application menu"),
			"Configuration", "shortkey",
			(CDBindkeyHandler) cd_app_menu_on_keybinding_pull);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(GldiNotificationFunc) cd_app_menu_on_active_window_changed, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_STATE_CHANGED,
		(GldiNotificationFunc) cd_app_menu_on_state_changed, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_NAME_CHANGED,
		(GldiNotificationFunc) cd_app_menu_on_name_changed, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_CREATED,
		(GldiNotificationFunc) cd_app_menu_on_new_appli, myApplet);
	
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) on_mouse_moved, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_UPDATE_SLOW,
		(GldiNotificationFunc) cd_app_menu_on_update_container, myApplet);
	
	cd_app_menu_stop ();

	// mouse events
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DOUBLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	if (myConfig.bDisplayMenu)
		gldi_object_unref (GLDI_OBJECT(myData.pKeyBinding));
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
		
		if (CD_APPLET_MY_OLD_CONTAINER != myContainer)
		{
			gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
				NOTIFICATION_MOUSE_MOVED,
				(GldiNotificationFunc) on_mouse_moved, myApplet);
			gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
				NOTIFICATION_UPDATE_SLOW,
				(GldiNotificationFunc) cd_app_menu_on_update_container, myApplet);
			
			gldi_object_register_notification (myContainer,
				NOTIFICATION_MOUSE_MOVED,
				(GldiNotificationFunc) on_mouse_moved,
				GLDI_RUN_AFTER, myApplet);
			gldi_object_register_notification (myContainer,
				NOTIFICATION_UPDATE_SLOW,
				(GldiNotificationFunc) cd_app_menu_on_update_container,
				GLDI_RUN_AFTER, myApplet);
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
		myData.iAnimIterMin = myData.iAnimIterMax = myData.iAnimIterClose = 0;
		myData.bButtonAnimating = FALSE;
		GldiWindowActor *pActiveWindow = myData.pCurrentWindow;
		myData.pCurrentWindow = NULL;
		cd_app_menu_set_current_window (pActiveWindow);
		
		// shortkey
		if (myConfig.bDisplayMenu)
		{
			if (myData.pKeyBinding)
				gldi_shortkey_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
			else
				myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
					D_("Show/hide the current application menu"),
					"Configuration", "shortkey",
					(CDBindkeyHandler) cd_app_menu_on_keybinding_pull);
		}
		else if (myData.pKeyBinding)
		{
			gldi_object_unref (GLDI_OBJECT(myData.pKeyBinding));
		}
		
		cairo_dock_set_icon_static (myIcon, myConfig.bDisplayControls);
	}
	
	if (myConfig.bDisplayControls)
	{
		myData.bReversedButtonsOrder = _reversed_buttons_order ();
		cd_app_menu_resize ();
	}
	
CD_APPLET_RELOAD_END

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
#include "applet-struct.h"
#include "applet-host.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("Status-Notifier",
	2, 3, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("A <b>notification area</b> for your dock\n"
	"Programs can use it to display their current status.\n"
	"If a program doesn't appear inside when it should, it's probably because it doesn't support this feature yet. Please fill a bug report to the devs."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_REDEFINE_TITLE (N_("Notification Area"))
CD_APPLET_DEFINE_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! CD_APPLET_RESERVE_DATA_SLOT ())
		return;
	
	if (myConfig.bCompactMode)
		CD_APPLET_SET_STATIC_ICON;
	
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (!myConfig.bCompactMode && myDock)
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	// CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	/*gldi_object_register_notification (&myIconsMgr,
		CAIRO_DOCK_ENTER_ICON,
		(GldiNotificationFunc) cd_status_notifier_on_enter_icon,
		GLDI_RUN_AFTER, myApplet);*/
	gldi_object_register_notification (&myContainersMgr,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(GldiNotificationFunc) cd_status_notifier_on_right_click,
		GLDI_RUN_FIRST, myApplet);
	
	if (myConfig.bCompactMode)
	{
		gldi_object_register_notification (myContainer,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) on_mouse_moved,
			GLDI_RUN_AFTER, myApplet);
		if (myDesklet)
		{
			gldi_object_register_notification (myContainer,
				NOTIFICATION_RENDER,
				(GldiNotificationFunc) on_render_desklet,
				GLDI_RUN_AFTER, myApplet);
			gldi_object_register_notification (myContainer,
				NOTIFICATION_UPDATE,
				(GldiNotificationFunc) on_update_desklet,
				GLDI_RUN_AFTER, myApplet);
			gldi_object_register_notification (myContainer,
				NOTIFICATION_LEAVE_DESKLET,
				(GldiNotificationFunc) on_leave_desklet,
				GLDI_RUN_AFTER, myApplet);
		}
	}
	
	myData.iDefaultWidth = myIcon->image.iWidth;
	myData.iDefaultHeight = myIcon->image.iHeight;
	cd_debug ("=== default size: %dx%d", myData.iDefaultWidth, myData.iDefaultHeight);
	
	cd_satus_notifier_launch_service ();
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	gldi_object_remove_notification (&myContainersMgr,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(GldiNotificationFunc) cd_status_notifier_on_right_click,
		myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) on_mouse_moved, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) on_render_desklet, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_UPDATE,
		(GldiNotificationFunc) on_update_desklet, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_LEAVE_DESKLET,
		(GldiNotificationFunc) on_leave_desklet, myApplet);
	
	cd_satus_notifier_stop_service ();
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	myData.iDefaultWidth = myIcon->image.iWidth;
	myData.iDefaultHeight = myIcon->image.iHeight;
	cd_debug ("=== default size <- %dx%d", myData.iDefaultWidth, myData.iDefaultHeight);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) on_mouse_moved, myApplet);
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_RENDER,
			(GldiNotificationFunc) on_render_desklet, myApplet);
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_UPDATE,
			(GldiNotificationFunc) on_update_desklet, myApplet);
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_LEAVE_DESKLET,
			(GldiNotificationFunc) on_leave_desklet, myApplet);
		
		if (myConfig.bCompactMode)
		{
			gldi_object_register_notification (myContainer,
				NOTIFICATION_MOUSE_MOVED,
				(GldiNotificationFunc) on_mouse_moved,
				GLDI_RUN_AFTER, myApplet);
			if (myDesklet)
			{
				gldi_object_register_notification (myContainer,
					NOTIFICATION_RENDER,
					(GldiNotificationFunc) on_render_desklet,
					GLDI_RUN_AFTER, myApplet);
				gldi_object_register_notification (myContainer,
					NOTIFICATION_UPDATE,
					(GldiNotificationFunc) on_update_desklet,
					GLDI_RUN_AFTER, myApplet);
				gldi_object_register_notification (myContainer,
					NOTIFICATION_LEAVE_DESKLET,
					(GldiNotificationFunc) on_leave_desklet,
					GLDI_RUN_AFTER, myApplet);
			}
		}
		
		if (myConfig.bCompactMode)
		{
			if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
			{
				CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
			}
			CD_APPLET_DELETE_MY_ICONS_LIST;
			if (myDock)  // on ne veut pas d'un sous-dock vide.
			{
				gldi_object_unref (GLDI_OBJECT(myIcon->pSubDock));
				myIcon->pSubDock = NULL;
			}
			cd_satus_notifier_reload_compact_mode ();
			CD_APPLET_SET_STATIC_ICON;
		}
		else
		{
			// reload all icons
			CD_APPLET_DELETE_MY_ICONS_LIST;
			myData.iItemSize = 0;  // unvalidate the grid.
			
			cd_satus_notifier_load_icons_from_items ();
			
			if (myDock)
				CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		}
	}
	else  // applet resized
	{
		if (myConfig.bCompactMode)
		{
			cd_satus_notifier_reload_compact_mode ();
		}
	}
CD_APPLET_RELOAD_END

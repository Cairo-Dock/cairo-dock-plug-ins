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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-host.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Status Notifier"),
	2, 2, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	("A <b>notification area</b> for your dock\n"
	"Also called 'systray'.\n"
	"It is designed to work on any desktop that supports the latest systray specifications (KDE, Gnome, etc)"),
	"Fabounet (Fabrice Rey)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
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
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	/*cairo_dock_register_notification_on_object (&myIconsMgr,
		CAIRO_DOCK_ENTER_ICON,
		(CairoDockNotificationFunc) cd_status_notifier_on_enter_icon,
		CAIRO_DOCK_RUN_AFTER, myApplet);*/
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(CairoDockNotificationFunc) cd_status_notifier_on_right_click,
		CAIRO_DOCK_RUN_FIRST, myApplet);
	
	cd_satus_notifier_launch_service ();
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(CairoDockNotificationFunc) cd_status_notifier_on_right_click,
		myApplet);
	cd_satus_notifier_stop_service ();
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.bCompactMode)
		{
			if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
			{
				CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
			}
			CD_APPLET_DELETE_MY_ICONS_LIST;
			cd_satus_notifier_reload_compact_mode ();
			
		}
		else
		{
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

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
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-bookmarks.h"
#include "applet-load-icons.h"
#include "applet-disk-usage.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("shortcuts"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FILES,
	N_("An applet that let you access quickly to all of your shortcuts.\n"
	"It can manage disks, network points, and Nautilus bookmarks (even if you don't have Nautilus).\n"
	"Drag and drop a folder on the main icon or the sub-dock to add a bookmark.\n"
	"Middle-click on the main icon to show your home folder.\n"
	"Middle-click on a mounting point icon to (un)mount is quickly.\n"
	"The applet can also display valuable information about your disks, like free space, type, etc."),
	"Fabounet (Fabrice Rey) &amp; Jackass (Benjamin SANS)")


CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	if (myDock)
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	//\_______________ On charge les icones dans un sous-dock.
	myData.pTask = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_shortcuts_get_shortcuts_data,
		(CairoDockUpdateSyncFunc) cd_shortcuts_build_shortcuts_from_data,
		myApplet);
	cairo_dock_launch_task (myData.pTask);
	
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		CAIRO_DOCK_RUN_FIRST, myApplet);  // on se met en premier pour pas que le dock essaye de lancer nos icones.
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_STOP_ICON,
		(CairoDockNotificationFunc) cd_shortcuts_free_data,
		CAIRO_DOCK_RUN_AFTER, myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_STOP_ICON,
		(CairoDockNotificationFunc) cd_shortcuts_free_data, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On charge les icones dans un sous-dock.
		cd_shortcuts_reset_all_datas (myApplet);  // stoppe toutes les mesures et remet myData a 0.
		
		if (myDock)
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		myData.pTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) cd_shortcuts_get_shortcuts_data,
			(CairoDockUpdateSyncFunc) cd_shortcuts_build_shortcuts_from_data,
			myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
CD_APPLET_RELOAD_END


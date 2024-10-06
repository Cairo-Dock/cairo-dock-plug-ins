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


CD_APPLET_DEFINITION2 (N_("shortcuts"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_FILES,
	N_("An applet that let you access quickly to all of your shortcuts.\n"
	"It can manage disks, network points, and Nautilus bookmarks (even if you don't have Nautilus).\n"
	"Drag and drop a folder on the main icon or the sub-dock to add a bookmark.\n"
	"Middle-click on the main icon to show your home folder.\n"
	"Middle-click on a mounting point icon to (un)mount is quickly.\n"
	"The applet can also display valuable information about your disks, like free space, type, etc."),
	"Fabounet (Fabrice Rey) &amp; Jackass (Benjamin SANS)")


CD_APPLET_INIT_BEGIN
	//\_______________ get a slot for our private data (for disk usage)
	if (! CD_APPLET_RESERVE_DATA_SLOT ())
		return;
	
	//\_______________ // set the default icon if none is specified in conf.
	if (myDock)  // in desklet, we use the "Slide" view, so we don't display the main icon.
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	
	//\_______________ get and load all the icons in a sub-dock.
	cd_shortcuts_start (myApplet);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_shortcuts_free_data,
		GLDI_RUN_AFTER, myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_shortcuts_free_data, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ reset all data (including sub-dock).
		cd_shortcuts_reset_all_datas (myApplet);  // stoppe toutes les mesures et remet myData a 0.
		
		//\_______________ // set the default icon if none is specified in conf.
		if (myDock)  // in desklet, we use the "Slide" view, so we don't display the main icon.
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
		
		//\_______________ get and load all the icons in a sub-dock.
		cd_shortcuts_start (myApplet);
	}
CD_APPLET_RELOAD_END


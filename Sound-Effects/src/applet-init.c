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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-sound.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Sound Effects"),
	3, 1, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FUN,
	N_("This plug-in add sound effects on various events in the dock:\n"
	" When clicking an icon, when hovering an icon, etc"),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;	
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END


static void _register_notifications (void)
{
	if (myConfig.bPlayOnClick)
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_CLICK_ICON,
			(GldiNotificationFunc) cd_sound_on_click,
			GLDI_RUN_FIRST, NULL);
	
	if (myConfig.bPlayOnMiddleClick)
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_MIDDLE_CLICK_ICON,
			(GldiNotificationFunc) cd_sound_on_middle_click,
			GLDI_RUN_FIRST, NULL);
	
	if (myConfig.bPlayOnHover)
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_ENTER_ICON,
			(GldiNotificationFunc) cd_sound_on_hover,
			GLDI_RUN_FIRST, NULL);
}

static void _unregister_notifications (void)
{
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) cd_sound_on_click, NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_MIDDLE_CLICK_ICON,
		(GldiNotificationFunc) cd_sound_on_middle_click, NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_sound_on_hover, NULL);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	
	_register_notifications ();

CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	// unregister from events
	_unregister_notifications ();
	
	// stop current tasks.
	cd_sound_free_current_tasks ();
	
	// delete sound files
	cd_sound_free_sound_file (myData.pOnClickSound);
	cd_sound_free_sound_file (myData.pOnMiddleClickSound);
	cd_sound_free_sound_file (myData.pOnHoverSound);
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// stop current tasks.
		cd_sound_free_current_tasks ();
		
		_unregister_notifications ();
		_register_notifications ();
		
		// delete sound files so that they are re-loaded on event
		cd_sound_free_sound_file (myData.pOnClickSound);
		myData.pOnClickSound = NULL;
		cd_sound_free_sound_file (myData.pOnMiddleClickSound);
		myData.pOnMiddleClickSound = NULL;
		cd_sound_free_sound_file (myData.pOnHoverSound);
		myData.pOnHoverSound = NULL;
	}
CD_APPLET_RELOAD_END

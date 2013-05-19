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
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("motion blur"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FUN,
	N_("This plugin adds a motion blur effect to docks."),
	"Fabounet (Fabrice Rey)")
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END



//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! g_bUseOpenGL || ! CD_APPLET_RESERVE_DATA_SLOT ())
		return;
	
	gldi_object_register_notification (&myDocksMgr,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) cd_motion_blur_pre_render,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myDocksMgr,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) cd_motion_blur_post_render,
		GLDI_RUN_AFTER, NULL);
	if (myConfig.bAlways)
		gldi_object_register_notification (&myContainersMgr,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) cd_motion_blur_mouse_moved,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myDocksMgr,
		NOTIFICATION_ENTER_DOCK,
		(GldiNotificationFunc) cd_motion_blur_mouse_moved,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myDocksMgr,
		NOTIFICATION_UPDATE,
		(GldiNotificationFunc) cd_motion_blur_update_dock,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myDocksMgr,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_motion_free_data,
		GLDI_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
static void _free_data_on_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cd_motion_free_data (NULL, pDock);
}
CD_APPLET_STOP_BEGIN
	gldi_object_remove_notification (&myDocksMgr,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) cd_motion_blur_pre_render, NULL);
	gldi_object_remove_notification (&myDocksMgr,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) cd_motion_blur_post_render, NULL);
	if (myConfig.bAlways)
		gldi_object_remove_notification (&myContainersMgr,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) cd_motion_blur_mouse_moved, NULL);
	gldi_object_remove_notification (&myDocksMgr,
		NOTIFICATION_ENTER_DOCK,
		(GldiNotificationFunc) cd_motion_blur_mouse_moved, NULL);
	gldi_object_remove_notification (&myDocksMgr,
		NOTIFICATION_UPDATE,
		(GldiNotificationFunc) cd_motion_blur_update_dock, NULL);
	gldi_object_remove_notification (&myDocksMgr,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_motion_free_data, NULL);
	
	cairo_dock_foreach_docks ((GHFunc) _free_data_on_dock, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	// rien a faire.
	
CD_APPLET_RELOAD_END

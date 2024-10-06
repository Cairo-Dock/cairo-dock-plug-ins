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

#include "evaporate-tex.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN (N_("illusion"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This plugin provides animations for the appearance &amp; disappearance of icons."),
	"Fabounet (Fabrice Rey)")
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! g_bUseOpenGL || ! CD_APPLET_RESERVE_DATA_SLOT ())
		return;
	
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_REMOVE_ICON,
		(GldiNotificationFunc) cd_illusion_on_remove_insert_icon,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_INSERT_ICON,
		(GldiNotificationFunc) cd_illusion_on_remove_insert_icon,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_UPDATE_ICON,
		(GldiNotificationFunc) cd_illusion_update_icon ,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_illusion_render_icon,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_STOP_ICON,
		(GldiNotificationFunc) cd_illusion_free_data,
		GLDI_RUN_AFTER, NULL);
CD_APPLET_INIT_END

static void _free_data_on_icon (Icon *pIcon, G_GNUC_UNUSED gpointer data)
{
	cd_illusion_free_data (NULL, pIcon);
}
//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	gldi_object_remove_notification (&myDockObjectMgr,
		NOTIFICATION_REMOVE_ICON,
		(GldiNotificationFunc) cd_illusion_on_remove_insert_icon, NULL);
	gldi_object_remove_notification (&myDockObjectMgr,
		NOTIFICATION_INSERT_ICON,
		(GldiNotificationFunc) cd_illusion_on_remove_insert_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_UPDATE_ICON,
		(GldiNotificationFunc) cd_illusion_update_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_illusion_render_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_STOP_ICON,
		(GldiNotificationFunc) cd_illusion_free_data, NULL);
	
	gldi_icons_foreach ((GldiIconFunc)_free_data_on_icon, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	// rien a faire, l'animation eventuellement en cours se poursuivra inchangee.
	
CD_APPLET_RELOAD_END

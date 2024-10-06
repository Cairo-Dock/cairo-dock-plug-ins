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


CD_APPLET_DEFINE2_BEGIN (N_("show mouse"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS | CAIRO_DOCK_MODULE_REQUIRES_OPENGL,
	CAIRO_DOCK_CATEGORY_APPLET_FUN,
	N_("This plugin draws some animations around the cursor when it's inside a dock/desklet."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END


#define _cd_mouse_register_on_dock(...) \
	gldi_object_register_notification (&myDockObjectMgr, NOTIFICATION_ENTER_DOCK, (GldiNotificationFunc) cd_show_mouse_enter_container, GLDI_RUN_AFTER, NULL);\
	gldi_object_register_notification (&myDockObjectMgr, NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_show_mouse_update_container, GLDI_RUN_AFTER, NULL);\
	gldi_object_register_notification (&myDockObjectMgr, NOTIFICATION_RENDER, (GldiNotificationFunc) cd_show_mouse_render, GLDI_RUN_AFTER, NULL);

#define _cd_mouse_register_on_desklet(...) \
	gldi_object_register_notification (&myDeskletObjectMgr, NOTIFICATION_ENTER_DESKLET, (GldiNotificationFunc) cd_show_mouse_enter_container, GLDI_RUN_AFTER, NULL);\
	gldi_object_register_notification (&myDeskletObjectMgr, NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_show_mouse_update_container, GLDI_RUN_AFTER, NULL);\
	gldi_object_register_notification (&myDeskletObjectMgr, NOTIFICATION_RENDER, (GldiNotificationFunc) cd_show_mouse_render, GLDI_RUN_AFTER, NULL);

#define _cd_mouse_unregister_from_dock(...) \
	gldi_object_remove_notification (&myDockObjectMgr, NOTIFICATION_RENDER, (GldiNotificationFunc) cd_show_mouse_render, NULL);\
	gldi_object_remove_notification (&myDockObjectMgr, NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_show_mouse_update_container, NULL);\
	gldi_object_remove_notification (&myDockObjectMgr, NOTIFICATION_ENTER_DOCK, (GldiNotificationFunc) cd_show_mouse_enter_container, NULL);

#define _cd_mouse_unregister_from_desklet(...) \
	gldi_object_remove_notification (&myDeskletObjectMgr, NOTIFICATION_RENDER, (GldiNotificationFunc) cd_show_mouse_render, NULL);\
	gldi_object_remove_notification (&myDeskletObjectMgr, NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_show_mouse_update_container, NULL);\
	gldi_object_remove_notification (&myDeskletObjectMgr, NOTIFICATION_ENTER_DESKLET, (GldiNotificationFunc) cd_show_mouse_enter_container, NULL);

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! CD_APPLET_RESERVE_DATA_SLOT())
		return;
	
	if (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DOCK)
	{
		_cd_mouse_register_on_dock ();
	}
	
	if (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DESKLET)
	{
		_cd_mouse_register_on_desklet ();
	}
	myData.iContainerType = myConfig.iContainerType;
	
	gldi_object_register_notification (&myDockObjectMgr, NOTIFICATION_DESTROY, (GldiNotificationFunc) cd_show_mouse_free_data, GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myDeskletObjectMgr, NOTIFICATION_DESTROY, (GldiNotificationFunc) cd_show_mouse_free_data, GLDI_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
static void _free_dock_data (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cd_show_mouse_free_data (data, CAIRO_CONTAINER (pDock));
}
static gboolean _free_desklet_data (CairoDesklet *pDesklet, gpointer data)
{
	cd_show_mouse_free_data (data, CAIRO_CONTAINER (pDesklet));
	return FALSE;
}
CD_APPLET_STOP_BEGIN
	_cd_mouse_unregister_from_dock ();
	_cd_mouse_unregister_from_desklet ();
	gldi_object_remove_notification (&myDockObjectMgr, NOTIFICATION_DESTROY, (GldiNotificationFunc) cd_show_mouse_free_data, NULL);
	gldi_object_remove_notification (&myDeskletObjectMgr, NOTIFICATION_DESTROY, (GldiNotificationFunc) cd_show_mouse_free_data, NULL);
	
	gldi_docks_foreach ((GHFunc)_free_dock_data, NULL);
	gldi_desklets_foreach (_free_desklet_data, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.iContainerType != myData.iContainerType)
		{
			if ((myConfig.iContainerType & CD_SHOW_MOUSE_ON_DOCK) && ! (myData.iContainerType & CD_SHOW_MOUSE_ON_DOCK))
			{
				_cd_mouse_register_on_dock ();
			}
			if (! (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DOCK) && (myData.iContainerType & CD_SHOW_MOUSE_ON_DOCK))
			{
				_cd_mouse_unregister_from_dock ();
			}
			if ((myConfig.iContainerType & CD_SHOW_MOUSE_ON_DESKLET) && ! (myData.iContainerType & CD_SHOW_MOUSE_ON_DESKLET))
			{
				_cd_mouse_register_on_desklet ();
			}
			if (! (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DESKLET) && (myData.iContainerType & CD_SHOW_MOUSE_ON_DESKLET))
			{
				_cd_mouse_unregister_from_desklet ();
			}
			myData.iContainerType = myConfig.iContainerType;
		}
	}
CD_APPLET_RELOAD_END

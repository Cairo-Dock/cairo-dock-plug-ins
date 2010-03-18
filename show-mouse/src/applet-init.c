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


CD_APPLET_PRE_INIT_BEGIN(N_("show mouse"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_PLUG_IN,
	N_("This plugin draws some animations around the cursor when it's inside a dock/desklet."),
	"Fabounet (Fabrice Rey)")
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
CD_APPLET_PRE_INIT_END


#define _cd_mouse_register_on_dock(...) \
	cairo_dock_register_notification (CAIRO_DOCK_ENTER_DOCK, (CairoDockNotificationFunc) cd_show_mouse_enter_container, CAIRO_DOCK_RUN_AFTER, NULL);\
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_show_mouse_update_container, CAIRO_DOCK_RUN_AFTER, NULL);\
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_show_mouse_render, CAIRO_DOCK_RUN_AFTER, NULL);

#define _cd_mouse_register_on_desklet(...) \
	cairo_dock_register_notification (CAIRO_DOCK_ENTER_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_enter_container, CAIRO_DOCK_RUN_AFTER, NULL);\
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_update_container, CAIRO_DOCK_RUN_AFTER, NULL);\
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_render, CAIRO_DOCK_RUN_AFTER, NULL);

#define _cd_mouse_unregister_from_dock(...) \
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_show_mouse_render, NULL);\
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_show_mouse_update_container, NULL);\
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_DOCK, (CairoDockNotificationFunc) cd_show_mouse_enter_container, NULL);

#define _cd_mouse_unregister_from_desklet(...) \
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_render, NULL);\
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_update_container, NULL);\
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_enter_container, NULL);

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
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
	
	cairo_dock_register_notification (CAIRO_DOCK_STOP_DOCK, (CairoDockNotificationFunc) cd_show_mouse_free_data, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_free_data, CAIRO_DOCK_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
static void _free_dock_data (gchar *cDockName, CairoDock *pDock, gpointer data)
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
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_DOCK, (CairoDockNotificationFunc) cd_show_mouse_free_data, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_DESKLET, (CairoDockNotificationFunc) cd_show_mouse_free_data, NULL);
	
	cairo_dock_foreach_docks (_free_dock_data, NULL);
	cairo_dock_foreach_desklet (_free_desklet_data, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.iContainerType != myData.iContainerType)
		{
			if ((myConfig.iContainerType & CD_SHOW_MOUSE_ON_DOCK) & ! (myData.iContainerType & CD_SHOW_MOUSE_ON_DOCK))
			{
				_cd_mouse_register_on_dock ();
			}
			if (! (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DOCK) & (myData.iContainerType & CD_SHOW_MOUSE_ON_DOCK))
			{
				_cd_mouse_unregister_from_dock ();
			}
			if ((myConfig.iContainerType & CD_SHOW_MOUSE_ON_DESKLET) & ! (myData.iContainerType & CD_SHOW_MOUSE_ON_DESKLET))
			{
				_cd_mouse_register_on_desklet ();
			}
			if (! (myConfig.iContainerType & CD_SHOW_MOUSE_ON_DESKLET) & (myData.iContainerType & CD_SHOW_MOUSE_ON_DESKLET))
			{
				_cd_mouse_unregister_from_desklet ();
			}
			myData.iContainerType = myConfig.iContainerType;
		}
	}
CD_APPLET_RELOAD_END

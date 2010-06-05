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


CD_APPLET_DEFINE_BEGIN (N_("drop indicator"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This plug-in displays an animated indicator when you drop something into the dock."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
	CD_APPLET_ATTACH_TO_INTERNAL_MODULE ("Indicators");
CD_APPLET_DEFINE_END


static void _load_indicators (void)
{
	double fMaxScale = cairo_dock_get_max_scale (g_pMainDock);
	double iInitialWidth = myIcons.tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] * fMaxScale;
	double iInitialHeight = myIcons.tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] * fMaxScale / 2;
	
	cd_drop_indicator_load_drop_indicator (myConfig.cDropIndicatorImageName,
		iInitialWidth,
		iInitialHeight);
	
	cd_drop_indicator_load_hover_indicator (myConfig.cHoverIndicatorImageName,
		iInitialWidth/3,
		iInitialHeight*2/3);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	_load_indicators ();
	
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_render, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) cd_drop_indicator_mouse_moved, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_update_dock, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_stop_dock, CAIRO_DOCK_RUN_AFTER, NULL);

CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
static void _free_data_on_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cd_drop_indicator_stop_dock (NULL, pDock);
}
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_render, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) cd_drop_indicator_mouse_moved, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_update_dock, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_stop_dock, NULL);
	
	cairo_dock_foreach_docks ((GHFunc)_free_data_on_dock, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cd_drop_indicator_free_buffers ();
		_load_indicators ();
	}
	
CD_APPLET_RELOAD_END

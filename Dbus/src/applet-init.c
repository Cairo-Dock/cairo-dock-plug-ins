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
#include "applet-dbus.h"
#include "applet-struct.h"
#include "interface-applet-signals.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("Dbus",
	2, 0, 9,
	CAIRO_DOCK_CATEGORY_PLUG_IN,
	N_("This plug-in lets extern applications interact on the dock.\n"
	"The communication between both sides is based on Dbus"),
	"Necropotame & Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	cd_dbus_launch_service ();
	cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
		CAIRO_DOCK_RUN_FIRST,
		NULL);  // on enregistre cette notification ici car elle gere le drop d'applets.
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
		NULL);
	if (myData.iSidRemoveAppletFromConf != 0)
		g_source_remove (myData.iSidRemoveAppletFromConf);
	cd_dbus_stop_service ();
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	
CD_APPLET_RELOAD_END

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
	2, 2, 1,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This plug-in lets extern applications interact on the dock.\n"
	"The communication between both sides is based on Dbus"),
	"Necropotame & Fabounet")
	pInterface->initModule = CD_APPLET_INIT_FUNC;  // no stop method -> the plug-in will be auto-loaded.
	pInterface->read_conf_file = CD_APPLET_READ_CONFIG_FUNC;  /// inutile je pense...
	CD_APPLET_REDEFINE_TITLE ("DBus");
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	cd_dbus_launch_service ();
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
		CAIRO_DOCK_RUN_FIRST,
		NULL);  // to register new applets by dropping a package on the dock.
CD_APPLET_INIT_END

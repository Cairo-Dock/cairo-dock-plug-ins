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

static AppletData s_myDataCopy;

static void cd_dbus_save_my_data (GldiModuleInstance *myApplet);


CD_APPLET_DEFINE2_BEGIN ("Dbus",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This plug-in lets extern applications interact on the dock.\n"
	"The communication between both sides is based on Dbus"),
	"Necropotame & Fabounet")
	pInterface->initModule = CD_APPLET_INIT_FUNC;  // no stop method -> the plug-in will be auto-loaded.
	pInterface->read_conf_file = CD_APPLET_READ_CONFIG_FUNC;
	pInterface->reset_config =  CD_APPLET_RESET_CONFIG_FUNC; // needed to correctly reload the config if Reboot is called
	//!! TODO: this might lead to a crash if a method call happens when this applet has been deactivated !!
	/// -> we should add a proper stop function and handle reinitializing
	pInterface->reset_data = cd_dbus_save_my_data;
	CD_APPLET_REDEFINE_TITLE ("DBus");
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	static gboolean s_bInitialized = FALSE;
	if (! cairo_dock_dbus_get_owned_name ()) return; // if we don't own a DBus name, we cannot register
	if (! CD_APPLET_RESERVE_DATA_SLOT ())
		return;
	if (!s_bInitialized)  // since the service lives on the bus, only launch it once.
	{
		s_bInitialized = TRUE;
		cd_dbus_launch_service ();
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_DROP_DATA,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_drop_data,
			GLDI_RUN_FIRST,
			NULL);  // to register new applets by dropping a package on the dock.
	}
	else
	{
		memcpy (myDataPtr, &s_myDataCopy, sizeof (AppletData));
		cd_dbus_clean_up_processes (TRUE);  // TRUE <=> including applets spawned from our process.
	}
CD_APPLET_INIT_END


static void cd_dbus_save_my_data (GldiModuleInstance *myApplet)
{
	memcpy (&s_myDataCopy, myDataPtr, sizeof (AppletData));
}

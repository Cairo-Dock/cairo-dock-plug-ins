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

#include <stdlib.h>

#include "dbus-shared-names.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-me.h"
#include "applet-menu.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Me Menu"),
	2, 2, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("A menu that lets you access quickly to your information, your online status, your friends."),
	"Fabounet")


/**static gboolean _get_menu_once (CairoDockModuleInstance *myApplet)
{
	cd_me_get_menu (myApplet);
	myData.iSidGetMenuOnce = 0;
	return FALSE;
}*/
//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	myData.pIndicator = cd_indicator_new (myApplet,
		INDICATOR_ME_DBUS_NAME,
		INDICATOR_ME_SERVICE_DBUS_OBJECT,
		INDICATOR_ME_SERVICE_DBUS_INTERFACE,
		INDICATOR_ME_DBUS_OBJECT);	
	myData.pIndicator->on_connect 			= cd_me_on_connect;
	myData.pIndicator->on_disconnect 		= cd_me_on_disconnect;
	myData.pIndicator->get_initial_values 	= cd_me_get_initial_values;
	myData.pIndicator->add_menu_handler 	= cd_me_add_menu_handler;
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_indicator_destroy (myData.pIndicator);
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	cd_indicator_reload_icon (myData.pIndicator);  // on remet l'icone (si avant on n'avait pas d'icone, on a mis un chemin. Il ne prendra pas en compte un changement de theme d'icone, donc on remet l'icone originale).
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet)  // we are in desklet mode now, set a desklet renderer
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
CD_APPLET_RELOAD_END

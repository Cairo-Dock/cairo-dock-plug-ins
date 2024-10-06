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
#ifndef INDICATOR_MESSAGES_WITH_IND3
#include "applet-messaging.h"
#include "applet-menu.h"
#else
#include "applet-indicator3.h"
#endif
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN (N_("Messaging Menu"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_INTERNET,
	N_("A menu that notices you about new messages from Mail or Chat applications.\n"
	"It handles Evolution, Pidgin, Empathy, etc\n"
	"It requires the Messaging service, which is available on Ubuntu by default."),
	"Fabounet &amp; Matthieu Baerts")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE2_END

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}

	#ifndef INDICATOR_MESSAGES_WITH_IND3
	myData.pIndicator = cd_indicator_new (myApplet,
		INDICATOR_MESSAGES_DBUS_NAME,
		INDICATOR_MESSAGES_DBUS_SERVICE_OBJECT,
		INDICATOR_MESSAGES_DBUS_SERVICE_INTERFACE,
		INDICATOR_MESSAGES_DBUS_OBJECT,
		INDICATOR_APPLET_DEFAULT_VERSION);
	myData.pIndicator->on_connect 			= cd_messaging_on_connect;
	myData.pIndicator->on_disconnect 		= cd_messaging_on_disconnect;
	myData.pIndicator->get_initial_values 	= cd_messaging_get_initial_values;
	myData.pIndicator->add_menu_handler 	= cd_messaging_add_menu_handler;
	#else
	if (myDock)
		gldi_icon_detach (myIcon); // the icon is inserted when the entry will be added

	myData.pIndicator = cd_indicator3_load (myConfig.cIndicatorName,
		cd_messaging_entry_added,
		cd_messaging_entry_removed,
		cd_messaging_accessible_desc_update,
		NULL, // menu show
		myApplet);

	// check other names (if the one in the config file is wrong or switch ng)
	if (! myData.pIndicator)
	{
		const gchar *cIndicatorNames[] = {"com.canonical.indicator.messages",
			"libmessaging.so", NULL};
		int i = 0;
		do
		{
			// no need to reload the same wrong indicator twice.
			if (strcmp (cIndicatorNames[i], myConfig.cIndicatorName) != 0)
			{
				myData.pIndicator = cd_indicator3_load (cIndicatorNames[i],
					cd_messaging_entry_added,
					cd_messaging_entry_removed,
					cd_messaging_accessible_desc_update,
					NULL, // menu show
					myApplet);
			}
			i++;
		} while (myData.pIndicator == NULL && cIndicatorNames[i] != NULL);

		if (! myData.pIndicator)
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	}
	#endif
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	// CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// keyboard events
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Show/hide the Messaging menu"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_messaging_on_keybinding_pull);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	// mouse events
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	// keyboard events
	gldi_object_unref (GLDI_OBJECT(myData.pKeyBinding));

	#ifndef INDICATOR_MESSAGES_WITH_IND3
	cd_indicator_destroy (myData.pIndicator);
	#else
	// It seems we doesn't need to free the indicator (object and event)
	cd_messaging_destroy (myData.pIndicator, myData.pEntry, myApplet); // remove the connection to signals (menu)
	cd_indicator3_unload (myData.pIndicator, // remove the connection to signals (indicator)
		cd_messaging_entry_added,
		cd_messaging_entry_removed,
		cd_messaging_accessible_desc_update,
		NULL,
		myApplet);
	#endif
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	#ifndef INDICATOR_MESSAGES_WITH_IND3
	cd_indicator_reload_icon (myData.pIndicator);  // we reload the icon (if we didn't have an icon, now we have a path). It will not consider a change of icon theme, so we return the original icon.
	#endif
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		#ifdef INDICATOR_MESSAGES_WITH_IND3
		// check if the name has changed and reload the icon
		cd_messaging_reload (myData.pIndicator, myData.pEntry, myApplet);
		#endif
		gldi_shortkey_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
CD_APPLET_RELOAD_END

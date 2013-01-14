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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-screenshot.h"
#include "applet-init.h"

CD_APPLET_DEFINITION ("Screenshot",
	3, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("A simple applet to quickly take screenshots.\n"
	"<b>Click</b> to take a screenshot.\n"
	"<b>Middle-click</b> gives you more options.\n"
	"You can bind a <b>shortcut</b> to the action."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// shortkey
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Take a screenshot"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_screenshot_on_keybinding_pull);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_screenshot_cancel ();
	cd_screenshot_free_apps_list (myApplet);
	
	cd_keybinder_unbind (myData.pKeyBinding);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		cd_keybinder_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
CD_APPLET_RELOAD_END

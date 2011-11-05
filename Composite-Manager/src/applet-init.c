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

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-composite-manager.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Composite-Manager"),
	2, 3, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("This applet allows you to <b>toggle the composite ON/OFF</b>.\n"
	"The composite is what allows transparency on the desktop, but it can slow down your PC, especially during games.\n"
	"<b>Click</b> on the icon to switch the composite ON/OFF. You can define a <b>shortcut</b> for this action.\n"
	"The applet also lets you acces to some actions of the Window-Manager, from <b>middle-click and the menu</b>.\n"
	"You can define in the configuration a Window-Manager that will provide the composite, and another as a fallback."),
	"Fabounet")


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	cd_init_wms ();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// keyboard events
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortcut,
		D_("Toggle the composite ON/OFF"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_on_keybinding_pull);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	cd_stop_wms ();
	
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	// shortkey
	cd_keybinder_unbind (myData.cKeyBinding);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		cd_define_prefered_wms ();
		
		cd_draw_current_state ();
		
		cd_keybinder_rebind (myData.cKeyBinding, myConfig.cShortcut, NULL);
	}
CD_APPLET_RELOAD_END

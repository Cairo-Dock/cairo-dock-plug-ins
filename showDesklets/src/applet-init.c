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


CD_APPLET_DEFINITION (N_("showDesklets"),
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("This applet let you access quickly to your desklets.\n"
	"Left click to show/hide your desklets.\n"
	"Basically, if you run under Compiz, you don't need this applet;\n"
	" you should just use the 'Widget Layer' capabilities of desklets."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_show_desklet_active_window_changed, CAIRO_DOCK_RUN_AFTER, myApplet);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage);
	cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull, (gpointer)NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_show_desklet_active_window_changed, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.bHide)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHideImage);
		}
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage);
		}
		
		cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull, (gpointer)NULL);
	}
CD_APPLET_RELOAD_END

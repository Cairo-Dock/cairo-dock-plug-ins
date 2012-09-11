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

#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__

#include <cairo-dock.h>


CD_APPLET_ON_CLICK_H


CD_APPLET_ON_MIDDLE_CLICK_H


CD_APPLET_ON_SCROLL_H


CD_APPLET_ON_DOUBLE_CLICK_H


// CD_APPLET_ON_BUILD_MENU_H

void cd_app_menu_on_keybinding_pull (const gchar *keystring, CairoDockModuleInstance *myApplet);

gboolean on_mouse_moved (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation);


gboolean cd_app_menu_on_active_window_changed (CairoDockModuleInstance *myApplet, Window *XActiveWindow);

gboolean cd_app_menu_on_state_changed (CairoDockModuleInstance *myApplet, Icon *pIcon, gboolean bHiddenChanged, gboolean bMaximizedChanged, gboolean bFullScreenChanged);

gboolean cd_app_menu_on_name_changed (CairoDockModuleInstance *myApplet, Icon *pIcon);

gboolean cd_app_menu_on_new_appli (CairoDockModuleInstance *myApplet, Icon *pIcon);

///gboolean cd_app_menu_on_property_changed (CairoDockModuleInstance *myApplet, Window Xid, Atom aProperty, int iState);


gboolean cd_app_menu_on_update_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation);

#endif

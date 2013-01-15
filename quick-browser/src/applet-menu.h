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


#ifndef __APPLET_MENU__
#define  __APPLET_MENU__

#include <cairo-dock.h>


CDQuickBrowserItem *cd_quick_browser_make_menu_from_dir (const gchar *cDirPath, CairoDockModuleInstance *myApplet);


void cd_quick_browser_free_apps_list (CairoDockModuleInstance *myApplet);


void cd_quick_browser_destroy_menu (CairoDockModuleInstance *myApplet);


void cd_quick_browser_show_menu (CairoDockModuleInstance *myApplet);


#endif

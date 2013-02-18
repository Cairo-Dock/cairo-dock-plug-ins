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

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

CD_APPLET_ON_SCROLL_H

gboolean on_change_active_window (CairoDockModuleInstance *myApplet, Window *XActiveWindow);

gboolean on_change_desktop (CairoDockModuleInstance *myApplet);

gboolean on_change_screen_geometry (CairoDockModuleInstance *myApplet);

gboolean on_window_configured (CairoDockModuleInstance *myApplet, Window Xid, XConfigureEvent *xconfigure);

gboolean on_change_desktop_names (CairoDockModuleInstance *myApplet);


gboolean on_mouse_moved (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation);

gboolean on_update_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation);

gboolean on_render_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext);

gboolean on_leave_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation);


#endif

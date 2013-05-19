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

gboolean on_change_desktop (GldiModuleInstance *myApplet);

gboolean on_change_screen_geometry (GldiModuleInstance *myApplet);

gboolean on_window_size_position_changed (GldiModuleInstance *myApplet, GldiWindowsManager *actor);

gboolean on_change_window_order (GldiModuleInstance *myApplet);

gboolean on_change_desktop_names (GldiModuleInstance *myApplet);


gboolean on_mouse_moved (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation);

gboolean on_update_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bContinueAnimation);

gboolean on_render_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, cairo_t *pCairoContext);

gboolean on_leave_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation);


#endif

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

#ifndef __APPLET_TRANSITIONS__
#define  __APPLET_TRANSITIONS__

#include <cairo-dock.h>

#include "applet-struct.h"

#define _get_frame_linewidth(myApplet) MIN (6, .1 * MIN (myData.iSurfaceWidth, myData.iSurfaceHeight))

void cd_slider_draw_default (CairoDockModuleInstance *myApplet);

gboolean cd_slider_fade (CairoDockModuleInstance *myApplet);

gboolean cd_slider_blank_fade (CairoDockModuleInstance *myApplet);

gboolean cd_slider_fade_in_out (CairoDockModuleInstance *myApplet);

gboolean cd_slider_side_kick (CairoDockModuleInstance *myApplet);

gboolean cd_slider_diaporama (CairoDockModuleInstance *myApplet);

gboolean cd_slider_grow_up (CairoDockModuleInstance *myApplet);

gboolean cd_slider_shrink_down (CairoDockModuleInstance *myApplet);

gboolean cd_slider_cube (CairoDockModuleInstance *myApplet);


#endif

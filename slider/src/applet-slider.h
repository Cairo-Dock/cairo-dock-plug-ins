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

#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__

#include <cairo-dock.h>

#include "applet-struct.h"


void cd_slider_jump_to_next_slide (GldiModuleInstance *myApplet);

void cd_slider_schedule_next_slide (GldiModuleInstance *myApplet);

#define cd_slider_next_slide_is_scheduled(myApplet) (myData.iTimerID != 0)


void cd_slider_start (GldiModuleInstance *myApplet, gboolean bDelay);

void cd_slider_stop (GldiModuleInstance *myApplet);


#endif

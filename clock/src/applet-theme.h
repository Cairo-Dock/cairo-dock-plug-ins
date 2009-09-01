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


#ifndef __CD_CLOCK_THEME__
#define  __CD_CLOCK_THEME__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_load_theme (CairoDockModuleInstance *myApplet);

void cd_clock_load_back_and_fore_ground (CairoDockModuleInstance *myApplet);

void cd_clock_load_textures (CairoDockModuleInstance *myApplet);

void cd_clock_clear_theme (CairoDockModuleInstance *myApplet, gboolean bClearAll);


#endif

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


#ifndef __CD_CLOCK_DIGITAL__
#define  __CD_CLOCK_DIGITAL__


#include <cairo-dock.h>
#include "applet-struct.h"

void cd_clock_configure_digital (CairoDockModuleInstance *myApplet);
void cd_clock_digital_load_frames (CairoDockModuleInstance *myApplet);
void cd_clock_draw_frames (CairoDockModuleInstance *myApplet);
void cd_clock_put_text_on_frames (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime);
void cd_clock_draw_ampm (CairoDockModuleInstance *myApplet, gchar *cMark);
void cd_clock_draw_text_from_surface (CairoDockModuleInstance *myApplet, int iNumber);
void cd_clock_fill_text_surface (CairoDockModuleInstance *myApplet, gchar *cStr, int iNumber);
void cd_clock_draw_date_on_frame (CairoDockModuleInstance *myApplet);


#endif

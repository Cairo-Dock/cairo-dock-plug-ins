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


#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>

#include <applet-struct.h>

gboolean cd_xmms_draw_icon (CairoDockModuleInstance *myApplet);

void cd_xmms_add_buttons_to_desklet(CairoDockModuleInstance *myApplet);

void cd_xmms_animate_icon(CairoDockModuleInstance *myApplet, int animationLength);
void cd_xmms_new_song_playing(CairoDockModuleInstance *myApplet);

void cd_xmms_set_surface (CairoDockModuleInstance *myApplet, MyPlayerStatus iStatus);

void cd_xmms_change_desklet_data (CairoDockModuleInstance *myApplet);
void cd_xmms_player_none (CairoDockModuleInstance *myApplet);

#endif

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

#ifndef __CD_3DCOVER_DRAW__
#define  __CD_3DCOVER_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_opengl_load_3D_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath);

void cd_opengl_reset_opengl_datas (CairoDockModuleInstance *myApplet);

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);

void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet);

int cd_opengl_check_buttons_state (CairoDockModuleInstance *myApplet);


#endif //__CD_3DCOVER_DRAW__

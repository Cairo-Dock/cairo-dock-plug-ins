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

#ifndef __TOMBOY_DRAW__
#define  __TOMBOY_DRAW__

#include <cairo-dock.h>


void cd_tomboy_load_note_surface (int iWidth, int iHeight);

void cd_tomboy_update_icon (void);

void cd_tomboy_draw_content_on_icon (cairo_t *pIconContext, Icon *pIcon);


void cd_tomboy_show_results (GList *pIconsList);

void cd_tomboy_reset_icon_marks (gboolean bForceRedraw);


#endif

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

#include "applet-struct.h"


gboolean cd_musicplayer_draw_icon (gpointer data);


void cd_musicplayer_popup_info (void);
void cd_musicplayer_animate_icon (int animationLength);

void cd_musicplayer_set_surface (MyPlayerStatus iStatus);

gboolean cd_musicplayer_check_size_is_constant (const gchar *cFilePath);

gboolean cd_musiplayer_set_cover_if_present (gboolean bCheckSize);


void cd_musicplayer_update_icon (gboolean bFirstTime);


#endif

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

#ifndef __RHYTHMBOX_DRAW__
#define  __RHYTHMBOX_DRAW__

#include <rhythmbox-struct.h>

void rhythmbox_add_buttons_to_desklet (void);

void rhythmbox_iconWitness(int animationLenght);

void update_icon (gboolean bCheckTwice);

void music_dialog(void);

void rhythmbox_set_surface (MyAppletPlayerStatus iStatus);

gboolean cd_check_if_size_is_constant (gchar *cFileName);

#endif

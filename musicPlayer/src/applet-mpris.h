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

#ifndef __APPLET_MPRIS__
#define  __APPLET_MPRIS__

#include <cairo-dock.h>
#include "applet-struct.h"


void onChangePlaying_mpris (DBusGProxy *player_proxy, GValueArray *status, gpointer data);  // used by Audacious too.

void cd_mpris_getPlaying (void);  // used by Audacious too.

void cd_mpris_get_time_elapsed (void);  // used by Audacious too.


MusicPlayerHandeler *cd_mpris_new_handler (void);


#endif

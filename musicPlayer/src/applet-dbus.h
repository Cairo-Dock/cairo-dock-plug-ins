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


#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>

#include "applet-struct.h"



gboolean cd_musicplayer_dbus_connect_to_bus(void);
gboolean musicplayer_dbus_connect_to_bus_Shell (void);
void musicplayer_dbus_disconnect_from_bus (void);
void musicplayer_dbus_disconnect_from_bus_Shell (void);
void cd_musicplayer_dbus_detect_player (void);


void cd_musicplayer_getStatus_string(const char*, const char*, const char*);
void cd_musicplayer_getStatus_integer (int, int);

void cd_musicplayer_getCoverPath (void);


MusicPlayerHandeler *cd_musicplayer_dbus_find_opened_player (void);


#endif

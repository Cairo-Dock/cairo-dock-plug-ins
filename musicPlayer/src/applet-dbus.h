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


gboolean cd_musicplayer_dbus_connect_handler (MusicPlayerHandler *pHandler);

void cd_musicplayer_dbus_disconnect_from_bus (void);


MusicPlayerHandler *cd_musicplayer_dbus_find_opened_player (void);


#endif

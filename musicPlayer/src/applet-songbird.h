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

#ifndef __APPLET_SONGBIRD__
#define  __APPLET_SONGBIRD__

#include <cairo-dock.h>

void cd_songbird_getSongInfos (void);

void cd_songbird_free_data (void);

void cd_songbird_control (MyPlayerControl pControl, char*);

void cd_songbird_load_dbus_commands (void);

gboolean cd_songbird_ask_control (MyPlayerControl pControl);

void cd_songbird_acquisition (void);

void cd_songbird_read_data (void);

void cd_musicplayer_register_songbird_handler (void);

#endif


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


#ifndef __APPLET_MUSICPLAYER__
#define  __APPLET_MUSICPLAYER__

#include <cairo-dock.h>

#include "applet-struct.h"


MusicPlayerHandler *cd_musicplayer_get_handler_by_name (const gchar *cName);

///MusicPlayerHandler *cd_musicplayer_get_handler_by_service (const gchar *cService);

void cd_musicplayer_launch_handler (void);

void cd_musicplayer_relaunch_handler (void);

void cd_musicplayer_stop_current_handler (gboolean bStopWatching);

void cd_musicplayer_register_my_handler (MusicPlayerHandler *pHandler);

void cd_musicplayer_free_handler (gpointer data);

void cd_musicplayer_set_current_handler (const gchar *cName);

gchar *cd_musicplayer_get_string_with_first_char_to_upper (const gchar *cName);


#endif

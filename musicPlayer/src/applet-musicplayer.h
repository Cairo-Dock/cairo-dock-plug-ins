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

/**
Set the current handler from its MPRIS2 service name.
@param cMpris2Service the DBus well-known name to use (required; set to NULL to set no handler)
@param cAppName the name to display (optional; will be retrieved from DBus)
@param cDesktopFileName the desktop file name (optional; will be retrieved from DBus)
@param bUpdateConfig whether to update the config file (set to FALSE when initially loading)
@param bUpdateIcon whether to update the icon surface
*/
void cd_musicplayer_set_current_handler (const gchar *cMpris2Service, const gchar *cAppName, const gchar *cDesktopFileName,
	gboolean bUpdateConfig, gboolean bUpdateIcon);

/** Update the .desktop file name of the current handler and save it in the config if needed. */
void cd_musicplayer_on_got_desktop_entry (const gchar *cDesktopFileName);

// in applet-notifications.c
const CDKnownMusicPlayer *cd_musicplayer_find_known_player (const gchar *cName);

#endif

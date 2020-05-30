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

#ifndef __APPLET_GENERIC__
#define  __APPLET_GENERIC__

#include <cairo-dock.h>


int cd_get_volume (void);

void cd_set_volume (int iVolume);

int cd_get_capture_volume (void);

void cd_set_capture_volume (int iVolume);

void cd_toggle_mute (void);

void cd_show_hide (void);

void cd_stop (void);

void cd_reload (void);

void cd_start (void);


GtkWidget *mixer_build_widget (gboolean bHorizontal);

void cd_mixer_set_volume_with_no_callback (GtkWidget *pScale, int iVolume);

#endif

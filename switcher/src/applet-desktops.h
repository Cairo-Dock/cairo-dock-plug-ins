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


#ifndef __APPLET_DESKTOPS__
#define  __APPLET_DESKTOPS__


#include <cairo-dock.h>


void cd_switcher_get_current_desktop (void);


void cd_switcher_compute_nb_lines_and_columns (void);

void cd_switcher_compute_coordinates_from_desktop (int iNumDesktop, int iNumViewportX, int iNumViewportY, int *iNumLine, int *iNumColumn);

void cd_switcher_compute_desktop_from_coordinates (int iNumLine, int iNumColumn, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY);


int cd_switcher_compute_index_from_desktop (int iNumDesktop, int iNumViewportX, int iNumViewportY);

void cd_switcher_compute_desktop_from_index (int iIndex, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY);


void cd_switcher_add_a_desktop (void);

void cd_switcher_remove_last_desktop (void);


void cd_switcher_trigger_update_from_screen_geometry (gboolean bNow);

void cd_switcher_trigger_update_from_wallpaper (void);

void cd_switcher_refresh_desktop_values (GldiModuleInstance *myApplet);


void cd_switcher_foreach_window_on_viewport (int iNumDesktop, int iNumViewportX, int iNumViewportY, CDSwitcherActionOnViewportFunc pFunction, gpointer pUserData);


#endif


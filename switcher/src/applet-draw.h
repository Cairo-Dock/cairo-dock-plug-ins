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


void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer);
gboolean switcher_draw_main_dock_icon (void);
//void cd_switcher_draw_windows_on_each_viewports();
void cd_switcher_draw_windows_on_each_viewports(double Xposition, double Yposition, double Xsize, double Ysize);
void cd_switcher_draw_main_icon_compact_mode (void);

void cd_switcher_draw_main_icon_expanded_mode (void);

void cd_switcher_draw_main_icon (void);


void cd_switcher_draw_desktops_bounding_box (CairoDesklet *pDesklet);
void cd_switcher_extract_viewport_coords_from_picked_object (CairoDesklet *pDesklet, int *iCoordX, int *iCoordY);

void cd_switcher_build_windows_list (GtkWidget *pMenu);

void cd_switcher_move_current_desktop_to (int iNumDesktop, int iNumViewportX, int iNumViewportY);


#endif


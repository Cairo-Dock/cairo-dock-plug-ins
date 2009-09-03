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


#ifndef __RENDERING_3D_PLANE_VIEW__
#define  __RENDERING_3D_PLANE_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY);


void cd_rendering_render_3D_plane (cairo_t *pCairoContext, CairoDock *pDock);


void cd_rendering_render_optimized_3D_plane (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock);


void cd_rendering_render_3D_plane_opengl (CairoDock *pDock);


void cd_rendering_register_3D_plane_renderer (const gchar *cRendererName);


void cd_rendering_draw_flat_separator_opengl (Icon *icon, CairoDock *pDock);

void cd_rendering_draw_physical_separator_opengl (Icon *icon, CairoDock *pDock, gboolean bBackGround, Icon *prev_icon, Icon *next_icon);


#endif

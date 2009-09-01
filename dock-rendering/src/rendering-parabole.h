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


#ifndef __RENDERING_PARABOLE__
#define  __RENDERING_PARABOLE__

#include "cairo-dock.h"


void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pParentDock);


void cd_rendering_calculate_reference_parabole (double alpha);


void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock);


void cd_rendering_calculate_construction_parameters_parabole (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iFlatDockWidth, gboolean bDirectionUp, double fAlign, gboolean bHorizontalDock);

void cd_rendering_render_icons_parabole (cairo_t *pCairoContext, CairoDock *pDock, double fRatio);

void cd_rendering_render_parabole (cairo_t *pCairoContext, CairoDock *pDock);

Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock);


void cd_rendering_register_parabole_renderer (const gchar *cRendererName);

void cd_rendering_render_parabole_opengl (CairoDock *pDock);


#endif

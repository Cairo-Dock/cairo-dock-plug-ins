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


#ifndef __RENDERING_ARC_EN_CIEL_VIEW__
#define  __RENDERING_ARC_EN_CIEL_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_rainbow (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_rainbow (Icon *icon, int iWidth, int iHeight, int iMaxDockWidth, double fReflectionOffsetY);

cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight);


void cd_rendering_render_rainbow (cairo_t *pCairoContext, CairoDock *pDock);


Icon *cd_rendering_calculate_icons_rainbow (CairoDock *pDock);


void cd_rendering_register_rainbow_renderer (const gchar *cRendererName);


void cd_rendering_reload_rainbow_buffers (void);


#endif

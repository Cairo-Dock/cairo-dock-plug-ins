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


#ifndef __RENDERING_COMMONS__
#define  __RENDERING_COMMONS__

#include "cairo-dock.h"

#define RENDERING_INTERPOLATION_NB_PTS 1000


cairo_surface_t *cd_rendering_create_flat_separator_surface (int iWidth, int iHeight);

void cd_rendering_load_flat_separator (CairoContainer *pContainer);


double cd_rendering_interpol (double x, double *fXValues, double *fYValues);


void cd_rendering_draw_physical_separator_opengl (Icon *icon, CairoDock *pDock, gboolean bBackGround, Icon *prev_icon, Icon *next_icon);

void cd_rendering_draw_flat_separator_opengl (Icon *icon, CairoDock *pDock);


#endif

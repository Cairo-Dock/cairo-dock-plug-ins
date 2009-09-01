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


#ifndef __RENDERING_DESKLET_SIMPLE__
#define  __RENDERING_DESKLET_SIMPLE__

#include "cairo.h"

#define MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME "Simple"


void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_simple_desklet_renderer (void);


void cd_rendering_register_desklet_decorations (void);

void rendering_draw_simple_in_desklet_opengl (CairoDesklet *pDesklet);


#endif

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


#ifndef __RENDERING_DESKLET_TREE__
#define  __RENDERING_DESKLET_TREE__

#include "cairo.h"

#define MY_APPLET_TREE_DESKLET_RENDERER_NAME "Tree"


typedef struct {
	gint iNbIconsInTree;
	gint iNbBranches;
	gdouble fTreeWidthFactor;
	gdouble fTreeHeightFactor;
	cairo_surface_t *pBrancheSurface[2];
	} CDTreeParameters;

CDTreeParameters *rendering_configure_tree (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_tree_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_tree_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_tree (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_tree_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_tree_desklet_renderer (void);


#endif

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


#ifndef __RENDERING_DESKLET_SLIDE__
#define  __RENDERING_DESKLET_SLIDE__

#include "cairo-dock.h"


typedef struct {
	// from config
	gboolean bRoundedRadius;
	gint iRadius;
	gdouble fLineColor[4];
	gint iLineWidth;
	gint iGapBetweenIcons;
	// computed data
	gdouble fMargin;
	gint iNbIcons;
	gint iIconSize;
	gint iNbLines, iNbColumns;
	} CDSlideParameters;

CDSlideParameters *rendering_configure_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_slide_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_slide_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_slide_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet);

void rendering_register_slide_desklet_renderer (void);


#endif

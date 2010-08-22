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

#ifndef __RENDERING_DESKLET_VIEWPORT__
#define  __RENDERING_DESKLET_VIEWPORT__

#include "cairo-dock.h"


typedef struct {
	// from config
	gboolean bHorizontalScrolBar;
	gint iIconGapX, iIconGapY;
	gdouble color_scrollbar_line[4];
	gdouble color_scrollbar_inside[4];
	gdouble color_grip[4];
	// computed data
	gint nRowsX;
	gint nRowsY;
	gint iDeltaHeight;  // hauteur scrollable, en pixels
	gint iScrollOffset;  // hauteur scrollee, en pixels, positive.
	gboolean bDraggingScrollbar;  // si le clic est couramment enfonce sur la scrollbar.
	guint iSidPressEvent;  // sid du clic
	guint iSidReleaseEvent;  // sid du relachement du clic
	gint iClickY;  // hauteur ou on a clique, en coordonnees fenetre
	gint iClickOffset;  // hauteur scrollee au moment du clic
	gdouble fMargin;
	
	gint iIconSize;
	gdouble fArrowHeight;
	gdouble fArrowGap;
	gdouble fScrollbarArrowGap;
	gdouble fScrollbarWidth;
	gdouble fScrollbarIconGap;
	} CDViewportParameters;


void rendering_register_viewport_desklet_renderer (void);

//gboolean cd_slide_on_click (gpointer data, Icon *pClickedIcon, CairoDesklet *pDesklet, guint iButtonState);


#endif

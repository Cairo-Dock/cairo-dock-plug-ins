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


#ifndef __RENDERING_DESKLET_CONTROLER__
#define  __RENDERING_DESKLET_CONTROLER__

#include "cairo-dock.h"

#define MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME "Controler"


typedef struct {
	gboolean b3D;
	gboolean bCircular;
	gdouble fGapBetweenIcons;
	gint iEllipseHeight;
	gdouble fInclinationOnHorizon;
	gint iFrameHeight;
	gdouble fExtraWidth;
	gint iControlPanelHeight;
	Icon *pClickedIcon;
	} CDControlerParameters;


CDControlerParameters *rendering_configure_controler (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_load_controler_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_free_controler_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_controler (CairoDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_controler_desklet_renderer (void);


#endif

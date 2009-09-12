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


#ifndef __RENDERING_CAROUSSEL__
#define  __RENDERING_CAROUSSEL__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_caroussel (CairoDock *pDock);


void cd_rendering_calculate_construction_parameters_caroussel (Icon *icon, int iWidth, int iHeight, int iMaxIconHeight, int iMaxIconWidth, int iEllipseHeight, gboolean bDirectionUp, double fExtraWidth, double fLinearWidth, double fXFirstIcon);

void cd_rendering_render_icons_caroussel (cairo_t *pCairoContext, CairoDock *pDock);

void cd_rendering_render_caroussel (cairo_t *pCairoContext, CairoDock *pDock);

Icon *cd_rendering_calculate_icons_caroussel (CairoDock *pDock);


void cd_rendering_register_caroussel_renderer (const gchar *cRendererName);


gboolean cd_rendering_caroussel_update_dock (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation);

#endif

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


#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_show_mouse_render (gpointer pUserData, GldiContainer *pContainer, cairo_t *pCairoContext);


gboolean cd_show_mouse_update_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bContinueAnimation);


gboolean cd_show_mouse_enter_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bStartAnimation);


gdouble *cd_show_mouse_init_sources (void);
CairoParticleSystem *cd_show_mouse_init_system (GldiContainer *pContainer, double dt, double *pSourceCoords);

void cd_show_mouse_update_sources (CDShowMouseData *pData);
void cd_show_mouse_update_particle_system (CairoParticleSystem *pParticleSystem, CDShowMouseData *pData);


gboolean cd_show_mouse_free_data (gpointer pUserData, GldiContainer *pContainer);


#endif

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
#define __APPLET_NOTIFICATIONS__

#include <cairo-dock.h>


gboolean cd_animations_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation);
gboolean cd_animations_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState);
gboolean cd_animations_on_request (gpointer pUserData, Icon *pIcon, CairoDock *pDock, const gchar *cAnimation, gint iNbRounds);


gboolean cd_animations_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext);
gboolean cd_animations_post_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext);


gboolean cd_animations_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation);

gboolean cd_animations_unfold_subdock (gpointer pUserData, Icon *pIcon);


gboolean cd_animations_free_data (gpointer pUserData, Icon *pIcon);


void cd_animations_register_animation (CDAnimation *pAnimation);

#endif

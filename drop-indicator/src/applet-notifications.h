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


gboolean cd_drop_indicator_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext);


gboolean cd_drop_indicator_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation);


gboolean cd_drop_indicator_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation);


void cd_drop_indicator_load_drop_indicator (gchar *cImagePath, cairo_t* pSourceContext, int iWidth, int iHeight);

void cd_drop_indicator_load_hover_indicator (gchar *cImagePath, cairo_t* pSourceContext, int iWidth, int iHeight);

gboolean cd_drop_indicator_stop_dock (gpointer data, CairoDock *pDock);


#endif

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

#ifndef __APPLET_IMPULSE__
#define  __APPLET_IMPULSE__

#include <cairo-dock.h>
#include "applet-struct.h"

void cd_impulse_im_setSourceIndex (gint iSourceIndex);
void cd_impulse_stop_animations (gboolean bChangeIcon);
void cd_impulse_launch_task (void); //(GldiModuleInstance *myApplet);
gboolean cd_impulse_on_icon_changed (gpointer pUserData, Icon *pIcon, CairoDock *pDock);
void cd_impulse_draw_current_state (void);
void cd_impulse_start_animating_with_delay (void);

#endif

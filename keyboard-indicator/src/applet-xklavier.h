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


#ifndef __APPLET_XKLAVIER__
#define  __APPLET_XKLAVIER__


#include <cairo-dock.h>


void cd_xkbd_set_prev_next_group (int iDelta);


void cd_xkbd_set_group (int iNumGroup);


gboolean cd_xkbd_keyboard_state_changed (GldiModuleInstance *myApplet, Window *pWindow);


void cd_xkbd_force_redraw ();

void cd_xkbd_init (Display *pDisplay);

void cd_xkbd_stop ();

#endif

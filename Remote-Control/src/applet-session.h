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

#ifndef __APPLET_SESSION__
#define  __APPLET_SESSION__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_open_session (void);

void cd_do_close_session (void);

void cd_do_exit_session (void);

#define cd_do_session_is_running(...) (myData.iSessionState == CD_SESSION_RUNNING)
#define cd_do_session_is_closing(...) (myData.iSessionState == CD_SESSION_CLOSING)
#define cd_do_session_is_off(...) (myData.iSessionState == CD_SESSION_NONE)


#endif

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

#ifndef __APPLET_COMPIZ__
#define  __APPLET_COMPIZ _

#include <cairo-dock.h>


void cd_compiz_start_system_wm (void);

void cd_compiz_start_compiz (void);

void cd_compiz_switch_manager(void);

void cd_compiz_start_favorite_decorator (void);
void cd_compiz_start_decorator (compizDecorator iDecorator);

void cd_compiz_kill_compmgr(void);


void cd_compiz_read_data (void);
gboolean cd_compiz_update_from_data (void);


#endif

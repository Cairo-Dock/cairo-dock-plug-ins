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


#ifndef __APPLET_STACKS__
#define  __APPLET_STACKS__


#include <cairo-dock.h>

void cd_stacks_check_local(void);
void cd_stacks_mklink(const gchar *cFile);
void cd_stacks_clean_local(void);
void cd_stacks_run_dir(void);
GList* cd_stacks_mime_filter(GList *pList);
void cd_stacks_remove_monitors (void);

#endif

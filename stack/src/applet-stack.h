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


#ifndef __APPLET_STACK__
#define  __APPLET_STACK__

#include <cairo-dock.h>

GList* cd_stack_mime_filter(GList *pList);

void cd_stack_check_local (GldiModuleInstance *myApplet, GKeyFile *pKeyFile);
void cd_stack_clear_stack (GldiModuleInstance *myApplet);

void cd_stack_remove_item (GldiModuleInstance *myApplet, Icon *pIcon);


void cd_stack_create_and_load_item (GldiModuleInstance *myApplet, const gchar *cContent);
void cd_stack_set_item_name (const gchar *cDesktopFilePath, const gchar *cName);
void cd_stack_set_item_order (const gchar *cDesktopFilePath, double fOrder);

#endif

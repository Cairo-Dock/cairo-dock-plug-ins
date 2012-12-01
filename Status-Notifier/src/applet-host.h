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

#ifndef __APPLET_HOST__
#define  __APPLET_HOST__

#include <cairo-dock.h>


CDStatusNotifierItem * cd_satus_notifier_find_item_from_service (const gchar *cService);

CDStatusNotifierItem * cd_satus_notifier_find_item_from_position (int iPosition);


void cd_satus_notifier_add_new_item_with_default (const gchar *cService, const gchar *cObjectPath, int iPosition, const gchar *cIconName, const gchar *cIconThemePath, const gchar *cLabel);

#define cd_satus_notifier_add_new_item(cService, cObjectPath, iPosition) cd_satus_notifier_add_new_item_with_default (cService, cObjectPath, iPosition, NULL, NULL, NULL)

void cd_satus_notifier_remove_item (const gchar *cService, int iPosition);


void cd_satus_notifier_launch_service (void);

void cd_satus_notifier_stop_service (void);

void cd_satus_notifier_launch_our_watcher (void);


void cd_satus_notifier_add_theme_path (const gchar * cThemePath);

void cd_satus_notifier_remove_theme_path (const gchar * cThemePath);


void cd_satus_notifier_load_icons_from_items (void);


#endif

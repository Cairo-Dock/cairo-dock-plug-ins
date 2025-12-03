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

#ifndef __APPLET_ITEM__
#define  __APPLET_ITEM__

#include <cairo-dock.h>

#define CD_STATUS_NOTIFIER_ITEM_OBJ "/StatusNotifierItem"

/** Create a new item asynchronously and add it to our list of items if successful.
@param pItem a newly allocated item with at least the cService member filled out by the caller
@param cObjectPath the object path to use

This function takes ownership of the pItem parameter (which will be added to our list of items or freed
in case of an error). The caller can set any member to a suggested default value; however, these will be
overwritten if they are provided by the application.
*/
void cd_satus_notifier_create_item (CDStatusNotifierItem *pItem, const gchar *cObjectPath);

void cd_free_item (CDStatusNotifierItem *pItem);

gchar *cd_satus_notifier_search_item_icon_s_path (CDStatusNotifierItem *pItem, gint iSize);

Icon *cd_satus_notifier_create_icon_for_item (CDStatusNotifierItem *pItem);


CDStatusNotifierItem *cd_satus_notifier_get_item_from_icon (Icon *pIcon);

Icon *cd_satus_notifier_get_icon_from_item (CDStatusNotifierItem *pItem);


// note: we can have items in the list which have not been fully created yet, we need to ignore these
// we check pProxy to decide this
#define _item_is_visible(item) ((item)->pProxy && ((item)->iStatus != CD_STATUS_PASSIVE || ! myConfig.bHideInactive))


void cd_satus_notifier_build_item_dbusmenu (CDStatusNotifierItem *pItem);


#endif

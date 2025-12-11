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

/** Helper function to get a string value from a variant with error checking.
* Empty strings are replaced by NULL.
*@param v2 the variant to process; will be unrefed by this call (i.e. "consumed")
*@param str destination to store the result in
*@return whether a string was properly extracted from v2
*
* Note: this function proceeds in three steps:
*   (1) First, it checks whether v2 contains a string. If not, it returns FALSE without touching str.
*   (2) Then, the current contents of str are freed and replaced by (a copy of) the contents of v2.
*   (3) If the result would be an empty string, *str is set to NULL instead.
* Notably, if str is a valid string, but v2 contains an empty string, str will be set to NULL; thi
* is to allow erasing properties.
*/
gboolean cd_status_notifier_get_string_from_variant (GVariant *v2, gchar **str);

/** Helper function to extract a child string value from a composite variant (tuple or array).
 * The logic for extracting the string is the same as in cd_status_notifier_get_string_from_variant ().
*@param v2 the variant to process; must be an array or a tuple (this is not checked)
*@param i_ the index of the value to extract; must be a valid index
*@param str destination to store the result in
*@return whether a string was properly extracted
*/
gboolean cd_status_notifier_get_string_child_from_variant (GVariant *v2, unsigned int i_, gchar **str);

CDStatusNotifierItem *cd_satus_notifier_get_item_from_icon (Icon *pIcon);

Icon *cd_satus_notifier_get_icon_from_item (CDStatusNotifierItem *pItem);


// note: we can have items in the list which have not been fully created yet, we need to ignore these
// we check pProxy to decide this
#define _item_is_visible(item) ((item)->pProxy && ((item)->iStatus != CD_STATUS_PASSIVE || ! myConfig.bHideInactive))


void cd_satus_notifier_build_item_dbusmenu (CDStatusNotifierItem *pItem);


#endif

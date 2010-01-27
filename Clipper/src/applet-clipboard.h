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


#ifndef __APPLET_CLIPBOARD__
#define  __APPLET_CLIPBOARD__


#include <cairo-dock.h>
#include "applet-struct.h"


void _on_text_received (GtkClipboard *pClipBoard, const gchar *text, gpointer user_data); // temporairement declaree ici.

void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data);


GList *cd_clipper_load_actions (const gchar *cConfFilePath);

void cd_clipper_free_item (CDClipperItem *pItem);

void cd_clipper_free_command (CDClipperCommand *pCommand);

void cd_clipper_free_action (CDClipperAction *pAction);


GtkWidget *cd_clipper_build_action_menu (CDClipperAction *pAction);

GtkWidget *cd_clipper_build_items_menu (void);

GtkWidget *cd_clipper_build_persistent_items_menu (void);

void cd_clipper_popup_menu (GtkWidget *pMenu);


gchar *cd_clipper_concat_items_of_type (CDClipperItemType iType);

#endif

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


#ifndef __APPLET_MENU_CALLBACKS__
#define  __APPLET_MENU_CALLBACKS__

#include <cairo-dock.h>
#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>


void handle_gmenu_tree_changed (GMenuTree *tree,
			   GtkWidget *menu);

void remove_gmenu_tree_monitor (GtkWidget *menu,
			  GMenuTree  *tree);

void remove_submenu_to_display_idle (gpointer data);

gboolean submenu_to_display_in_idle (gpointer data);

void submenu_to_display (GtkWidget *menu);

void panel_desktop_menu_item_append_menu (GtkWidget *menu,
				     gpointer   data);
void main_menu_append (GtkWidget *main_menu,
		  gpointer   data);

void icon_to_load_free (IconToLoad *icon);

void image_menu_shown (GtkWidget *image, gpointer data);

void activate_app_def (GtkWidget      *menuitem,
		  GMenuTreeEntry *entry);

void  drag_data_get_menu_cb (GtkWidget        *widget,
		       GdkDragContext   *context,
		       GtkSelectionData *selection_data,
		       guint             info,
		       guint             time,
		       GMenuTreeEntry   *entry);





#endif

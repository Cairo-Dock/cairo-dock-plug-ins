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

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-stack.h"


static inline void _launch_item (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	if (pIcon->iVolumeID == 1)
	{
		cairo_dock_fm_launch_uri (pIcon->cCommand);
	}
	else
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (pIcon->cCommand, pIcon, (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer), 2000, "same icon");
		
		cairo_dock_stop_icon_animation (pIcon);
	}
}
//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)
	{
		if (CD_APPLET_MY_ICONS_LIST == NULL)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("No items in the stack.\nYou can add files, URL, and even a piece of text by dragging them onto the icon."), myIcon, myContainer, 8000., "same icon");
		}
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;  // on laisse passer la notification (pour ouvrir le sous-dock au clic).
	}
	else if (CD_APPLET_CLICKED_ICON != NULL)
	{
		_launch_item (CD_APPLET_CLICKED_ICON, myApplet);  // on intercepte la notification.
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _on_text_received (GtkClipboard *clipboard, const gchar *text, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (text != NULL);
	CD_APPLET_ENTER;
	cd_stack_create_and_load_item (myApplet, text);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_clear_stack (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	int iAnswer = cairo_dock_ask_question_and_wait (D_("Clear the stack?"), myIcon,  myContainer);
	if (iAnswer == GTK_RESPONSE_YES)
		cd_stack_clear_stack (myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_remove_item (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	cd_stack_remove_item (myApplet, pIcon);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_rename_item (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	CairoContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
	gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Set new name for this item:"), pIcon, pContainer, pIcon->cName);
	if (cNewName == NULL)
		CD_APPLET_LEAVE ();
		//return ;
	
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", myConfig.cStackDir, pIcon->cDesktopFileName);
	cd_stack_set_item_name (cDesktopFilePath, cNewName);
	g_free (cDesktopFilePath);
	
	cairo_dock_set_icon_name (cNewName, pIcon, pContainer);
	g_free (cNewName);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_copy_content (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	GtkClipboard *pClipBoard = (myConfig.bSelectionClipBoard ? gtk_clipboard_get (GDK_SELECTION_PRIMARY) : gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
	cd_debug ("stack : '%s' has been copied into the clipboard\n", pIcon->cCommand);
	gtk_clipboard_set_text (pClipBoard, pIcon->cCommand, -1);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_paste_content (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	GtkClipboard *pClipBoard = (myConfig.bSelectionClipBoard ? gtk_clipboard_get (GDK_SELECTION_PRIMARY) : gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
	gtk_clipboard_request_text (pClipBoard, (GtkClipboardTextReceivedFunc) _on_text_received, myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_cut_item (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	GtkClipboard *pClipBoard = (myConfig.bSelectionClipBoard ? gtk_clipboard_get (GDK_SELECTION_PRIMARY) : gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
	gtk_clipboard_set_text (pClipBoard, pIcon->cCommand, -1);
	cd_stack_remove_item (myApplet, pIcon);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_open_item (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	_launch_item (pIcon, myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_stack_open_item_folder (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	
	gchar *cFolderPath = g_path_get_dirname (pIcon->cCommand);
	cairo_dock_fm_launch_uri (cFolderPath);
	g_free (cFolderPath);
	CD_APPLET_LEAVE ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	static gpointer data[2] = {NULL, NULL};
	data[0] = myApplet;
	data[1] = pClickedIcon;
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Copy (middle click)"), GTK_STOCK_COPY, _cd_stack_copy_content, pSubMenu, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Cut"), GTK_STOCK_CUT, _cd_stack_cut_item, pSubMenu, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this item"), GTK_STOCK_SAVE_AS, _cd_stack_rename_item, pSubMenu, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Remove this item"), GTK_STOCK_REMOVE, _cd_stack_remove_item, pSubMenu, data);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (pSubMenu);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open (click)"), GTK_STOCK_EXECUTE, _cd_stack_open_item, pSubMenu, data);
		if (pClickedIcon->iVolumeID == 1)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open folder"), GTK_STOCK_OPEN, _cd_stack_open_item_folder, pSubMenu, data);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (pSubMenu);
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Paste (drag'n'drop)"), GTK_STOCK_PASTE, _cd_stack_paste_content, pSubMenu);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear the stack"), GTK_STOCK_CLEAR, _cd_stack_clear_stack, pSubMenu);
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	//cd_message ("File to link in local: %s", CD_APPLET_RECEIVED_DATA);
	cd_stack_create_and_load_item (myApplet, CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		gpointer data[2] = {myApplet, pClickedIcon};
		_cd_stack_copy_content (NULL, data);
	}
CD_APPLET_ON_MIDDLE_CLICK_END

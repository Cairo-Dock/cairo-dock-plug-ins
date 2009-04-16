/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Copyright (C) 2007 GESTES Cedric
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "stdlib.h"

#include "terminal-config.h"
#include "terminal-menu-functions.h"
#include "terminal-struct.h"
#include "terminal-widget.h"
#include "terminal-init.h"


CD_APPLET_DEFINITION ("terminal",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("Add a terminal to your dock !\n"
	"Left/middle click to show/hide terminal (you can bind a keyboard shortcut for it.)\n"
	"You can open many tabs, rename them, change their color.\n"
	"You can drag'n'drop files or text and select an action.\n"
	"Shortcuts are available : 'CTRL+t' and 'CTRL+w' to open/close current tab,\n"
	"Right-click on a tab to interact on it,\n"
	"Middle-click on a tab to close it,\n"
	"Double-click next to the tabs to open a new one."),
	"Ctaf (Cedric Gestes) & Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
{
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;

	if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
	{
		terminal_build_and_show_tab ();
		CD_APPLET_SET_STATIC_DESKLET;
	}
	if (myDock)  // en mode desklet, on n'a pas besoin de l'icone.
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	}
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
  CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
}
CD_APPLET_STOP_END



CD_APPLET_RELOAD_BEGIN
{
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (! myData.tab)
		{
			if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
				terminal_build_and_show_tab ();
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				myData.tab = cairo_dock_steal_widget_from_its_container (myData.tab);
				cairo_dock_dialog_unreference (myData.dialog);
				myData.dialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.tab, myDesklet);
				//myDesklet->renderer = term_draw_in_desklet;
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
				CD_APPLET_SET_STATIC_DESKLET;
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.dialog = cd_terminal_build_dialog ();
				//myData.dialog = cairo_dock_build_dialog (D_("Terminal"), myIcon, myContainer, NULL, myData.tab, GTK_BUTTONS_NONE, NULL, NULL, NULL);
				cairo_dock_hide_dialog (myData.dialog);
			}
		}
		
		if (myData.tab)
		{
			term_apply_settings();
		}
		
		if (myDock)  // en mode desklet, on n'a pas besoin de l'icone.
		{
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		}
	}
}
CD_APPLET_RELOAD_END

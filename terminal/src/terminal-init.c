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
#include "terminal-init.h"


CD_APPLET_DEFINITION ("terminal", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN (erreur)
{
  CD_APPLET_REGISTER_FOR_CLICK_EVENT;
  CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
  CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;

	if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
		terminal_build_and_show_tab ();
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
  CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

  //\_________________ On libere toutes nos ressources.
  reset_config ();
  reset_data ();
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
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.dialog = cairo_dock_build_dialog (_D("Terminal"), myIcon, myContainer, NULL, myData.tab, GTK_BUTTONS_NONE, NULL, NULL, NULL);
				cairo_dock_hide_dialog (myData.dialog);
			}
		}
		
		if (myData.tab)
		{
			term_apply_settings();
		}
	}
}
CD_APPLET_RELOAD_END

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Author(s):
**  - Cedric GESTES <ctaf42@gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <vte/vte.h>

#include "terminal-init.h"
#include "terminal-callbacks.h"
#include "terminal-struct.h"
#include "terminal-widget.h"
#include "terminal-menu-functions.h"


CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very convenient terminal applet made by Cedric GESTES for Cairo-Dock");


CD_APPLET_ON_CLICK_BEGIN
{
	if (! myData.tab)
		terminal_build_and_show_tab ();
	else if (myDesklet)
		cairo_dock_show_desklet (myDesklet);
	else if (myData.dialog)
		cairo_dock_unhide_dialog(myData.dialog);
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	if (myData.tab)
	{
          if (myData.dialog)
            cairo_dock_hide_dialog (myData.dialog);
	}
}
CD_APPLET_ON_MIDDLE_CLICK_END



static void on_new_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_new_tab();
}
static void on_rename_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_rename_current_tab ();
}
static void on_close_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_close_current_tab ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
{
	CD_APPLET_ADD_SUB_MENU("Terminal", pSubMenu, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU(D_("New Tab"), on_new_tab, pSubMenu);
	CD_APPLET_ADD_IN_MENU(D_("Rename Tab"), on_rename_tab, pSubMenu);
	CD_APPLET_ADD_IN_MENU(D_("Close the current Tab"), on_close_tab, pSubMenu);
	CD_APPLET_ADD_SEPARATOR();
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END

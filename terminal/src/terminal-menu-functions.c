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


CD_APPLET_ON_CLICK_BEGIN
{
	// on cree ou montre le dialogue/desklet
	if (! myData.tab)
		terminal_build_and_show_tab ();
	else if (myData.dialog)
		gldi_dialog_toggle_visibility (myData.dialog);
	// on donne le focus au terminal de l'onglet courant.
	cd_terminal_grab_focus ();
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	if (myConfig.cTerminal != NULL)
		cairo_dock_launch_command (myConfig.cTerminal);
	else
		cairo_dock_launch_command (cairo_dock_get_default_terminal ());
}
CD_APPLET_ON_MIDDLE_CLICK_END


static void on_new_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_new_tab();
}
static void on_rename_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_rename_tab (NULL);
}
static void on_close_tab(GtkMenuItem *menu_item, gpointer *data)
{
	terminal_close_tab (NULL);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
{
	// Menu on the icon (when clicking on a tab or in the terminal, another menu is raised)
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("New Tab"), GTK_STOCK_NEW, on_new_tab, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Rename current Tab"), GTK_STOCK_EDIT, on_rename_tab, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Close current Tab"), GTK_STOCK_CLOSE, on_close_tab, CD_APPLET_MY_MENU);
}
CD_APPLET_ON_BUILD_MENU_END

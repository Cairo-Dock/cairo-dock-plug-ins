/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "systray-menu-functions.h"
#include "systray-init.h"
#include "systray-struct.h"
#include "na-tray.h"


CairoDialog *cd_systray_build_dialog (void)
{
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	attr.pInteractiveWidget = GTK_WIDGET (myData.tray);
	return cairo_dock_build_dialog (&attr, myIcon, myContainer);
}

void systray_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	if (myData.tray)
	{
		if (myDesklet)
			cairo_dock_show_desklet(myDesklet);
		else if (myData.dialog)
			cairo_dock_unhide_dialog(myData.dialog);
	}
	else
	{
		systray_build_and_show ();
	}
}

void systray_apply_settings()
{
  cd_keybinder_bind(myConfig.shortcut, (CDBindkeyHandler)systray_on_keybinding_pull, 0);
}

void systray_build_and_show (void)
{
	myData.tray = na_tray_new_for_screen (gtk_widget_get_screen (GTK_WIDGET (myContainer->pWidget)),
		myConfig.iIconPacking == 0 ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
	gtk_widget_show (GTK_WIDGET (myData.tray));
	myData.iIconPacking = myConfig.iIconPacking;

	systray_apply_settings();

	if (myDock)
	{
		myData.dialog = cd_systray_build_dialog ();
		gtk_window_set_resizable(GTK_WINDOW(myData.dialog->container.pWidget), FALSE);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (GTK_WIDGET (myData.tray), myDesklet);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		gtk_window_set_resizable(GTK_WINDOW(myDesklet->container.pWidget), FALSE);
	}
}

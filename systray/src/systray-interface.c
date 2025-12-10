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

#include "systray-interface.h"
#include "systray-init.h"
#include "systray-struct.h"
#include "na-tray.h"
#include "na-tray-manager.h"

static const char *_get_warning_message (void)
{
	return D_("Another systray is already running (probably on your panel)\nSince there can only be one systray at once, you should remove it to avoid any conflict.");
}

void cd_systray_build_dialog (void)
{
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	if (myData.tray) attr.pInteractiveWidget = GTK_WIDGET (myData.tray); // myData.tray will be NULL if we could not start it
	else attr.cText = _get_warning_message (); // show a warning about not having an active tray
	attr.bHideOnClick = TRUE;  // keep the dialog alive on click (hide it).
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	myData.dialog = gldi_dialog_new (&attr);
	gldi_dialog_hide (myData.dialog);
}



static void _show_error_dialog (void)
{
	gldi_dialog_show_temporary_with_icon (_get_warning_message (), myIcon, myContainer, 8000, NULL);
}

static gboolean _show_error_dialog_delayed (G_GNUC_UNUSED void *ptr)
{
	CD_APPLET_ENTER;
	if (cairo_dock_is_loading ())
	{
		CD_APPLET_LEAVE (G_SOURCE_CONTINUE);
	}
	
	_show_error_dialog ();
	
	CD_APPLET_LEAVE (G_SOURCE_REMOVE);
}

static gboolean s_bChecked = FALSE;

void cd_systray_build_desklet (void)
{
	if (myData.tray) gldi_desklet_add_interactive_widget (myDesklet, GTK_WIDGET (myData.tray));
	else
	{
		// no tray, just add a placeholder -- maybe we could display a broken icon instead?
		GtkWidget *dummy = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		g_object_set (dummy, "height-request", 24, NULL);
		gldi_desklet_add_interactive_widget_with_margin (myDesklet, dummy, 24);
	}
	CD_APPLET_SET_DESKLET_RENDERER (NULL);
	CD_APPLET_SET_STATIC_DESKLET;
}

void cd_systray_build_systray (void)
{
	if (myData.tray != NULL)
		return;
	
	if (na_tray_manager_check_running (gtk_widget_get_screen (GTK_WIDGET (myContainer->pWidget))))
	{
		cd_warning ("another systray is already running, not creating ours");
		if (!s_bChecked)
		{
			s_bChecked = TRUE;
			// show an error dialog (do this only once)
			if (cairo_dock_is_loading ())
				g_timeout_add_seconds (2, _show_error_dialog_delayed, NULL);
			else _show_error_dialog ();
		}
	}
	else
	{
		myData.tray = na_tray_new_for_screen (gtk_widget_get_screen (GTK_WIDGET (myContainer->pWidget)),
			myConfig.iIconPacking == 0 ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
		na_tray_set_icon_size (myData.tray, 24);
		na_tray_set_padding (myData.tray, 3);
	}
	
	if (myDock)
	{
		cd_systray_build_dialog ();
	}
	else
	{
		cd_systray_build_desklet ();
	}
	gtk_widget_show (GTK_WIDGET (myData.tray));
}



void systray_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	if (myData.tray)
	{
		if (myDesklet)
			gldi_desklet_show(myDesklet);
		else if (myData.dialog)
			gldi_dialog_unhide (myData.dialog);
	}
}


void cd_systray_set_orientation (GtkOrientation o)
{
	GtkOrientation o_ = na_tray_get_orientation (myData.tray);
	if (o != o_)
	{
		na_tray_set_orientation (myData.tray, o);
	}
}

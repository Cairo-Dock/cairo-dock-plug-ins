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

#include "stdlib.h"

#include "systray-config.h"
#include "systray-menu-functions.h"
#include "systray-init.h"
#include "cd-tray.h"
#include "systray-struct.h"


CD_APPLET_DEFINITION ("systray", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN
{
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
		systray_build_and_show ();
	if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE)
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
		if (! myData.tray)
		{
			if (myDesklet != NULL)  // on cree le systray pour avoir qqch a afficher dans le desklet.
				systray_build_and_show ();
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				myData.tray->widget = cairo_dock_steal_widget_from_its_container (myData.tray->widget);
				cairo_dock_dialog_unreference (myData.dialog);
				myData.dialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.tray->widget, myDesklet);
				//myDesklet->renderer = systray_draw_in_desklet;
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.dialog = cairo_dock_build_dialog (NULL, myIcon, myContainer, NULL, myData.tray->widget, GTK_BUTTONS_NONE, NULL, NULL, NULL);
				cairo_dock_hide_dialog (myData.dialog);
			}
		}

		if (myData.tray)
		{
			systray_apply_settings();
		}
	}
}
CD_APPLET_RELOAD_END

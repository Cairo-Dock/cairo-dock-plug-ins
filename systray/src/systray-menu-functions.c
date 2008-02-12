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
#include "cd-tray.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_ABOUT("This is a simple systray applet made by Cedric GESTES for Cairo-Dock");

extern AppletConfig myConfig;
extern AppletData myData;

static void systray_build_new_dialog();
void systray_on_keybinding_pull (const char *keystring, gpointer user_data);

void systray_on_keybinding_pull (const char *keystring, gpointer user_data)
{
  if (myData.desklet)
    cairo_dock_show_desklet((CairoDockDesklet *)user_data);
  else if (myData.dialog)
    cairo_dock_unhide_dialog(myData.dialog);
  else
    systray_build_new_dialog();
}

void systray_apply_settings()
{
  cd_keybinder_bind(myConfig.shortcut, (CDBindkeyHandler)systray_on_keybinding_pull, 0);
}

static void systray_build_new_dialog()
{
  GtkRequisition req;

  myData.tray = tray_init(myDock->pWidget);
  myData.desklet = cairo_dock_create_desklet (myIcon, myData.tray->widget);
  //we always want the minimal size
  gtk_window_set_resizable(GTK_WINDOW(myData.desklet->pWidget), FALSE);
  gtk_widget_size_request(GTK_WIDGET(myData.tray->box), &req);
  gtk_window_resize(GTK_WINDOW(myData.desklet->pWidget), 24, 24);
  systray_apply_settings();
}

CD_APPLET_ON_CLICK_BEGIN
{
  if (myData.desklet)
    cairo_dock_show_desklet(myData.desklet);
  else if (myData.dialog)
    cairo_dock_unhide_dialog(myData.dialog);
  else
    systray_build_new_dialog();
}
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
  if (myData.desklet)
    cairo_dock_hide_desklet(myData.desklet);
  else if (myData.dialog)
    cairo_dock_hide_dialog (myData.dialog);
}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
{
  CD_APPLET_ADD_SUB_MENU("Systray", pSubMenu, pAppletMenu);
  CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END



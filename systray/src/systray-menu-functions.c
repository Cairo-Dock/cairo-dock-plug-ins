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
#include "cd-tray.h"
#include "cairo-dock-desklet.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_ABOUT("This is a simple systray applet made by Cedric GESTES for Cairo-Dock");

extern t_systray systray;

static CairoDockDesklet *systray_new_dialog();
static void onKeybindingPull (const char *keystring, gpointer user_data);

static void onKeybindingPull (const char *keystring, gpointer user_data)
{
  //  printf("{{##OnKeybindingPull\n");
  if (user_data) {
    cairo_dock_show_desklet((CairoDockDesklet *)user_data);
    return;
  }
  if (!systray.dialog) {
    systray.dialog = systray_new_dialog();
    cairo_dock_show_desklet(systray.dialog);
    //rebind with the dialog
    cd_keybinder_unbind(systray.prev_shortcut, (CDBindkeyHandler)onKeybindingPull);
    systray.prev_shortcut = systray.shortcut;
    cd_keybinder_bind(systray.shortcut, (CDBindkeyHandler)onKeybindingPull, (gpointer)systray.dialog);
  }
}

void systray_dialog_apply_settings()
{
/*   if (systray.dialog) */
/*     gtk_window_set_keep_above(GTK_WINDOW(systray.dialog->pWidget), systray.always_on_top); */
  cd_keybinder_unbind(systray.prev_shortcut, (CDBindkeyHandler)onKeybindingPull);
  systray.prev_shortcut = systray.shortcut;
  cd_keybinder_bind(systray.shortcut, (CDBindkeyHandler)onKeybindingPull, (gpointer)systray.dialog);
}

static CairoDockDesklet *systray_new_dialog()
{
  GtkRequisition req;

  systray.tray = tray_init(myDock->pWidget);
  systray.dialog = cairo_dock_create_desklet (0, systray.tray->widget, 0, 0);
  gtk_widget_size_request(GTK_WIDGET(systray.tray->box), &req);
  gtk_window_resize(GTK_WINDOW(systray.dialog->pWidget), 24, 24);
  systray_dialog_apply_settings();
  return systray.dialog;
}

CD_APPLET_ON_CLICK_BEGIN
{
  if (!systray.dialog)
    systray.dialog = systray_new_dialog();
  cairo_dock_show_desklet(systray.dialog);
}
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
  cairo_dock_hide_desklet(systray.dialog);
}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
{
  CD_APPLET_ADD_SUB_MENU("Systray", pSubMenu, pAppletMenu);
  CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END



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

#include <vte/vte.h>

#include "terminal-init.h"
#include "terminal-menu-functions.h"

extern t_terminal term;

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very simple terminal applet made by Cedric GESTES for Cairo-Dock")

static gboolean on_terminal_button_press_dialog (GtkWidget* pWidget,
                                                 GdkEventButton* pButton,
                                                 Icon *pIcon)
{
  cairo_dock_hide_dialog (term.dialog);
  return 0;

}

void term_dialog_apply_settings(GtkWidget *vterm)
{
  if (vterm) {
    vte_terminal_set_opacity(VTE_TERMINAL(vterm), term.transparency);
    vte_terminal_set_colors(VTE_TERMINAL(vterm), &term.forecolor, &term.backcolor, NULL, 0);
  }
  if (term.dialog)
    gtk_window_set_keep_above(GTK_WINDOW(term.dialog->pWidget), term.always_on_top);
}

static CairoDockDialog *terminal_new_dialog()
{
  CairoDockDialog *dialog;
  GtkWidget *vterm = NULL;

  vterm = vte_terminal_new();
  term.vterm = vterm;
  //transparency enable, otherwise we cant change value after
  vte_terminal_set_opacity(VTE_TERMINAL(vterm), term.transparency);
  vte_terminal_set_emulation(VTE_TERMINAL(vterm), "xterm");
  vte_terminal_fork_command(VTE_TERMINAL(vterm),
                            NULL,
                            NULL,
                            NULL,
                            "~/",
                            FALSE,
                            FALSE,
                            FALSE);
  dialog = cairo_dock_build_dialog("terminal", myIcon, myDock, "", vterm, GTK_BUTTONS_NONE, NULL, NULL);
  g_signal_connect (G_OBJECT (dialog->pWidget),
                    "button-press-event",
                    G_CALLBACK (on_terminal_button_press_dialog),
                    myIcon);
  cairo_dock_dialog_reference(dialog);
  term.dialog = dialog;
  term_dialog_apply_settings(vterm);
  return dialog;
}

CD_APPLET_ON_CLICK_BEGIN
  if (!term.dialog) {
    term.dialog = terminal_new_dialog();
    }
  else {
    cairo_dock_unhide_dialog(term.dialog);
    term_dialog_apply_settings(term.vterm);
  }

CD_APPLET_ON_CLICK_END

static void on_reload(GtkMenuItem *menu_item, gpointer *data)
{
  if (term.dialog) {
    cairo_dock_isolate_dialog(term.dialog);
    cairo_dock_dialog_unreference(term.dialog);
    cairo_dock_dialog_unreference(term.dialog);
    term.dialog = NULL;
    term.vterm = NULL;
  }
  term.dialog = terminal_new_dialog();
}


CD_APPLET_ON_BUILD_MENU_BEGIN
  CD_APPLET_ADD_SUB_MENU("Terminal", pSubMenu, pAppletMenu)
  CD_APPLET_ADD_IN_MENU("Reload", on_reload, pSubMenu)
  CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END



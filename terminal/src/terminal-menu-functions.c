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

#include "cairo-applet.h"
#include "terminal-init.h"
#include "terminal-menu-functions.h"

extern t_terminal term;

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very simple terminal applet made by Cedric GESTES for Cairo-Dock")

static void term_dialog_apply_settings(GtkWidget *vterm)
{
  if (vterm) {
    vte_terminal_set_opacity(VTE_TERMINAL(vterm), term.transparency);
    vte_terminal_set_colors(VTE_TERMINAL(vterm), &term.forecolor, &term.backcolor, NULL, 0);
  }
  if (term.dialog)
    gtk_window_set_keep_above(GTK_WINDOW(term.dialog->pWidget), term.always_on_top);
}


void term_tab_apply_settings()
{
  int sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab));
  GtkWidget *vterm = NULL;

  for (int i = 0; i < sz; ++i) {
    vterm = gtk_notebook_get_nth_page(GTK_NOTEBOOK(term.tab), i);
    term_dialog_apply_settings(vterm);
  }
}

static void on_terminal_child_exited(VteTerminal *vteterminal,
                                     gpointer t)
{
  gint p = gtk_notebook_page_num(GTK_NOTEBOOK(term.tab), GTK_WIDGET(vteterminal));
  gint sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab));

  if (sz > 1)
    gtk_notebook_remove_page(GTK_NOTEBOOK(term.tab), p);
  else {
    //TODO: echo somethink on the term, to say it's okay
    applet_hide_dialog(term.dialog);
    vte_terminal_fork_command(VTE_TERMINAL(vteterminal),
                              NULL,
                              NULL,
                              NULL,
                              "~/",
                              FALSE,
                              FALSE,
                              FALSE);
  }
}


/* static gboolean applet_on_key_press_cb(GtkWidget *window, GdkEventKey *event, t_terminal *terminal) */
/* { */

/*   if (!(terminal && terminal->vterm)) */
/*     return FALSE; */
/*   // Checks if the modifiers control and shift are pressed */
/*   if (event->state & GDK_CONTROL_MASK && event->state & GDK_SHIFT_MASK) */
/*     { */
/*       gchar *key = gdk_keyval_name (gdk_keyval_to_lower (event->keyval)); */

/*       // Copy */
/*       if (! strncmp (key, "c", 1)) vte_terminal_copy_clipboard (VTE_TERMINAL (terminal->vterm)); */
/*       // Paste */
/*       if (! strncmp (key, "v", 1)) vte_terminal_paste_clipboard (VTE_TERMINAL (terminal->vterm)); */
/*       // Signify that event has been handled */
/*       return TRUE; */
/*     } */
/*   else */
/*     { */
/*       // Signify that event has not been handled */
/*       return FALSE; */
/*     } */
/* } */


static void terminal_new_tab()
{
  GtkWidget *vterm = NULL;

  vterm = vte_terminal_new();
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
  g_signal_connect (G_OBJECT (vterm), "child-exited",
                    G_CALLBACK (on_terminal_child_exited), 0);

  gtk_notebook_append_page(GTK_NOTEBOOK(term.tab), vterm, NULL);
  gtk_widget_show(vterm);
}

static CairoDockDialog *terminal_new_dialog()
{
  CairoDockDialog *dialog;
  GtkWidget *vterm = NULL;

  term.tab = gtk_notebook_new();
  terminal_new_tab();
  gtk_widget_show(term.tab);

  dialog = applet_build_dialog (myDock, term.tab, NULL);

  term.dialog = dialog;
  term_tab_apply_settings();
  return dialog;
}


CD_APPLET_ON_CLICK_BEGIN
  if (!term.dialog) {
    term.dialog = terminal_new_dialog();
  }
  else {
    applet_unhide_dialog(term.dialog);
    term_tab_apply_settings();
  }
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  applet_hide_dialog(term.dialog);
CD_APPLET_ON_MIDDLE_CLICK_END


static void on_new_tab(GtkMenuItem *menu_item, gpointer *data)
{
  terminal_new_tab();
  term_tab_apply_settings();
}

static void on_close_tab(GtkMenuItem *menu_item, gpointer *data)
{
  gint p = gtk_notebook_get_current_page(GTK_NOTEBOOK(term.tab));
  gtk_notebook_remove_page(GTK_NOTEBOOK(term.tab), p);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU("Terminal", pSubMenu, CD_APPLET_MY_MENU)
        CD_APPLET_ADD_IN_MENU("New Tab", on_new_tab, pSubMenu)
        CD_APPLET_ADD_IN_MENU("Close the current Tab", on_close_tab, pSubMenu)
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END



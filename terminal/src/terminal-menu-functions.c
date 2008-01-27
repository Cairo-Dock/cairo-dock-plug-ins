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
#include "terminal-callbacks.h"
#include "terminal-menu-functions.h"

extern t_terminal term;

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very simple terminal applet made by Cedric GESTES for Cairo-Dock")



  static void terminal_new_tab();

static void on_new_tab(GtkMenuItem *menu_item, gpointer *data)
{
  terminal_new_tab();
  term_tab_apply_settings();
}

static void on_close_tab(GtkMenuItem *menu_item, gpointer *data)
{
  gint p = gtk_notebook_get_current_page(GTK_NOTEBOOK(term.tab));
  if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab)) > 1)
    gtk_notebook_remove_page(GTK_NOTEBOOK(term.tab), p);
}

static void term_dialog_apply_settings(GtkWidget *vterm)
{
  if (vterm) {
    vte_terminal_set_opacity(VTE_TERMINAL(vterm), term.transparency);
    vte_terminal_set_colors(VTE_TERMINAL(vterm), &term.forecolor, &term.backcolor, NULL, 0);
    vte_terminal_set_size(VTE_TERMINAL(vterm), term.iNbColumns, term.iNbRows);
  }
  if (term.dialog)
    gtk_window_set_keep_above(GTK_WINDOW(term.dialog->pWidget), term.always_on_top);
}


void term_tab_apply_settings()
{
  int sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab));
  GtkWidget *vterm = NULL;

  for (int i = 0; i < sz; ++i) {
    printf("settings...\n");
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



static GtkWidget *_terminal_build_menu_tab (GtkWidget *pWidget, gchar *cReceivedData)
{
	static gpointer *my_data = NULL;
	if (my_data == NULL)
		my_data = g_new0 (gpointer, 2);
	my_data[0] = pWidget;
	my_data[1] = cReceivedData;
	GtkWidget *menu = gtk_menu_new ();

	GtkWidget *menu_item, *image;
	menu_item = gtk_image_menu_item_new_with_label (_("Copy"));
	image = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
/* 	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_copy), my_data); */

	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_label ("New Tab");
	image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_new_tab), 0);

	menu_item = gtk_image_menu_item_new_with_label ("Close Tab");
	image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_close_tab), 0);

	return menu;
}

gboolean applet_on_terminal_press_cb(GtkWidget *window, GdkEventButton *event, t_terminal *terminal)
{
  printf("HEREEE\n");
  static gchar *cReceivedData = NULL;  // on en peut recevoir qu'un drop a la fois, donc pas de collision possible.
/*   if (!(terminal && terminal->vterm)) */
/*     return FALSE; */
  //TODO: window should be replaced with the good vterm
  if (event->button == 1)
    return FALSE;
  GtkWidget *menu = _terminal_build_menu_tab (window, cReceivedData);

  gtk_widget_show_all (menu);

  gtk_menu_popup (GTK_MENU (menu),
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  1,
                  gtk_get_current_event_time ());
  return TRUE;
}


/* static gboolean            on_terminal_press_cb                      (GtkWidget      *widget, */
/*                                                         GdkEventButton *event, */
/*                                                         gpointer        user_data) */
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
  vte_terminal_set_size (VTE_TERMINAL (vterm), term.iNbColumns, term.iNbRows);
  vte_terminal_fork_command(VTE_TERMINAL(vterm),
                            NULL,
                            NULL,
                            NULL,
                            "~/",
                            FALSE,
                            FALSE,
                            FALSE);
  g_signal_connect (G_OBJECT (vterm), "child-exited",
                    G_CALLBACK (on_terminal_child_exited), NULL);
  g_signal_connect (G_OBJECT (vterm), "button-release-event",
                    G_CALLBACK (applet_on_terminal_press_cb), &term);

  cairo_dock_allow_widget_to_receive_data (vterm, G_CALLBACK (on_terminal_drag_data_received));
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

  g_signal_connect (G_OBJECT (term.tab), "button-release-event",
                    G_CALLBACK (applet_on_terminal_press_cb), &term);
  dialog = applet_build_dialog (myDock, term.tab, NULL);
  //gtk_widget_set_size_request(dialog->pWidget, 600, 400);

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



CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU("Terminal", pSubMenu, CD_APPLET_MY_MENU)
        CD_APPLET_ADD_IN_MENU("New Tab", on_new_tab, pSubMenu)
        CD_APPLET_ADD_IN_MENU("Close the current Tab", on_close_tab, pSubMenu)
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END



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

#include "cairo-dock-desklet.h"
#include "terminal-init.h"
#include "terminal-callbacks.h"
#include "terminal-menu-functions.h"

extern t_terminal term;

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very simple terminal applet made by Cedric GESTES for Cairo-Dock");

static void terminal_new_tab();
static CairoDockDesklet *terminal_new_dialog();
static void onKeybindingPull (const char *keystring, gpointer user_data);


static void onKeybindingPull (const char *keystring, gpointer user_data)
{
  //  printf("{{##OnKeybindingPull\n");
  if (user_data) {
    cd_desklet_show((CairoDockDesklet *)user_data);
    return;
  }
  if (!term.dialog) {
    term.dialog = terminal_new_dialog();
    //rebind with the dialog
    cd_keybinder_unbind(term.prev_shortcut, (CDBindkeyHandler)onKeybindingPull);
    term.prev_shortcut = term.shortcut;
    cd_keybinder_bind(term.shortcut, (CDBindkeyHandler)onKeybindingPull, (gpointer)term.dialog);
  }
}

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
    vte_terminal_set_colors(VTE_TERMINAL(vterm), &term.forecolor, &term.backcolor, NULL, 0);
/*     vte_terminal_set_background_saturation(VTE_TERMINAL(vterm), 1.0); */
/*     vte_terminal_set_background_transparent(VTE_TERMINAL(vterm), FALSE); */
    vte_terminal_set_opacity(VTE_TERMINAL(vterm), term.transparency);
    vte_terminal_set_size(VTE_TERMINAL(vterm), term.iNbColumns, term.iNbRows);
    //    gtk_widget_queue_draw(term.dialog->pWidget);
  }
}


void term_tab_apply_settings()
{
  int sz = 0;
  GtkWidget *vterm = NULL;

  if (term.dialog) {
    sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab));
    for (int i = 0; i < sz; ++i) {
      vterm = gtk_notebook_get_nth_page(GTK_NOTEBOOK(term.tab), i);
      term_dialog_apply_settings(vterm);
    }
    gtk_window_set_keep_above(GTK_WINDOW(term.dialog->pWidget), term.always_on_top);
  }
  cd_keybinder_unbind(term.prev_shortcut, (CDBindkeyHandler)onKeybindingPull);
  term.prev_shortcut = term.shortcut;
  cd_keybinder_bind(term.shortcut, (CDBindkeyHandler)onKeybindingPull, (gpointer)term.dialog);
}

static void on_terminal_child_exited(VteTerminal *vteterminal,
                                     gpointer t)
{
  gint p = gtk_notebook_page_num(GTK_NOTEBOOK(term.tab), GTK_WIDGET(vteterminal));
  gint sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(term.tab));

  if (sz > 1)
    gtk_notebook_remove_page(GTK_NOTEBOOK(term.tab), p);
  else {
    // \r needed to return to the beginning of the line
    vte_terminal_feed(VTE_TERMINAL(vteterminal), "Shell exited. Another one is launching...\r\n\n", -1);
    cd_desklet_hide(term.dialog);
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


static void _terminal_copy (GtkMenuItem *menu_item, GtkWidget *data)
{
  vte_terminal_copy_clipboard(VTE_TERMINAL(data));
}
static void _terminal_paste (GtkMenuItem *menu_item, GtkWidget *data)
{
  vte_terminal_paste_clipboard(VTE_TERMINAL(data));
}

static GtkWidget *_terminal_build_menu_tab (GtkWidget *pWidget)
{
	GtkWidget *menu = gtk_menu_new ();

	GtkWidget *menu_item, *image;
	menu_item = gtk_image_menu_item_new_with_label (_D("Copy"));
	image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
        g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_copy), pWidget);

	menu_item = gtk_image_menu_item_new_with_label (_D("Paste"));
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
 	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_paste), pWidget);

	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_label (_D("New Tab"));
	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_new_tab), 0);

	menu_item = gtk_image_menu_item_new_with_label (_D("Close Tab"));
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_close_tab), 0);

	return menu;
}

gboolean applet_on_terminal_press_cb(GtkWidget *window, GdkEventButton *event, t_terminal *terminal)
{
	if (event->button == 3)
	{
		GtkWidget *menu = _terminal_build_menu_tab (window);
		
		gtk_widget_show_all (menu);
		
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
	return FALSE;
}
static void applet_on_terminal_eof(VteTerminal *vteterminal,
                                   gpointer     user_data)
{
  printf("youkata EOF\n");
}



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
  g_signal_connect (G_OBJECT (vterm), "eof",
                    G_CALLBACK (applet_on_terminal_eof), &term);

  cairo_dock_allow_widget_to_receive_data (vterm, G_CALLBACK (on_terminal_drag_data_received));
  gtk_notebook_append_page(GTK_NOTEBOOK(term.tab), vterm, NULL);
  gtk_widget_show(vterm);
}

static CairoDockDesklet *terminal_new_dialog()
{
  GtkWidget *vterm = NULL;

  term.tab = gtk_notebook_new();
  terminal_new_tab();
  gtk_widget_show(term.tab);

  g_signal_connect (G_OBJECT (term.tab), "button-release-event",
                    G_CALLBACK (applet_on_terminal_press_cb), &term);
  term.dialog = cd_desklet_new(0, term.tab, 0, 0);

  term_tab_apply_settings();
  return term.dialog;
}


CD_APPLET_ON_CLICK_BEGIN
{
  if (!term.dialog) {
    term.dialog = terminal_new_dialog();
  }
  else {
    cd_desklet_show(term.dialog);
    term_tab_apply_settings();
  }
}
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
  cd_desklet_hide(term.dialog);
}
CD_APPLET_ON_MIDDLE_CLICK_END



CD_APPLET_ON_BUILD_MENU_BEGIN
{
  CD_APPLET_ADD_SUB_MENU("Terminal", pSubMenu, CD_APPLET_MY_MENU);
  CD_APPLET_ADD_IN_MENU(_D("New Tab"), on_new_tab, pSubMenu);
  CD_APPLET_ADD_IN_MENU(_D("Close the current Tab"), on_close_tab, pSubMenu);
  CD_APPLET_ADD_SEPARATOR();
  CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END



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
#include "terminal-menu-functions.h"

extern AppletConfig myConfig;
extern AppletData myData;

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT ("This is a very simple terminal applet made by Cedric GESTES for Cairo-Dock");

static void terminal_new_tab();


void term_on_keybinding_pull(const char *keystring, gpointer user_data)
{
	if (myData.tab)
	{
		if (myDesklet)
			cairo_dock_show_desklet(myDesklet);
		else if (myData.dialog)
			cairo_dock_unhide_dialog(myData.dialog);
	}
	else
	{
		terminal_build_and_show_tab ();
	}
}


static void on_new_tab(GtkMenuItem *menu_item, gpointer *data)
{
  terminal_new_tab();
  term_apply_settings();
}

static void on_close_tab(GtkMenuItem *menu_item, gpointer *data)
{
  gint p = gtk_notebook_get_current_page(GTK_NOTEBOOK(myData.tab));
  if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(myData.tab)) > 1)
    gtk_notebook_remove_page(GTK_NOTEBOOK(myData.tab), p);
}

static void term_apply_settings_on_vterm(GtkWidget *vterm)
{
  g_return_if_fail (vterm != NULL);
  vte_terminal_set_colors(VTE_TERMINAL(vterm), &myConfig.forecolor, &myConfig.backcolor, NULL, 0);
  /*     vte_terminal_set_background_saturation(VTE_TERMINAL(vterm), 1.0); */
  /*     vte_terminal_set_background_transparent(VTE_TERMINAL(vterm), FALSE); */
#if GTK_MINOR_VERSION >= 12
  vte_terminal_set_opacity(VTE_TERMINAL(vterm), myConfig.transparency);
#endif
  vte_terminal_set_size(VTE_TERMINAL(vterm), myConfig.iNbColumns, myConfig.iNbRows);
  //    gtk_widget_queue_draw(myData.desklet->pWidget);
}


void term_apply_settings()
{
  int sz = 0;
  GtkWidget *vterm = NULL;

  if (myData.tab) {
    sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(myData.tab));
    for (int i = 0; i < sz; ++i) {
      vterm = gtk_notebook_get_nth_page(GTK_NOTEBOOK(myData.tab), i);
      term_apply_settings_on_vterm(vterm);
    }
  }
/*   if (myData.desklet) */
/*     gtk_window_set_keep_above(GTK_WINDOW(myData.desklet->pWidget), myConfig.always_on_top); */
  ///cd_keybinder_unbind(term.prev_shortcut, (CDBindkeyHandler)onKeybindingPull);
  ///term.prev_shortcut = term.shortcut;
  cd_keybinder_bind(myConfig.shortcut, (CDBindkeyHandler)term_on_keybinding_pull, (gpointer)NULL);
}

static void on_terminal_child_exited(VteTerminal *vteterminal,
                                     gpointer t)
{
  gint p = gtk_notebook_page_num(GTK_NOTEBOOK(myData.tab), GTK_WIDGET(vteterminal));
  gint sz = gtk_notebook_get_n_pages(GTK_NOTEBOOK(myData.tab));

  if (sz > 1)
    gtk_notebook_remove_page(GTK_NOTEBOOK(myData.tab), p);
  else {
    // \r needed to return to the beginning of the line
    vte_terminal_feed(VTE_TERMINAL(vteterminal), "Shell exited. Another one is launching...\r\n\n", -1);
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

gboolean applet_on_terminal_press_cb(GtkWidget *window, GdkEventButton *event, gpointer user_data)
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
  cd_message ("youkata EOF\n");
}



static void terminal_new_tab()
{
  GtkWidget *vterm = NULL;

  vterm = vte_terminal_new();
  //transparency enable, otherwise we cant change value after
#if GTK_MINOR_VERSION >= 12
  vte_terminal_set_opacity(VTE_TERMINAL(vterm), myConfig.transparency);
#endif
  vte_terminal_set_emulation(VTE_TERMINAL(vterm), "xterm");
  vte_terminal_set_size (VTE_TERMINAL (vterm), myConfig.iNbColumns, myConfig.iNbRows);
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
                    G_CALLBACK (applet_on_terminal_press_cb), NULL);
  g_signal_connect (G_OBJECT (vterm), "eof",
                    G_CALLBACK (applet_on_terminal_eof), NULL);

  cairo_dock_allow_widget_to_receive_data (vterm, G_CALLBACK (on_terminal_drag_data_received));
  int num_new_tab = gtk_notebook_append_page(GTK_NOTEBOOK(myData.tab), vterm, NULL);  /// creer un widget avec un label et un bouton 'close' a mettre a la place du NULL. garder une trace du label pour pouvoir le changer plus tard (numerotation ou repertoire courant ou nom utilisateur).
  gtk_widget_show(vterm);
  cd_message ("num_new_tab : %d\n", num_new_tab);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (myData.tab), num_new_tab);
}

void terminal_build_and_show_tab (void)
{
	myData.tab = gtk_notebook_new();
	terminal_new_tab();
	gtk_widget_show(myData.tab);
	
	term_apply_settings();
	
	if (myDock)
	{
		myData.dialog = cairo_dock_build_dialog (_D("Terminal"), myIcon, myDock, NULL, myData.tab, GTK_BUTTONS_NONE, NULL, NULL, NULL);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (myData.tab, myDesklet);
		myDesklet->renderer = term_draw_in_desklet;
	}
}

void term_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	
}


CD_APPLET_ON_CLICK_BEGIN
{
	if (! myData.tab)
		terminal_build_and_show_tab ();
	else if (myDesklet)
		cairo_dock_show_desklet (myDesklet);
	else if (myData.dialog)
		cairo_dock_unhide_dialog(myData.dialog);
}
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	if (myData.tab)
	{
		if (myDesklet)
			cairo_dock_hide_desklet(myDesklet);
		else if (myData.dialog)
			cairo_dock_hide_dialog (myData.dialog);
	}
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

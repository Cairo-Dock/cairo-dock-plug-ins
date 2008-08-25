/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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
#include <gdk/gdkkeysyms.h>
// gdk.h semble necessaire pour certains
#include <gdk/gdk.h>

#include <vte/vte.h>

#include "terminal-callbacks.h"
#include "terminal-struct.h"
#include "terminal-menu-functions.h"
#include "terminal-widget.h"


CD_APPLET_INCLUDE_MY_VARS


void term_on_keybinding_pull(const char *keystring, gpointer user_data)
{
	if (myData.tab)
	{
		if (myDesklet)
		{
			gboolean bHasFocus = (gtk_window_has_toplevel_focus (GTK_WINDOW (myDesklet->pWidget)) || GTK_WIDGET_HAS_FOCUS (myData.tab) || GTK_WIDGET_HAS_FOCUS (myDesklet->pWidget));
			if (! bHasFocus)
			{
				GtkWidget *vterm;
				int i, iNbPages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (myData.tab));
				for (i = 0; i < iNbPages && ! bHasFocus; ++i)
				{
					vterm = gtk_notebook_get_nth_page(GTK_NOTEBOOK(myData.tab), i);
					bHasFocus = GTK_WIDGET_HAS_FOCUS (vterm);
				}
			}
			g_print ("%s (%d)\n", __func__, bHasFocus);
			
			if (bHasFocus)
			{
				cairo_dock_hide_desklet(myDesklet);
			}
			else
			{
				cairo_dock_show_desklet(myDesklet);
				int iCurrentNumPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(myData.tab));
				GtkWidget *vterm = gtk_notebook_get_nth_page(GTK_NOTEBOOK(myData.tab), iCurrentNumPage);
				gtk_widget_grab_focus (vterm);
			}
		}
		else if (myData.dialog)
		{
			if (GTK_WIDGET_VISIBLE (myData.dialog->pWidget))
				cairo_dock_hide_dialog(myData.dialog);
			else
				cairo_dock_unhide_dialog(myData.dialog);
		}
	}
	else
	{
		terminal_build_and_show_tab ();
	}
}


static gchar *_get_label_and_color (const gchar *cLabel, GdkColor *pColor, gboolean *bColorSet)
{
	gchar *cUsefulLabel;
	gchar *str = strchr (cLabel, '>');
	if (cLabel != NULL && strncmp (cLabel, "<span color='", 13) == 0 && str != NULL)  // approximatif mais devrait suffire.
	{
		gchar *cColor = g_strndup (cLabel+13, 7);
		if (pColor != NULL)
			*bColorSet = gdk_color_parse (cColor, pColor);
		g_free (cColor);
		
		cUsefulLabel = g_strdup (str+1);
		str = strrchr (cLabel, '<');
		if (str != NULL && strcmp (str, "</span>") == 0)
			*str = '\0';
	}
	else
	{
		cUsefulLabel = g_strdup (cLabel);
	}
	return cUsefulLabel;
}

void terminal_rename_tab (GtkWidget *vterm)
{
	cd_message ("");
	if (vterm == NULL)
	{
		int iCurrentNumPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(myData.tab));
		vterm = gtk_notebook_get_nth_page (GTK_NOTEBOOK(myData.tab), iCurrentNumPage);
	}
	GtkWidget *pTabLabelWidget = gtk_notebook_get_tab_label (GTK_NOTEBOOK(myData.tab), vterm);
	GList *pTabWidgetList = gtk_container_get_children (GTK_CONTAINER (pTabLabelWidget));
	GtkLabel *pLabel;
	const gchar *cCurrentName;
	if (pTabWidgetList != NULL && pTabWidgetList->data != NULL)
	{
		GtkLabel *pLabel = pTabWidgetList->data;
		const gchar *cCurrentName = gtk_label_get_text (pLabel);
		GdkColor color;
		gboolean bColorSet = FALSE;
		gchar *cUsefulLabel = _get_label_and_color (cCurrentName, &color, &bColorSet);
		
		gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Set title for this tab :"), NULL, (myDock ? CAIRO_CONTAINER (myData.dialog) : CAIRO_CONTAINER (myDesklet)), cUsefulLabel);
		g_free (cUsefulLabel);
		
		if (cNewName != NULL)
		{
			if (bColorSet)
			{
				gchar *cColor = gdk_color_to_string (&color);
				gchar *cNewColoredName = g_strdup_printf ("<span color='%s'>%s</span>", cColor, cNewName);
				gtk_label_set_markup (pLabel, cNewColoredName);
				g_free (cNewColoredName);
				g_free (cColor);
			}
			else
			{
				gtk_label_set_text (pLabel, cNewName);
			}
			g_free (cNewName);
		}
	}
}

static void _set_color (GtkColorSelection *pColorSelection, GtkLabel *pLabel)
{
	GdkColor color;
	gtk_color_selection_get_current_color (pColorSelection, &color);
	
	gchar *cColor = gdk_color_to_string (&color);
	
	const gchar *cCurrentLabel = gtk_label_get_text (pLabel);
	gchar *cUsefulLabel = _get_label_and_color (cCurrentLabel, NULL, NULL);
	gchar *cNewLabel = g_strdup_printf ("<span color='%s'>%s</span>", cColor, cUsefulLabel);
	gtk_label_set_markup (pLabel, cNewLabel);
	
	g_free (cNewLabel);
	g_free (cUsefulLabel);
	g_free (cColor);
}
void terminal_change_color_tab (GtkWidget *vterm)
{
	cd_message ("");
	if (vterm == NULL)
	{
		int iCurrentNumPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(myData.tab));
		vterm = gtk_notebook_get_nth_page (GTK_NOTEBOOK(myData.tab), iCurrentNumPage);
	}
	GtkWidget *pTabLabelWidget = gtk_notebook_get_tab_label (GTK_NOTEBOOK(myData.tab), vterm);
	GList *pTabWidgetList = gtk_container_get_children (GTK_CONTAINER (pTabLabelWidget));
	GtkLabel *pLabel;
	const gchar *cCurrentName = NULL;
	if (pTabWidgetList != NULL && pTabWidgetList->data != NULL)
	{
		GtkLabel *pLabel = pTabWidgetList->data;
		
		GtkWidget *pColorDialog = gtk_color_selection_dialog_new (D_("Select a color"));
		
		const gchar *cCurrentLabel = gtk_label_get_text (pLabel);
		GdkColor color;
		gboolean bColorSet = FALSE;
		gchar *cUsefulLabel = _get_label_and_color (cCurrentLabel, &color, &bColorSet);
		if (bColorSet)
		{
			gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (((GtkColorSelectionDialog *) pColorDialog)->colorsel), &color);
		}
		
		gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (((GtkColorSelectionDialog *) pColorDialog)->colorsel), FALSE);
		g_signal_connect (((GtkColorSelectionDialog *) pColorDialog)->colorsel, "color-changed", GTK_SIGNAL_FUNC (_set_color), pLabel);
		gtk_widget_hide (((GtkColorSelectionDialog *) pColorDialog)->cancel_button);
		gtk_widget_hide (((GtkColorSelectionDialog *) pColorDialog)->help_button);
		g_signal_connect_swapped (((GtkColorSelectionDialog *) pColorDialog)->ok_button, "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), pColorDialog);
		
		gtk_window_present (GTK_WINDOW (pColorDialog));
	}
}

void terminal_close_tab (GtkWidget *vterm)
{
	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(myData.tab)) > 1)
	{
		int iNumPage;
		if (vterm == NULL)
		{
			iNumPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(myData.tab));
		}
		else
		{
			iNumPage = gtk_notebook_page_num (GTK_NOTEBOOK(myData.tab), vterm);
		}
		gtk_notebook_remove_page(GTK_NOTEBOOK(myData.tab), iNumPage);
	}
}


static void term_apply_settings_on_vterm(GtkWidget *vterm)
{
  g_return_if_fail (vterm != NULL);
  vte_terminal_set_colors(VTE_TERMINAL(vterm), &myConfig.forecolor, &myConfig.backcolor, NULL, 0);
      //vte_terminal_set_background_saturation(VTE_TERMINAL(vterm), 1.0);
      //vte_terminal_set_background_transparent(VTE_TERMINAL(vterm), TRUE);
#if GTK_MINOR_VERSION >= 12
  vte_terminal_set_opacity(VTE_TERMINAL(vterm), myConfig.transparency);
#endif
  
	if (myDock)
	{
		//g_print ("set_size (%d , %d)\n", myConfig.iNbColumns, myConfig.iNbRows);
		gtk_widget_set (vterm, "width-request", 0, NULL);
		gtk_widget_set (vterm, "height-request", 0, NULL);
		vte_terminal_set_size(VTE_TERMINAL(vterm), myConfig.iNbColumns, myConfig.iNbRows);
		GtkRequisition requisition = {0, 0};
		gtk_widget_size_request (vterm, &requisition);
		//g_print (" -> %dx%d\n", requisition.width, requisition.height);
		if (myData.dialog)
			gtk_window_resize (GTK_WINDOW (myData.dialog->pWidget), requisition.width, requisition.height);
	}
	else
	{
		gtk_widget_set (vterm, "width-request", 64, NULL);
		gtk_widget_set (vterm, "height-request", 64, NULL);
	}
}


void term_apply_settings (void)
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



static void on_new_tab (GtkMenuItem *menu_item, gpointer *data)
{
	terminal_new_tab();
}
static void on_rename_tab (GtkMenuItem *menu_item, GtkWidget *vterm)
{
	terminal_rename_tab (vterm);
}
static void on_change_tab_color (GtkMenuItem *menu_item, GtkWidget *vterm)
{
	terminal_change_color_tab (vterm);
}
static void on_close_tab (GtkMenuItem *menu_item, GtkWidget *vterm)
{
	terminal_close_tab (vterm);
}
static GtkWidget *_terminal_build_menu_tab (GtkWidget *vterm)
{
	GtkWidget *menu = gtk_menu_new ();
	
	GtkWidget *menu_item, *image;
	if (vterm)
	{
		menu_item = gtk_image_menu_item_new_with_label (D_("Copy"));
		image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_copy), vterm);
		
		menu_item = gtk_image_menu_item_new_with_label (D_("Paste"));
		image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_paste), vterm);
		
		menu_item = gtk_separator_menu_item_new ();
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	}

  menu_item = gtk_image_menu_item_new_with_label (D_("New Tab"));
  image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_new_tab), vterm);
  
  menu_item = gtk_image_menu_item_new_with_label (D_("Rename this Tab"));
  image = gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_rename_tab), vterm);

  menu_item = gtk_image_menu_item_new_with_label (D_("Change this Tab's color"));
  image = gtk_image_new_from_stock (GTK_STOCK_COLOR_PICKER, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_change_tab_color), vterm);

  menu_item = gtk_image_menu_item_new_with_label (D_("Close this Tab"));
  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(on_close_tab), vterm);

  return menu;
}

static gboolean applet_on_terminal_press_cb(GtkWidget *vterm, GdkEventButton *event, gpointer user_data)
{
  g_print ("%s ()\n", __func__);
  if (event->button == 3)
    {
      GtkWidget *menu = _terminal_build_menu_tab (vterm);

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
  cd_debug ("youkata EOF");
}
static gboolean on_key_press_term (GtkWidget *pWidget,
	GdkEventKey *pKey,
	gpointer data)
{
	if (pKey->type == GDK_KEY_PRESS && (pKey->state & GDK_CONTROL_MASK))
	{
		switch (pKey->keyval)
		{
			case GDK_t :
				terminal_new_tab();
			break ;
			case GDK_w :
				terminal_close_tab (NULL);
			break ;
			default :
			break ;
		}
	}
	return FALSE;
}
static gchar * _terminal_get_tab_name (int iNumPage)
{
	GtkWidget *vterm = gtk_notebook_get_nth_page (GTK_NOTEBOOK(myData.tab), iNumPage);
	GtkWidget *pTabLabelWidget = gtk_notebook_get_tab_label (GTK_NOTEBOOK(myData.tab), vterm);
	GList *pTabWidgetList = gtk_container_get_children (GTK_CONTAINER (pTabLabelWidget));
	GtkLabel *pLabel;
	const gchar *cCurrentName;
	if (pTabWidgetList != NULL && pTabWidgetList->data != NULL)
	{
		GtkLabel *pLabel = pTabWidgetList->data;
		const gchar *cCurrentName = gtk_label_get_text (pLabel);
		return _get_label_and_color (cCurrentName, NULL, NULL);
	}
	return NULL;
}
void terminal_new_tab(void)
{
  GtkWidget *vterm = NULL;

  vterm = vte_terminal_new();
  //transparency enable, otherwise we cant change value after
  //vte_terminal_set_background_transparent(VTE_TERMINAL(vterm), TRUE);
#if GTK_MINOR_VERSION >= 12
  vte_terminal_set_opacity(VTE_TERMINAL(vterm), myConfig.transparency);
#endif
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
                    G_CALLBACK (on_terminal_child_exited), NULL);
  g_signal_connect (G_OBJECT (vterm), "button-release-event",
                    G_CALLBACK (applet_on_terminal_press_cb), NULL);
  g_signal_connect (G_OBJECT (vterm),
		"key-press-event",
		G_CALLBACK (on_key_press_term),
		NULL);
  g_signal_connect (G_OBJECT (vterm), "eof",
                    G_CALLBACK (applet_on_terminal_eof), NULL);

  cairo_dock_allow_widget_to_receive_data (vterm, G_CALLBACK (on_terminal_drag_data_received), NULL);

	GtkWidget *pHBox = gtk_hbox_new (FALSE, 0);
	
	//\_________________On choisit un nom qui ne soit pas deja present, de la forme " # n ".
	int i, iNbPages = gtk_notebook_get_n_pages (GTK_NOTEBOOK(myData.tab));
	GList *pTabNameList = NULL;
	for (i = 0; i < iNbPages; i ++)
	{
		pTabNameList = g_list_prepend (pTabNameList, _terminal_get_tab_name (i));
	}
	int iChoosedNum = 1;
	gchar *cLabel = g_strdup_printf (" # %d ", iChoosedNum);
	gchar *cTabName;
	GList *pElement = pTabNameList;
	do
	{
		if (pElement == NULL)
			break ;
		cTabName = pElement->data;
		if (cTabName != NULL && strcmp (cTabName, cLabel) == 0)
		{
			g_free (cLabel);
			iChoosedNum ++;
			cLabel = g_strdup_printf (" # %d ", iChoosedNum);
			
			g_free (cTabName);
			pElement->data = NULL;
			pElement = pTabNameList;
		}
		else
			pElement = pElement->next;
	} while (TRUE);
	g_list_foreach (pTabNameList, (GFunc) g_free, NULL);
	g_list_free (pTabNameList);
	
	GtkWidget *pLabel = gtk_label_new (cLabel);
	g_free (cLabel);
	gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
	gtk_box_pack_start (GTK_BOX (pHBox),
		pLabel,
		FALSE,
		FALSE,
		0);
	
	GtkWidget *pButton = gtk_button_new_with_label ("x");
	g_signal_connect (G_OBJECT (pButton),
		"clicked",
		G_CALLBACK (on_close_tab),
		NULL);
	gtk_box_pack_start (GTK_BOX (pHBox),
		pButton,
		FALSE,
		FALSE,
		0);
	
	gtk_widget_show_all (pHBox);
	int num_new_tab = gtk_notebook_append_page(GTK_NOTEBOOK(myData.tab), vterm, pHBox);
	GtkWidget *pNewTab = gtk_notebook_get_nth_page (GTK_NOTEBOOK(myData.tab), num_new_tab);
	
  gtk_widget_show(vterm);
  cd_message ("num_new_tab : %d", num_new_tab);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (myData.tab), num_new_tab);
  
  term_apply_settings_on_vterm (vterm);
}


static void _hide_show_tab_button (GtkNotebook *pNotebook, int iNumPage, gboolean bShow)
{
	GtkWidget *pPageChild = gtk_notebook_get_nth_page (pNotebook, iNumPage);
	GtkWidget *pTabLabelWidget = gtk_notebook_get_tab_label (pNotebook, pPageChild);
	GList *pTabWidgetList = gtk_container_get_children (GTK_CONTAINER (pTabLabelWidget));
	if (pTabWidgetList != NULL && pTabWidgetList->next != NULL)
	{
		GtkWidget *pButton = pTabWidgetList->next->data;
		if (pButton != NULL)
		{
			if (bShow)
				gtk_widget_show (pButton);
			else
				gtk_widget_hide (pButton);
		}
	}
	g_list_free (pTabWidgetList);
}
static void on_switch_page (GtkNotebook *pNotebook,
	GtkNotebookPage *pNextPage,
	guint iNextNumPage,
	gpointer user_data)
{
	int iCurrentNumPage = gtk_notebook_get_current_page (pNotebook);
	
	_hide_show_tab_button (pNotebook, iCurrentNumPage, FALSE);
	_hide_show_tab_button (pNotebook, iNextNumPage, TRUE);
}
static GtkWidget * _terminal_find_clicked_tab_child (int x, int y)  // x,y relativement au notebook.
{
	GtkRequisition requisition;
	GtkWidget *pPageChild, *pTabLabelWidget, *vterm = NULL;
	
	int iMaxTabHeight = 0;
	int iCurrentNumPage = gtk_notebook_get_current_page (GTK_NOTEBOOK (myData.tab));
	pPageChild = gtk_notebook_get_nth_page(GTK_NOTEBOOK(myData.tab), iCurrentNumPage);
	pTabLabelWidget = gtk_notebook_get_tab_label (GTK_NOTEBOOK(myData.tab), pPageChild);
	gtk_widget_get_child_requisition (pTabLabelWidget, &requisition);
	iMaxTabHeight = requisition.height;
	//g_print ("iMaxTabHeight : %d\n", iMaxTabHeight);
	
	int i, iNbPages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (myData.tab));
	gint src_x, src_y, dest_x, dest_y;
	for (i = 0; i < iNbPages; ++i)
	{
		pPageChild = gtk_notebook_get_nth_page(GTK_NOTEBOOK(myData.tab), i);
		pTabLabelWidget = gtk_notebook_get_tab_label (GTK_NOTEBOOK(myData.tab), pPageChild);
		gtk_widget_get_child_requisition (pTabLabelWidget, &requisition);
		gtk_widget_translate_coordinates (myData.tab,
			pTabLabelWidget,
			x, y,
			&dest_x,
			&dest_y);
		//g_print ("%d) (%d;%d) (%dx%d)\n", i, dest_x, dest_y, requisition.width, requisition.height);
		if (dest_x >= 0 && dest_y >= 0 && dest_x <= requisition.width && dest_y <= iMaxTabHeight)
		{
			vterm = pPageChild;
			break;
		}
	}
	return vterm;
}
static gboolean on_button_press_tab (GtkWidget* pWidget,
	GdkEventButton* pButton,
	GtkWidget *vterm)
{
	//g_print ("%s (%d;%d)\n", __func__, (int)pButton->x, (int)pButton->y);
	if (pButton->type == GDK_2BUTTON_PRESS)
	{
		if (vterm == NULL)
			vterm = _terminal_find_clicked_tab_child (pButton->x, pButton->y);
		if (vterm == NULL)
			terminal_new_tab ();
		else
			terminal_rename_tab (vterm);
	}
	else if (pButton->button == 3)
	{
		if (vterm == NULL)
			vterm = _terminal_find_clicked_tab_child (pButton->x, pButton->y);
		if (vterm != NULL)
		{
			GtkWidget *menu = _terminal_build_menu_tab (vterm);
			
			gtk_widget_show_all (menu);
			
			gtk_menu_popup (GTK_MENU (menu),
				NULL,
				NULL,
				NULL,
				NULL,
				1,
				gtk_get_current_event_time ());
			return TRUE;  // on empeche le menu de cairo-dock d'apparaitre par-dessus.
		}
	}
	else if (pButton->button == 2)
	{
		if (vterm == NULL)
			vterm = _terminal_find_clicked_tab_child (pButton->x, pButton->y);
		if (vterm != NULL)
		{
			terminal_close_tab (vterm);
		}
	}
	return FALSE;
}
void terminal_build_and_show_tab (void)
{
	myData.tab = gtk_notebook_new();
	g_signal_connect (G_OBJECT (myData.tab),
		"switch-page",
		G_CALLBACK (on_switch_page),
		NULL);
	g_signal_connect (G_OBJECT (myData.tab),
		"button-press-event",
		G_CALLBACK (on_button_press_tab),
		NULL);
	g_signal_connect (G_OBJECT (myData.tab),
		"key-press-event",
		G_CALLBACK (on_key_press_term),
		NULL);
	
	terminal_new_tab();
	gtk_widget_show(myData.tab);

	term_apply_settings();

	if (myDock)
	{
		myData.dialog = cairo_dock_build_dialog (D_("Terminal"), myIcon, myContainer, NULL, myData.tab, GTK_BUTTONS_NONE, NULL, NULL, NULL);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (myData.tab, myDesklet);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
}

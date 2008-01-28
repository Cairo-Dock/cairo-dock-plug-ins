/*
** Login : <ctaf42@gmail.com>
** Started on  Sun Jan 27 18:35:38 2008 Cedric GESTES
** $Id$
**
** Copyright (C) 2008 Cedric GESTES
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
#include <math.h>

#include <cairo-dock.h>
#include "cairo-applet.h"

static gboolean applet_on_expose_dialog (GtkWidget *pWidget,
                                         GdkEventExpose *pExpose,
                                         CairoDockDialog *pDialog)
{
  gint w = 0, h = 0;
  int border = 7;

  if (!pDialog)
    return FALSE;

  cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
  if (cairo_status (pCairoContext) != CAIRO_STATUS_SUCCESS, FALSE) {
    cairo_destroy (pCairoContext);
    return FALSE;
  }

  //erase the background
  cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
  cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint (pCairoContext);

  //set the color
  cairo_set_source_rgba (pCairoContext, 1., 1., 1., 0.7);
  cairo_set_line_width (pCairoContext, 10.0);
  cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_ROUND);

  //draw a rounded square
  gtk_window_get_size(GTK_WINDOW(pDialog->pWidget), &w, &h);
  cairo_move_to (pCairoContext, border, border);
  cairo_rel_line_to (pCairoContext, w - (border << 1), 0);
  cairo_rel_line_to (pCairoContext, 0, h - (border << 1));
  cairo_rel_line_to (pCairoContext, -(w - (border << 1)) , 0);
  cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
  cairo_close_path (pCairoContext);
  cairo_stroke (pCairoContext);

/*   cairo_set_source_rgba (pCairoContext, 1., 1., 1., 1.); */
  cairo_rectangle(pCairoContext, border, border, (w - (border << 1)), (h - (border << 1)));
  cairo_fill(pCairoContext);

  cairo_destroy (pCairoContext);
  return FALSE;
}

static void applet_on_click_close(GtkButton *button, gpointer   user_data)
{
}

static volatile gboolean loool_kikou_mdr_move = FALSE;
static volatile gint loool_x = 0, loool_y = 0;

static gboolean applet_on_release(GtkWidget      *widget,
                                  GdkEventButton *event,
                                  gpointer        user_data)
{
  loool_kikou_mdr_move = FALSE;
  return TRUE;
}

static gboolean applet_on_click(GtkWidget      *widget,
                                GdkEventButton *event,
                                gpointer        user_data)
{
  if (((GdkEventButton *)event)->button == 1) {
    loool_kikou_mdr_move = TRUE;
    loool_x = -((GdkEventButton *)event)->x;
    loool_y = -((GdkEventButton *)event)->y;
    return TRUE;
  }
}

static gboolean applet_on_event_move(GtkWidget *widget,
                                     GdkEvent *event,
                                     CairoDockDialog *pDialog)
{
  if (loool_kikou_mdr_move && event->type == GDK_MOTION_NOTIFY) {
    gtk_window_move(GTK_WINDOW(pDialog->pWidget), event->motion.x_root + loool_x, event->motion.y_root + loool_y);
    return TRUE;
  }
  return FALSE;
}

static void applet_on_click_nba(GtkButton *button, CairoDockDialog *pDialog)
{
  gchar *lbl = gtk_button_get_label(button);

  if (!strcmp(lbl, "on top")) {
    gtk_window_set_keep_above(GTK_WINDOW(pDialog->pWidget), FALSE);
    gtk_window_set_keep_below(GTK_WINDOW(pDialog->pWidget), FALSE);
    gtk_button_set_label(button, "normal");
  } else if (!strcmp(lbl, "below")) {
    gtk_window_set_keep_above(GTK_WINDOW(pDialog->pWidget), TRUE);
    gtk_button_set_label(button, "on top");
  } else if (!strcmp(lbl, "normal")) {
    gtk_window_set_keep_below(GTK_WINDOW(pDialog->pWidget), TRUE);
    gtk_button_set_label(button, "below");
  }
}



/* void applet_isolate_dialog (CairoDockDialog *pDialog) */
/* { */
/* 	if (pDialog == NULL) */
/* 		return ; */

/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_expose_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_button_press_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_configure_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_enter_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_leave_dialog, NULL); */
/* } */


void applet_free_dialog (CairoDockDialog *pDialog)
{
  if (pDialog == NULL)
    return;

  cairo_surface_destroy (pDialog->pTextBuffer);
  pDialog->pTextBuffer = NULL;

  gtk_widget_destroy (pDialog->pWidget);  // detruit aussi le widget interactif.
  pDialog->pWidget = NULL;

  if (pDialog->pUserData != NULL && pDialog->pFreeUserDataFunc != NULL)
    pDialog->pFreeUserDataFunc (pDialog->pUserData);

  g_free (pDialog);
}


CairoDockDialog *applet_build_dialog (CairoDock *pDock, GtkWidget *pInteractiveWidget, gpointer data)
{
  GtkWidget* vbox, *hbox, *btn;
  CairoDockDialog *pDialog = g_new0 (CairoDockDialog, 1);
  GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  pDialog->pWidget = pWindow;
  pDialog->pUserData = data;
  pDialog->pInteractiveWidget = pInteractiveWidget;

  gtk_window_stick(GTK_WINDOW(pWindow));
  gtk_window_set_keep_above(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
  cairo_dock_set_colormap_for_window(pWindow);
  gtk_widget_set_app_paintable(pWindow, TRUE);
  gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_title(GTK_WINDOW(pWindow), "cairo-dock-dialog");
  gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
  //the border is were cairo paint
  gtk_container_set_border_width(GTK_CONTAINER(pWindow), 10);
  gtk_window_set_default_size(GTK_WINDOW(pWindow), 32, 32);
  g_signal_connect (G_OBJECT (pWindow), "expose-event",
                    G_CALLBACK (applet_on_expose_dialog), pDialog);

  vbox = gtk_vbox_new(0, 0);
  gtk_container_add(GTK_CONTAINER(pWindow), vbox);

  hbox = gtk_hbox_new(0, 1);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, 0, 0, 0);

  btn = gtk_button_new_with_label("X");
  gtk_box_pack_end(GTK_BOX(hbox), btn, 0, 1, 0);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (applet_on_click_close), pDialog);

  btn = gtk_button_new_with_label("on top");
  gtk_box_pack_end(GTK_BOX(hbox), btn, 0, 1, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (applet_on_click_nba), pDialog);


  g_signal_connect (G_OBJECT (pWindow), "event",
                    G_CALLBACK (applet_on_event_move), pDialog);
  g_signal_connect (G_OBJECT (pWindow), "button-press-event",
                    G_CALLBACK (applet_on_click), pDialog);
  g_signal_connect (G_OBJECT (pWindow), "button-release-event",
                    G_CALLBACK (applet_on_release), pDialog);

  //user widget
  if (pInteractiveWidget != NULL)
    gtk_box_pack_start(GTK_BOX(vbox), pInteractiveWidget, 0, 0, 0);
  gtk_widget_show_all(pWindow);
  return pDialog;
}


//TODO lol
static gint root_x = 0, root_y = 0;

void applet_hide_dialog(CairoDockDialog *pDialog)
{
  gint x, y;

  if (!pDialog)
    return ;
  gtk_window_get_position(GTK_WINDOW(pDialog->pWidget), &x, &y);
  root_x = (x > 0 ? x : root_x);
  root_y = (y > 0 ? y : root_y);
  gtk_widget_hide (pDialog->pWidget);
}

void applet_unhide_dialog (CairoDockDialog *pDialog)
{
  if (!pDialog)
    return;
  gtk_window_present(GTK_WINDOW(pDialog->pWidget));
  gtk_window_move(GTK_WINDOW(pDialog->pWidget), root_x, root_y);
}

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
#include "cairo-dock-desklet.h"

static gboolean cd_desklet_on_expose(GtkWidget *pWidget,
                                     GdkEventExpose *pExpose,
                                     CairoDockDesklet *pDialog)
{
  gint w = 0, h = 0;
  int border = 7;

  if (!pDialog)
    return FALSE;

  cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
  if (cairo_status(pCairoContext) != CAIRO_STATUS_SUCCESS) {
    cairo_destroy (pCairoContext);
    return FALSE;
  }

  //erase the background
  cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
  cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint (pCairoContext);
cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);

  //set the color
  if (gtk_window_is_active(GTK_WINDOW(pDialog->pWidget)))  /// je verrais bien un changement au survol (utiliser un flag bIsInside et les enter & leave events.) On pourrait meme avoir les 3 etats.
    ///eviter a tous prix les flags qui servent a rien qd on peut determiner l'etat autrement, flag a mettre a js = source d'erreur
    cairo_set_source_rgba (pCairoContext, 1., 1., 1., 0.7);
  else
    cairo_set_source_rgba (pCairoContext, 1., 1., 1., 0.3);

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

static void cd_desklet_on_close(GtkButton *widget, CairoDockDesklet *pDialog)
{
  cd_desklet_hide(pDialog);
}

static gboolean cd_desklet_on_release(GtkWidget      *widget,
                                      GdkEventButton *event,
                                      CairoDockDesklet *dialog)
{
  dialog->moving = FALSE;
  return TRUE;
}

static gboolean cd_desklet_on_click(GtkWidget      *widget,
                                    GdkEventButton *event,
                                    CairoDockDesklet *dialog)
{
  if (((GdkEventButton *)event)->button == 1) {  /// utiliser le champ 'state' des MotionEvent, comme pour les docks.
    dialog->moving = TRUE;
    dialog->diff_x = -((GdkEventButton *)event)->x;
    dialog->diff_y = -((GdkEventButton *)event)->y;
    return TRUE;
  }
}

static gboolean cd_desklet_on_event(GtkWidget *widget,
                                    GdkEvent *event,
                                    CairoDockDesklet *pDialog)
{
  if (pDialog->moving && event->type == GDK_MOTION_NOTIFY) {
    gtk_window_move(GTK_WINDOW(pDialog->pWidget),
                    event->motion.x_root + pDialog->diff_x,
                    event->motion.y_root + pDialog->diff_y);
    return TRUE;
  }
  return FALSE;
}

static void cd_desklet_on_above(GtkWidget *widget, CairoDockDesklet *pDialog)
{
  gtk_window_set_keep_below(GTK_WINDOW(pDialog->pWidget), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW(pDialog->pWidget), TRUE);
}

static void cd_desklet_on_normal(GtkWidget *widget, CairoDockDesklet *pDialog)
{
  gtk_window_set_keep_above(GTK_WINDOW(pDialog->pWidget), FALSE);
  gtk_window_set_keep_below(GTK_WINDOW(pDialog->pWidget), FALSE);
}

static void cd_desklet_on_below(GtkWidget *widget, CairoDockDesklet *pDialog)
{
  gtk_window_set_keep_above(GTK_WINDOW(pDialog->pWidget), FALSE);
  gtk_window_set_keep_below(GTK_WINDOW(pDialog->pWidget), TRUE);
}

//for compiz fusion "widget layer"
//set behaviour in compiz to: (name=cairo-dock & type=utility)
static void cd_desklet_on_widget_layer(GtkWidget *widget, CairoDockDesklet *pDialog)
{
  cd_desklet_hide(pDialog);
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
    gtk_window_set_type_hint(GTK_WINDOW(pDialog->pWidget), GDK_WINDOW_TYPE_HINT_UTILITY);
  else
    gtk_window_set_type_hint(GTK_WINDOW(pDialog->pWidget), GDK_WINDOW_TYPE_HINT_NORMAL);
  cd_desklet_show(pDialog);
}

static GtkWidget *cd_desklet_build_menu(CairoDockDesklet *pDialog)
{
  GtkWidget *menu = gtk_menu_new ();
  GtkWidget *menu_item, *image;
  GSList *group = NULL;

  menu_item = gtk_image_menu_item_new_with_label(_D("Close"));
  image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(cd_desklet_on_close), pDialog);

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_radio_menu_item_new_with_label(group, _D("Always on top"));
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(cd_desklet_on_above), pDialog);

  menu_item = gtk_radio_menu_item_new_with_label(group, _D("Normal"));
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(cd_desklet_on_normal), pDialog);

  menu_item = gtk_radio_menu_item_new_with_label(group, _D("Always below"));
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(cd_desklet_on_below), pDialog);

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_check_menu_item_new_with_label(_D("Compiz Fusion Widget"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(cd_desklet_on_widget_layer), pDialog);
  return menu;
}

static void cd_desklet_on_click_nbt(GtkButton *button, CairoDockDesklet *pDialog)
{
  gtk_menu_popup(GTK_MENU(pDialog->pMenu), NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time());
}


void cd_desklet_free(CairoDockDesklet *pDialog)
{
  if (pDialog == NULL)
    return;

  gtk_widget_destroy (pDialog->pWidget);  // detruit aussi le widget interactif.
  pDialog->pWidget = NULL;

  if (pDialog->pUserData != NULL && pDialog->pFreeUserDataFunc != NULL)
    pDialog->pFreeUserDataFunc (pDialog->pUserData);
  g_free(pDialog);
}

static gboolean cd_desklet_on_focus_in_out(GtkWidget *widget,
                                           GdkEventFocus *event,
                                           CairoDockDesklet *pDialog)
{
  if (!pDialog)
    return FALSE;
  gtk_widget_queue_draw(pDialog->pWidget);
  return FALSE;
}

CairoDockDesklet *cd_desklet_new(Icon *pIcon,
                                 GtkWidget *pInteractiveWidget,
                                 gpointer data,
                                 GFreeFunc freefunc)
{
  g_print ("%s ()\n", __func__);
  GtkWidget* vbox, *hbox, *btn;
  CairoDockDesklet *pDialog = g_new0(CairoDockDesklet, 1);
  GtkWidget* pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  pDialog->x = pDialog->y = 0;
  pDialog->pWidget = pWindow;
  pDialog->pUserData = data;
  pDialog->pFreeUserDataFunc = freefunc;
  pDialog->pInteractiveWidget = pInteractiveWidget;

  gtk_window_stick(GTK_WINDOW(pWindow));
  gtk_window_set_keep_above(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
  cairo_dock_set_colormap_for_window(pWindow);
  gtk_widget_set_app_paintable(pWindow, TRUE);
  gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_title(GTK_WINDOW(pWindow), "cairo-dock-desklet");  /// tilte = nom de l'applet, et class = "cairo-dock-desklet" serait peut-etre plus interessant pour le WM.
  gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_FOCUS_CHANGE_MASK);
  gtk_window_set_policy(GTK_WINDOW(pWindow), 0, 0, 1);
  //the border is were cairo paint
  gtk_container_set_border_width(GTK_CONTAINER(pWindow), 10);
  gtk_window_set_default_size(GTK_WINDOW(pWindow), 32, 32);
  g_signal_connect (G_OBJECT (pWindow), "expose-event",
                    G_CALLBACK (cd_desklet_on_expose), pDialog);

  hbox = gtk_hbox_new(0, 0);
  gtk_container_add(GTK_CONTAINER(pWindow), hbox);

  vbox = gtk_vbox_new(0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, 0, 0, 0);

  btn = gtk_button_new_with_label("-");
  gtk_box_pack_start(GTK_BOX(vbox), btn, 0, 0, 0);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (cd_desklet_on_click_nbt), pDialog);

  g_signal_connect (G_OBJECT (pWindow), "event",
                    G_CALLBACK (cd_desklet_on_event), pDialog);  /// motion-notify
  g_signal_connect (G_OBJECT (pWindow), "button-press-event",
                    G_CALLBACK (cd_desklet_on_click), pDialog);  /// il manque un configure-event por choper la taille et la position.
  g_signal_connect (G_OBJECT (pWindow), "button-release-event",
                    G_CALLBACK (cd_desklet_on_release), pDialog);
  g_signal_connect (G_OBJECT (pWindow), "focus-in-event",
                    G_CALLBACK (cd_desklet_on_focus_in_out), pDialog);  /// distinguer les 2 pour eviter d'avoir a faire le test dans le dessin.
  ///non justement
  g_signal_connect (G_OBJECT (pWindow), "focus-out-event",
                    G_CALLBACK (cd_desklet_on_focus_in_out), pDialog);  /// pourquoi pas enter-notify-event et leave-notify-event au fait ?

  pDialog->pMenu = cd_desklet_build_menu(pDialog);
  gtk_widget_show_all(pDialog->pMenu);

  //user widget
  if (pInteractiveWidget != NULL)
  {
    g_print ("ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);
    if (gtk_widget_get_parent (pInteractiveWidget) != NULL)
      {
        gtk_object_ref((gpointer)pInteractiveWidget);
        gtk_widget_unparent(pInteractiveWidget);
      }
    else
      gtk_object_ref((gpointer)pInteractiveWidget);
    gtk_box_pack_start(GTK_BOX(hbox), pInteractiveWidget, 1, 1, 0);
    gtk_object_unref((gpointer)pInteractiveWidget);
    g_print ("pack -> ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);
  }

  gtk_widget_show_all(pWindow);
  return pDialog;
}

void cd_desklet_hide(CairoDockDesklet *pDialog)
{
  gint x, y;

  if (!pDialog)
    return ;
  gtk_window_get_position(GTK_WINDOW(pDialog->pWidget), &x, &y);
  pDialog->x = (x > 0 ? x : pDialog->x);
  pDialog->y = (y > 0 ? y : pDialog->y);
  gtk_widget_hide (pDialog->pWidget);
}

void cd_desklet_show(CairoDockDesklet *pDialog)
{
  if (!pDialog)
    return;
  gtk_window_present(GTK_WINDOW(pDialog->pWidget));
  //gtk_window_move(GTK_WINDOW(pDialog->pWidget), pDialog->x, pDialog->y);
}


GtkWidget *cd_desklet_steal_widget_from_desklet (CairoDockDesklet *pDialog)
{
	static GtkWidget *pWidgetCatcher = NULL;
	if (pWidgetCatcher == NULL)
		pWidgetCatcher = gtk_hbox_new (0, FALSE);

	GtkWidget *pInteractiveWidget = pDialog->pInteractiveWidget;
	if (pInteractiveWidget != NULL)
	{
		gtk_widget_reparent (pInteractiveWidget, pWidgetCatcher);  // j'ai rien trouve de mieux pour empecher que le 'pInteractiveWidget' ne soit pas detruit avec le dialogue apres l'appel de la callback (g_object_ref ne marche pas).
		g_print ("reparent -> ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);

		gtk_object_ref (GTK_OBJECT (pInteractiveWidget));
		gtk_widget_unparent (pInteractiveWidget);
		g_print ("ref+unparent -> ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);

		pDialog->pInteractiveWidget = NULL;
	}
	return pInteractiveWidget;
}

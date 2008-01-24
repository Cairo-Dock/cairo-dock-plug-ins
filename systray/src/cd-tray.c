/*
 * Copyright (C) 2007 Neil Jagdish Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include <math.h>

#include <cairo-dock.h>
#include "cairo-applet.h"
#include "cd-tray.h"
#include "systray-init.h"

CD_APPLET_INCLUDE_MY_VARS
extern t_systray systray;

static const guint icon_size_w = 24;
static const guint icon_size_h = 24;

static void
tray_icon_added (NaTrayManager *manager,
                 GtkWidget      *icon,
                 TrayApplet     *applet);

static void
tray_icon_removed (NaTrayManager *manager,
                   GtkWidget      *icon,
                   TrayApplet     *applet);

static void
tray_icon_message_sent (NaTrayManager *manager,
                        GtkWidget      *icon,
                        const char     *text,
                        glong           id,
                        glong           timeout,
                        TrayApplet     *applet);

static void
tray_icon_message_cancelled (NaTrayManager *manager,
                             GtkWidget      *icon,
                             glong           id,
                             TrayApplet     *applet);



static void tray_resize_container(TrayApplet *applet)
{
  GtkRequisition req;

/*   gtk_widget_size_request(GTK_WIDGET(applet->box), &req); */
/*   if (systray.dialog && &systray.dialog->pWidget) */
/*     gtk_widget_set_size_request(GTK_WINDOW(systray.dialog->pWidget), req.width, req.height); */
  //    gtk_window_set_default_size(GTK_WINDOW(systray.dialog->pWidget), req.width, req.height);
}



static gboolean idle_redraw_cb (TrayApplet *applet)
{
  applet->idle_redraw_id = 0;
  gtk_widget_hide(applet->box);
  gtk_widget_show(applet->box);
  return FALSE;
}

static void force_redraw (TrayApplet *applet)
{
  /* Force the icons to redraw their backgrounds.
   * gtk_widget_queue_draw() doesn't work across process boundaries,
   * so we do this instead.
   */
  if (applet->idle_redraw_id == 0)
    applet->idle_redraw_id = g_idle_add ((GSourceFunc) idle_redraw_cb, applet);
}


static void
tray_icon_added (NaTrayManager *manager,
                 GtkWidget      *icon,
                 TrayApplet     *applet)
{
  applet->icons = g_list_append (applet->icons, icon);

  gtk_widget_set_colormap(icon, gdk_screen_get_rgb_colormap (gdk_screen_get_default()));
  //gtk_widget_set_size_request(icon, icon_size_w, icon_size_h);

  //gtk_widget_show(icon);
  gtk_box_pack_start(GTK_BOX(applet->box), icon, TRUE, TRUE, 0);
  //dont know why but it's screew up if we dont do that
  //  gtk_widget_set_size_request(applet->box, icon_size_w * g_list_length(applet->icons), icon_size_h);
  tray_resize_container(applet);

  force_redraw(applet);
}

static void
tray_icon_removed (NaTrayManager *manager,
                   GtkWidget      *icon,
                   TrayApplet     *applet)
{
  applet->icons = g_list_remove (applet->icons, icon);
  //  gtk_widget_set_size_request(applet->box, icon_size_w * g_list_length(applet->icons), icon_size_h);
  tray_resize_container(applet);

  gtk_container_remove(GTK_CONTAINER(applet->box), icon);
}

static void
tray_icon_message_sent (NaTrayManager *manager,
                        GtkWidget      *icon,
                        const char     *text,
                        glong           id,
                        glong           timeout,
                        TrayApplet     *applet)
{
  g_warning ("tray_icon_message_sent : %s\n", text);
  cairo_dock_show_temporary_dialog(text, myIcon, myDock, timeout);
}

static void tray_icon_message_cancelled (NaTrayManager *manager,
                                         GtkWidget      *icon,
                                         glong           id,
                                         TrayApplet     *applet)
{
  /* FIXME: Er, cancel the message :-/? */
}

static gboolean tray_clean_up(GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   user_data)
{
  TrayApplet *tray = (TrayApplet *)user_data;
  g_warning("free baby\n");
  g_object_unref (tray->manager);
  g_object_unref (tray);
}

static NaTrayManager *na_tray_man_bah_cest_moche = NULL;

TrayApplet* tray_init (GtkWidget *parent)
{
  TrayApplet *applet = g_new0 (TrayApplet, 1);
  GdkScreen  *screen;
  GtkWidget *widget = GTK_WIDGET(parent);

  //get the real parent
  while (widget->parent)
        widget = widget->parent;
  screen = gtk_widget_get_screen(GTK_WIDGET(widget));

  applet->box = gtk_hbox_new(TRUE, 0);
  gtk_widget_show(applet->box);
  //gtk_widget_set_size_request(applet->box, icon_size_w*10, icon_size_h + 6);
  tray_resize_container(applet);
  applet->icons = NULL;

  if (na_tray_manager_check_running(screen)) {
    g_warning ("There is already another notification area running on this screen\n");
    //we delete the previous task man
    g_object_unref(na_tray_man_bah_cest_moche);
    g_warning ("There is already another notification area running on this screen\n");
  }

  applet->manager = na_tray_manager_new();
  na_tray_man_bah_cest_moche = applet->manager;

  if (!na_tray_manager_manage_screen (applet->manager, screen))
    g_warning ("The notification area could not manage the screen \n");

  g_signal_connect (applet->box, "delete-event",
                    G_CALLBACK (tray_clean_up), applet);

  g_signal_connect (applet->manager, "tray_icon_added",
                    G_CALLBACK (tray_icon_added), applet);
  g_signal_connect (applet->manager, "tray_icon_removed",
                    G_CALLBACK (tray_icon_removed), applet);
  g_signal_connect (applet->manager, "message_sent",
                    G_CALLBACK (tray_icon_message_sent), applet);
  g_signal_connect (applet->manager, "message_cancelled",
                    G_CALLBACK (tray_icon_message_cancelled), applet);

  gtk_widget_set_colormap (applet->box, gdk_screen_get_rgb_colormap (screen));

  applet->widget = gtk_event_box_new ();
  gtk_event_box_set_visible_window(GTK_EVENT_BOX (applet->widget), TRUE);
  gtk_widget_set_colormap(applet->widget, gdk_screen_get_rgb_colormap (screen));
  gtk_container_add (GTK_CONTAINER (applet->widget), applet->box);

  return applet;
}

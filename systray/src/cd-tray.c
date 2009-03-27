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
#include <X11/Xatom.h>
#include <math.h>

#include <cairo-dock.h>
#include "cd-tray.h"
#include "systray-struct.h"


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
/*   GtkRequisition req; */

/*   gtk_widget_size_request(GTK_WIDGET(applet->box), &req); */
/*   if (systray.dialog && &systray.dialog->pWidget) */
/* /\*     gtk_widget_set_size_request(GTK_WINDOW(systray.dialog->pWidget), req.width, req.height); *\/ */
/*   //    gtk_window_set_default_size(GTK_WINDOW(systray.dialog->pWidget), req.width, req.height); */
/*   gtk_window_resize(GTK_WINDOW(systray.dialog->pWidget), req.width, req.height); */
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
  gtk_box_pack_start(GTK_BOX(applet->box), icon, TRUE, TRUE, 0);
  //gtk_widget_set_size_request((applet->box), 52, 28);
  gtk_widget_set_size_request(icon, 24, 24);
  //tray_resize_container(applet);
  force_redraw(applet);
}

static void
tray_icon_removed (NaTrayManager *manager,
                   GtkWidget      *icon,
                   TrayApplet     *applet)
{
  applet->icons = g_list_remove (applet->icons, icon);
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
  cairo_dock_show_temporary_dialog(text, myIcon, myContainer, timeout);
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

 
  static gboolean cd_desklet_on_expose(GtkWidget *pWidget, 
                                      GdkEventExpose *pExpose, 
                                      gpointer pDialog) 
 { 
 cairo_t *pCairoContext = gdk_cairo_create (pWidget->window); 
   if (cairo_status (pCairoContext) != CAIRO_STATUS_SUCCESS, FALSE) { 
     cairo_destroy (pCairoContext); 
     return FALSE; 
   } 
   //erase the background
   cairo_set_source_rgba (pCairoContext, 0., 0., 1., 0.15); 
   cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE); 
   cairo_paint (pCairoContext); 
   cairo_destroy (pCairoContext); 
   return FALSE; 
 } 

static void tray_create_widget(TrayApplet *applet)
{
  applet->manager = na_tray_manager_new();
  if (!na_tray_manager_manage_screen (applet->manager, applet->screen))
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
  g_signal_connect (applet->box, "expose-event",
                     G_CALLBACK (cd_desklet_on_expose), applet->box);


	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (applet->screen);
	gtk_widget_set_colormap (applet->box, pColormap);
	
// 	GdkVisual* pVisual = gdk_rgb_get_visual ();
// 	Visual *vis = GDK_VISUAL_XVISUAL (pVisual);
// 	VisualID visualid = vis->visualid;
// 	
// 	Window Xid = GDK_DRAWABLE_XID (applet->box);
// 	Atom aNetVisualID = XInternAtom (cairo_dock_get_Xdisplay (), "_NET_SYSTEM_TRAY_VISUAL", False);
// 	XChangeProperty (cairo_dock_get_Xdisplay (),
// 		Xid,
// 		aNetVisualID,
// 		XA_VISUALID, 32, PropModeReplace,
// 		(guchar *)&visualid, 1);
	
	gtk_container_add (GTK_CONTAINER (applet->widget), applet->box);
}

static void tray_icon_cb_click_steal(GtkWidget *w, TrayApplet* applet)
{
  gtk_container_remove(GTK_CONTAINER(applet->widget), w);
  tray_create_widget(applet);
}

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
  applet->screen = screen;

  applet->widget = gtk_event_box_new ();
  //gtk_event_box_set_visible_window(GTK_EVENT_BOX (applet->widget), TRUE);  /// utile ?...
  ///gtk_widget_set_colormap(applet->widget, gdk_screen_get_rgb_colormap (screen));
	//GdkColormap* pColormap = gdk_screen_get_rgba_colormap (screen);  /// utile ?...
	//if (!pColormap)
	//	pColormap = gdk_screen_get_rgb_colormap (screen);
	//gtk_widget_set_colormap (applet->widget, pColormap);
  
  
  if (na_tray_manager_check_running(screen)) {
    cd_warning ("There is already another notification area running on this screen");
    GtkWidget *w = gtk_button_new_with_label("TRY to steal systray icons");
    gtk_widget_show(w);
    gtk_container_add (GTK_CONTAINER (applet->widget), w);
    g_signal_connect (w, "clicked",
                      G_CALLBACK (tray_icon_cb_click_steal), applet);
    return applet;
  }

  tray_create_widget(applet);

  return applet;
}

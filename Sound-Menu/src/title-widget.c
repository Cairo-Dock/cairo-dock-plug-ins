/*
Copyright 2010 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include "title-widget.h"
#include "common-defs.h"
#include <gtk/gtk.h>


typedef struct _TitleWidgetPrivate TitleWidgetPrivate;

struct _TitleWidgetPrivate
{
  DbusmenuMenuitem* twin_item;  
};

#define TITLE_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TITLE_WIDGET_TYPE, TitleWidgetPrivate))

/* Prototypes */
static void title_widget_class_init (TitleWidgetClass *klass);
static void title_widget_init       (TitleWidget *self);
static void title_widget_dispose    (GObject *object);
static void title_widget_finalize   (GObject *object);

// keyevent consumers
static gboolean title_widget_button_release_event (GtkWidget *menuitem,
                                                   GdkEventButton *event);

// Dbusmenuitem properties update callback
static void title_widget_property_update(DbusmenuMenuitem* item, gchar* property, 
#if (INDICATOR_OLD_NAMES == 0)
                                       GVariant* value, gpointer userdata);
#else
                                       GValue *value, gpointer userdata);
#endif
static void title_widget_set_twin_item( TitleWidget* self,
                                        DbusmenuMenuitem* twin_item);
static gboolean title_widget_triangle_draw_cb (GtkWidget *widget,
                                               #if (GTK_MAJOR_VERSION < 3)
                                               GdkEventExpose *event,
                                               #else
                                               cairo_t *cr,
                                               #endif
                                               gpointer data);

G_DEFINE_TYPE (TitleWidget, title_widget, GTK_TYPE_IMAGE_MENU_ITEM);

static void
title_widget_class_init (TitleWidgetClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->button_release_event = title_widget_button_release_event;

  g_type_class_add_private (klass, sizeof (TitleWidgetPrivate));

  gobject_class->dispose = title_widget_dispose;
  gobject_class->finalize = title_widget_finalize;
}

static void
title_widget_init (TitleWidget *self)
{
  //g_debug("TitleWidget::title_widget_init");
}

static void 
title_widget_set_icon(TitleWidget *self)
{
  TitleWidgetPrivate *priv = TITLE_WIDGET_GET_PRIVATE(self);

  gint padding = 0;
  gtk_widget_style_get(GTK_WIDGET(self), "horizontal-padding", &padding, NULL);
  gint width, height;
  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
  
  GString* banshee_string = g_string_new ( "banshee" );
  GString* app_panel = g_string_new ( g_utf8_strdown ( dbusmenu_menuitem_property_get(priv->twin_item, DBUSMENU_TITLE_MENUITEM_NAME),
                                                    -1 ));
  GtkWidget * icon = NULL;
  
  // Not ideal but apparently we want the banshee icon to be the greyscale one
  // and any others to be the icon from the desktop file => colour.
  if ( g_string_equal ( banshee_string, app_panel ) == TRUE &&
      gtk_icon_theme_has_icon ( gtk_icon_theme_get_default(), app_panel->str ) ){
    g_string_append ( app_panel, "-panel" );                                                   
    icon = gtk_image_new_from_icon_name ( app_panel->str,
                                          GTK_ICON_SIZE_MENU );    
  }
  else{
    icon = gtk_image_new_from_icon_name ( g_strdup (dbusmenu_menuitem_property_get ( priv->twin_item, DBUSMENU_TITLE_MENUITEM_ICON )),
                                          GTK_ICON_SIZE_MENU );
  }
  g_string_free ( app_panel, FALSE) ;
  g_string_free ( banshee_string, FALSE) ;

  gtk_widget_set_size_request(icon, width
                                    + 5 /* ref triangle is 5x9 pixels */
                                    + 1 /* padding */,
                                    height);
  gtk_misc_set_alignment(GTK_MISC(icon), 0.5 /* right aligned */, 0);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(self), GTK_WIDGET(icon));
  gtk_widget_show(icon);
}

  
static void
title_widget_dispose (GObject *object)
{
  G_OBJECT_CLASS (title_widget_parent_class)->dispose (object);
}

static void
title_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (title_widget_parent_class)->finalize (object);
}

/* Suppress/consume keyevents */
static gboolean
title_widget_button_release_event (GtkWidget *menuitem,
                                   GdkEventButton *event)
{
  //g_debug("TitleWidget::menu_press_event");
  TitleWidgetPrivate * priv = TITLE_WIDGET_GET_PRIVATE(menuitem);
  
#if (INDICATOR_OLD_NAMES == 0)
  GVariant* new_title_event = g_variant_new_boolean(TRUE);
  dbusmenu_menuitem_handle_event (priv->twin_item,
                                  "Title menu event",
                                  new_title_event,
                                  0);
#else
  GValue new_title_event = G_VALUE_INIT;
  g_value_init (&new_title_event, G_TYPE_BOOLEAN);
  g_value_set_boolean (&new_title_event, TRUE);
  dbusmenu_menuitem_handle_event (priv->twin_item,
                                  "Transport state change",
                                  &new_title_event,
                                  0);
#endif
  return FALSE;
}

static void 
title_widget_property_update (DbusmenuMenuitem* item, gchar* property,
#if (INDICATOR_OLD_NAMES == 0)
                              GVariant* value, gpointer userdata)
#else
                              GValue *value, gpointer userdata)
#endif
{
  g_return_if_fail (IS_TITLE_WIDGET (userdata));  
  TitleWidget* mitem = TITLE_WIDGET(userdata);
  if(g_ascii_strcasecmp(DBUSMENU_TITLE_MENUITEM_NAME, property) == 0){
        gtk_menu_item_set_label (GTK_MENU_ITEM(mitem),
#if (INDICATOR_OLD_NAMES == 0)
                                g_variant_get_string(value, NULL));
#else
                                g_value_get_string (value, NULL));
#endif
  }
  else if(g_ascii_strcasecmp(DBUSMENU_TITLE_MENUITEM_ICON, property) == 0){
    title_widget_set_icon (mitem);
  }
}

static void
title_widget_set_twin_item(TitleWidget* self,
                           DbusmenuMenuitem* twin_item)
{
  TitleWidgetPrivate *priv = TITLE_WIDGET_GET_PRIVATE(self);

  priv->twin_item = twin_item;

  g_signal_connect (G_OBJECT (twin_item),
                    "property-changed",
                    G_CALLBACK (title_widget_property_update),
                    self);
  g_signal_connect_after (G_OBJECT (self),
                         "expose_event",
                          G_CALLBACK (title_widget_triangle_draw_cb),
                          twin_item);

  gtk_menu_item_set_label (GTK_MENU_ITEM(self),
                           dbusmenu_menuitem_property_get(priv->twin_item,
                                                          DBUSMENU_TITLE_MENUITEM_NAME));                             
  title_widget_set_icon(self);
}

#if (GTK_MAJOR_VERSION < 3)
static gboolean
title_widget_triangle_draw_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GtkStyle *style;
  cairo_t *cr;
  int x, y, arrow_width, arrow_height;
  
  if (!GTK_IS_WIDGET (widget)) return FALSE;
  if (!DBUSMENU_IS_MENUITEM (data)) return FALSE;

  /* render the triangle indicator only if the application is running */
  if (! dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(data),
                                             DBUSMENU_TITLE_MENUITEM_RUNNING)){   
    return FALSE;
  }
  
  /* get style */
  style = gtk_widget_get_style (widget);

  /* set arrow position / dimensions */
  arrow_width = 5; /* the pixel-based reference triangle is 5x9 */
  arrow_height = 9;
  x = widget->allocation.x;
  y = widget->allocation.y + widget->allocation.height/2.0 - (double)arrow_height/2.0;

  /* initialize cairo drawing area */
  cr = (cairo_t*) gdk_cairo_create (widget->window);

  /* set line width */  
  cairo_set_line_width (cr, 1.0);

  /* cairo drawing code */
  cairo_move_to (cr, x, y);
  cairo_line_to (cr, x, y + arrow_height);
  cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
                            style->fg[gtk_widget_get_state(widget)].green/65535.0,
                            style->fg[gtk_widget_get_state(widget)].blue/65535.0);
  cairo_fill (cr);

  /* remember to destroy cairo context to avoid leaks */
  cairo_destroy (cr);

  return FALSE;
}
#else
static gboolean
title_widget_triangle_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  int x, y, arrow_width, arrow_height;
  
  if (!GTK_IS_WIDGET (widget)) return FALSE;
  if (!DBUSMENU_IS_MENUITEM (data)) return FALSE;

  /* render the triangle indicator only if the application is running */
  if (! dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(data),
                                             DBUSMENU_TITLE_MENUITEM_RUNNING)){   
    return FALSE;
  }
  
  /* get style */
	GtkStyleContext *style = gtk_widget_get_style_context (widget);
	GdkRGBA color;
	gtk_style_context_get_color (style, gtk_widget_get_state(widget), &color);
	
  /* set arrow position / dimensions */
  arrow_width = 5; /* the pixel-based reference triangle is 5x9 */
  arrow_height = 9;
  
  x = 0;
  y = 0 + gtk_widget_get_allocated_height (widget)/2.0 - (double)arrow_height/2.0;

  /* set line width */  
  cairo_set_line_width (cr, 1.0);

  /* cairo drawing code */
  cairo_move_to (cr, x, y);
  cairo_line_to (cr, x, y + arrow_height);
  cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, color.red,
                           color.green,
                            color.blue);
  cairo_fill (cr);

  return FALSE;
}
#endif


 /**
 * transport_new:
 * @returns: a new #TitleWidget.
 **/
GtkWidget* 
title_widget_new(DbusmenuMenuitem *item)
{
  GtkWidget* widget = g_object_new (TITLE_WIDGET_TYPE,
                                          NULL);
  gtk_image_menu_item_set_always_show_image (GTK_IMAGE_MENU_ITEM (widget), TRUE);
  title_widget_set_twin_item((TitleWidget*)widget, item);

  return widget;
}


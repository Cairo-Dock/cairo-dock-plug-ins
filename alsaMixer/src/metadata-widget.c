/*
Copyright 2010 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>
    Mirco Müller <mirco.mueller@canonical.com>

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

#include <cairo-dock.h>
#include <glib/gi18n.h>
#include "metadata-widget.h"
#include "common-defs.h"
#include <gtk/gtk.h>
#include <glib.h>
#include "transport-widget.h"

typedef struct _MetadataWidgetPrivate MetadataWidgetPrivate;

struct _MetadataWidgetPrivate
{
  gboolean   theme_change_occured;
  GtkWidget* meta_data_h_box;
  GtkWidget* meta_data_v_box;
  GtkWidget* album_art;
  GString*   image_path;
  GString*   old_image_path;
  GtkWidget* artist_label;
  GtkWidget* piece_label;
  GtkWidget* container_label;
  GtkWidget* player_label;
  GdkPixbuf* icon_buf;
  DbusmenuMenuitem* twin_item;
  gint		 current_height;
};

#define METADATA_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), METADATA_WIDGET_TYPE, MetadataWidgetPrivate))

/* Prototypes */
static void metadata_widget_class_init    (MetadataWidgetClass *klass);
static void metadata_widget_init          (MetadataWidget *self);
static void metadata_widget_dispose       (GObject *object);
static void metadata_widget_finalize      (GObject *object);
static void metadata_widget_set_style     (GtkWidget* button, GtkStyle* style);
static void metadata_widget_set_twin_item (MetadataWidget* self,
                                           DbusmenuMenuitem* twin_item);
// keyevent consumers
static gboolean metadata_widget_button_release_event (GtkWidget *menuitem, 
                                                    GdkEventButton *event);
// Dbusmenuitem properties update callback
static void metadata_widget_property_update (DbusmenuMenuitem* item,
                                             gchar* property, 
                                             GVariant* value,
                                             gpointer userdata);
static void metadata_widget_style_labels ( MetadataWidget* self,
                                           GtkLabel* label);
static void metadata_widget_selection_received_event_callback( GtkWidget        *widget,
                                                                GtkSelectionData *data,
                                                                guint             time,
                                                                gpointer          user_data);

                                                                

#if GTK_CHECK_VERSION(3, 0, 0)
static void metadata_widget_get_preferred_width (GtkWidget* self,
                                                 gint* minimum_width,
                                                 gint* natural_width);
static void draw_album_art_placeholder_gtk_3 (GtkWidget *metadata, cairo_t* cr);

static void draw_album_border_gtk_3 (GtkWidget *metadata, gboolean selected, cairo_t* cr);

static gboolean metadata_widget_icon_triangle_draw_cb_gtk_3 (GtkWidget *image,
															 cairo_t* cr,
															 gpointer user_data);
static gboolean metadata_image_expose_gtk_3 (GtkWidget *image,
											 cairo_t* cr,
											 gpointer user_data);
#else
static void draw_album_art_placeholder (GtkWidget *metadata);

static void draw_album_border (GtkWidget *metadata, gboolean selected);

static gboolean metadata_widget_icon_triangle_draw_cb (GtkWidget *image,
                                                       GdkEventExpose *event,
                                                       gpointer user_data);
static gboolean metadata_image_expose (GtkWidget *image,
                                       GdkEventExpose *event,
                                       gpointer user_data);
#endif

static void metadata_widget_set_icon (MetadataWidget *self);
static void metadata_widget_handle_resizing (MetadataWidget* self);


G_DEFINE_TYPE (MetadataWidget, metadata_widget, GTK_TYPE_MENU_ITEM);

static void
metadata_widget_class_init (MetadataWidgetClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->button_release_event = metadata_widget_button_release_event;
  #if GTK_CHECK_VERSION(3, 0, 0)
  widget_class->get_preferred_width = metadata_widget_get_preferred_width;  
  #endif
  g_type_class_add_private (klass, sizeof (MetadataWidgetPrivate));

  gobject_class->dispose = metadata_widget_dispose;
  gobject_class->finalize = metadata_widget_finalize;
}

static void
metadata_widget_init (MetadataWidget *self)
{
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(self);
  GtkWidget *hbox;
  GtkWidget *outer_v_box;
  priv->icon_buf = NULL;
    
  outer_v_box = _gtk_vbox_new (0);
  hbox = _gtk_hbox_new(0);
  
  priv->meta_data_h_box = hbox;
  priv->current_height = 1;
  
  // image
  priv->album_art = gtk_image_new();
  priv->image_path = g_string_new("");
  priv->old_image_path = g_string_new("");
  
  #if GTK_CHECK_VERSION(3, 0, 0) 
  g_signal_connect(priv->album_art, "draw", 
                   G_CALLBACK(metadata_image_expose_gtk_3),
                   GTK_WIDGET(self));

  g_signal_connect_after (GTK_WIDGET(self), "draw", 
                          G_CALLBACK(metadata_widget_icon_triangle_draw_cb_gtk_3),
                          GTK_WIDGET(self));
  #else
  g_signal_connect(priv->album_art, "expose-event", 
                   G_CALLBACK(metadata_image_expose),
                   GTK_WIDGET(self));

  g_signal_connect_after (GTK_WIDGET(self), "expose-event", 
                          G_CALLBACK(metadata_widget_icon_triangle_draw_cb),
                          GTK_WIDGET(self));  
  #endif
  gtk_box_pack_start (GTK_BOX (priv->meta_data_h_box),
                      priv->album_art,
                      FALSE,
                      FALSE,
                      1); 
  priv->theme_change_occured = FALSE;
  GtkWidget* vbox = _gtk_vbox_new(0);
  
  // artist
  GtkWidget* artist;
  artist = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(artist), (gfloat)0, (gfloat)0);
  gtk_misc_set_padding (GTK_MISC(artist), (gfloat)10, (gfloat)0);
  gtk_widget_set_size_request (artist, 140, 15);

  gtk_label_set_ellipsize(GTK_LABEL(artist), PANGO_ELLIPSIZE_MIDDLE); 
  metadata_widget_style_labels(self, GTK_LABEL(artist));
  priv->artist_label = artist;
  
  // title
  GtkWidget* piece;
  piece = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(piece), (gfloat)0, (gfloat)0);
  gtk_misc_set_padding (GTK_MISC(piece), (gfloat)10, (gfloat)-5);
  
  gtk_widget_set_size_request (piece, 140, 15);
  gtk_label_set_ellipsize(GTK_LABEL(piece), PANGO_ELLIPSIZE_MIDDLE);
  metadata_widget_style_labels(self, GTK_LABEL(piece));
  priv->piece_label =  piece;

  // container
  GtkWidget* container;
  container = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(container), (gfloat)0, (gfloat)0);
  gtk_misc_set_padding (GTK_MISC(container), (gfloat)10, (gfloat)0);  
  gtk_widget_set_size_request (container, 140, 15);

  gtk_label_set_ellipsize(GTK_LABEL(container), PANGO_ELLIPSIZE_MIDDLE);  
  metadata_widget_style_labels(self, GTK_LABEL(container));
  priv->container_label = container;

  gtk_box_pack_start (GTK_BOX (vbox), priv->piece_label, FALSE, FALSE, 0);  
  gtk_box_pack_start (GTK_BOX (vbox), priv->artist_label, FALSE, FALSE, 0); 
  gtk_box_pack_start (GTK_BOX (vbox), priv->container_label, FALSE, FALSE, 0);  
  
  gtk_box_pack_start (GTK_BOX (priv->meta_data_h_box), vbox, FALSE, FALSE, 0); 

  g_signal_connect(self, "style-set", 
                   G_CALLBACK(metadata_widget_set_style), GTK_WIDGET(self));    
  g_signal_connect (self, "selection-received",
                   G_CALLBACK(metadata_widget_selection_received_event_callback),
                   GTK_WIDGET(self));   

  // player label
  GtkWidget* player_label;
  player_label = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(player_label), (gfloat)0, (gfloat)0);
  gtk_misc_set_padding (GTK_MISC(player_label), (gfloat)1, (gfloat)4);  
  gtk_widget_set_size_request (player_label, 150, 24);
  priv->player_label = player_label;
      
  gtk_box_pack_start (GTK_BOX(outer_v_box), priv->player_label, FALSE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX(outer_v_box), priv->meta_data_h_box, FALSE, FALSE, 0);
    
  gtk_container_add (GTK_CONTAINER (self), outer_v_box);  
  
  gtk_widget_show_all (priv->meta_data_h_box);
  gtk_widget_set_no_show_all (priv->meta_data_h_box, TRUE);
  
  gtk_widget_hide (priv->meta_data_h_box);
}

static void
metadata_widget_dispose (GObject *object)
{
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(METADATA_WIDGET(object)); 

  if (priv->icon_buf != NULL){
    g_object_unref (priv->icon_buf);
    priv->icon_buf = NULL;
  }
  g_string_free (priv->image_path, TRUE);
  priv->image_path = NULL;
  g_string_free (priv->old_image_path, TRUE);
  priv->old_image_path = NULL;
  G_OBJECT_CLASS (metadata_widget_parent_class)->dispose (object);
}

static void
metadata_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (metadata_widget_parent_class)->finalize (object);
}


#if GTK_CHECK_VERSION(3, 0, 0)  
static void
metadata_widget_get_preferred_width (GtkWidget* self,
                                     gint* minimum_width,
                                     gint* natural_width)
{
  *minimum_width = *natural_width =  200;
}

static void
draw_album_border_gtk_3(GtkWidget *metadata, gboolean selected, cairo_t *cr)
{
  gtk_style_context_add_class (gtk_widget_get_style_context (metadata),
                               "menu");
  
  int x, y, width, height;
  x = 0;
  y = 0;
  width = gtk_widget_get_allocated_width (metadata);
  height = gtk_widget_get_allocated_height (metadata);
  
  gint offset = 1;
  
  width = width + (offset * 2);
  height = height + (offset * 2) - 7;
  x = x - offset;
  y = y - offset + 3;

  gint state = selected ? 5 : 0;
  CairoColorRGB bg_normal, fg_normal;
  GtkStyleContext *style = gtk_widget_get_style_context (metadata);
	gtk_style_context_get_background_color (style, 0, (GdkRGBA*)&bg_normal);
	gtk_style_context_get_color (style, state, (GdkRGBA*)&fg_normal);

  CairoColorRGB dark_top_color;
  CairoColorRGB light_bottom_color;
  CairoColorRGB background_color;
  
  _color_shade ( &bg_normal, 0.93, &background_color );
  _color_shade ( &bg_normal, 0.23, &dark_top_color );
  _color_shade ( &fg_normal, 0.55, &light_bottom_color );
  
  cairo_rectangle (cr,
                   x, y,
                   width, height);

  cairo_set_line_width (cr, 1.0);
  
  cairo_clip ( cr );  /// seems redundant here ...

  cairo_move_to (cr, x, y );
  cairo_line_to (cr, x + width,
                y );
  cairo_line_to ( cr, x + width,
                  y + height );
  cairo_line_to ( cr, x, y + height );
  cairo_line_to ( cr, x, y);
  cairo_close_path (cr);

  cairo_set_source_rgba ( cr,
                          background_color.r,
                          background_color.g,
                          background_color.b,
                          1.0 );
  
  cairo_fill ( cr );
  
  cairo_move_to (cr, x, y );
  cairo_line_to (cr, x + width,
                y );

  cairo_close_path (cr);
  cairo_set_source_rgba ( cr,
                          dark_top_color.r,
                          dark_top_color.g,
                          dark_top_color.b,
                          1.0 );
  
  cairo_stroke ( cr );
  
  cairo_move_to ( cr, x + width,
                  y + height );
  cairo_line_to ( cr, x, y + height );

  cairo_close_path (cr);
  cairo_set_source_rgba ( cr,
                         light_bottom_color.r,
                         light_bottom_color.g,
                         light_bottom_color.b,
                         1.0);
  
  cairo_stroke ( cr );
}

static void
draw_album_art_placeholder_gtk_3(GtkWidget *metadata, cairo_t *cr)
{       
  int x, y, width;
  x = 0;
  y = 0;
  width = gtk_widget_get_allocated_width (metadata);

  PangoLayout *layout;
  PangoFontDescription *desc;
  layout = pango_cairo_create_layout(cr);
  PangoContext* pcontext = pango_cairo_create_context(cr); 
  pango_cairo_context_set_resolution (pcontext, 96);

  GString* string = g_string_new("");
  gssize size = -1;
  gunichar code = g_utf8_get_char_validated("\342\231\253", size);  
  g_string_append_unichar (string, code);
  
  pango_layout_set_text(layout, string->str, -1);
  desc = pango_font_description_from_string("Sans Bold 30");
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  CairoColorRGB fg_normal, light_bottom_color;
  GtkStyleContext *style = gtk_widget_get_style_context (metadata);
	gtk_style_context_get_color (style, 0, (GdkRGBA*)&fg_normal);

  _color_shade ( &fg_normal, 0.78, &light_bottom_color );
  
  cairo_set_source_rgba (cr,
                         light_bottom_color.r, 
                         light_bottom_color.g,
                         light_bottom_color.b,
                         1.0);
  
  pango_cairo_update_layout(cr, layout);
  cairo_move_to (cr, x + width/6, y + 3);  
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout); 
  g_object_unref(pcontext);
  g_string_free (string, TRUE);
}

/**
 * We override the expose method to enable primitive drawing of the 
 * empty album art image and rounded rectangles on the album art.
 */
static gboolean
metadata_image_expose_gtk_3 (GtkWidget *metadata,
							 cairo_t* cr,
							 gpointer user_data)
{
  g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  MetadataWidget* widget = METADATA_WIDGET(user_data);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(widget);  

  if ( TRUE == dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(priv->twin_item),
                                                    DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS))
  {
    return FALSE;
  }

  if(priv->image_path->len > 0){
    if(g_string_equal(priv->image_path, priv->old_image_path) == FALSE ||
       priv->theme_change_occured == TRUE){
      priv->theme_change_occured = FALSE;
      GdkPixbuf* pixbuf;
      pixbuf = gdk_pixbuf_new_from_file_at_size(priv->image_path->str, 60, 60, NULL);

      if(GDK_IS_PIXBUF(pixbuf) == FALSE){
        gtk_image_clear ( GTK_IMAGE(priv->album_art));          
        gtk_widget_set_size_request(GTK_WIDGET(priv->album_art), 60, 60);
	  draw_album_border_gtk_3 (metadata, FALSE, cr);
        draw_album_art_placeholder_gtk_3(metadata, cr);
        return FALSE;
      }

      gtk_image_set_from_pixbuf(GTK_IMAGE(priv->album_art), pixbuf);
      gtk_widget_set_size_request(GTK_WIDGET(priv->album_art),
                                  gdk_pixbuf_get_width(pixbuf),
                                  gdk_pixbuf_get_height(pixbuf));

      draw_album_border_gtk_3 (metadata, FALSE, cr);
	g_string_erase (priv->old_image_path, 0, -1);
      g_string_overwrite (priv->old_image_path, 0, priv->image_path->str);
      g_object_unref(pixbuf);
    }
    return FALSE;       
  }
  gtk_image_clear (GTK_IMAGE(priv->album_art));  
  gtk_widget_set_size_request(GTK_WIDGET(priv->album_art), 60, 60);
  draw_album_border_gtk_3 (metadata, FALSE, cr);
  draw_album_art_placeholder_gtk_3(metadata, cr);
  return FALSE;
}

// Draw the triangle if the player is running ...
static gboolean
metadata_widget_icon_triangle_draw_cb_gtk_3 (GtkWidget *widget,
											 cairo_t* cr,
											 gpointer user_data)
{
g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  MetadataWidget* meta = METADATA_WIDGET(user_data);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(meta);  

  int x, y, height, arrow_width, arrow_height;

  arrow_width = 5; 
  arrow_height = 9;
  x = 0;
  y = 0;
  height = gtk_widget_get_allocated_height (widget);
  
  gint offset = (height - gdk_pixbuf_get_height (priv->icon_buf)) / 2;

  // Draw player icon  
  if (priv->icon_buf != NULL){  
    gdk_cairo_set_source_pixbuf (cr,
                                 priv->icon_buf,
                                 x + arrow_width + 1,
                                 y + offset);
    cairo_paint (cr);
  }

  // Draw triangle but only if the player is running.
  if (dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                             DBUSMENU_METADATA_MENUITEM_PLAYER_RUNNING)){
    y += height/2.0 - (double)arrow_height/2.0;
    cairo_set_line_width (cr, 1.0);

    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x, y + arrow_height);
    cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
    cairo_close_path (cr);
    
    CairoColorRGB fg_normal;
    GtkStyleContext *style = gtk_widget_get_style_context (widget);
	gtk_style_context_get_color (style, gtk_widget_get_state(widget), (GdkRGBA*)&fg_normal);
    
    cairo_set_source_rgb (cr, fg_normal.r,
                              fg_normal.g,
                              fg_normal.b);
    cairo_fill (cr);                                             
  }
  
  return FALSE;   
}

// GTK 2 Expose handler
#else

static void
draw_album_border(GtkWidget *metadata, gboolean selected)
{
  cairo_t *cr;  
  cr = gdk_cairo_create (gtk_widget_get_window (metadata));
  #if GTK_CHECK_VERSION(3, 0, 0)
  gtk_style_context_add_class (gtk_widget_get_style_context (metadata),
                               "menu");
  #endif
  
  GtkStyle *style;
  style = gtk_widget_get_style (metadata);
  
  GtkAllocation alloc;
  gtk_widget_get_allocation (metadata, &alloc);
  gint offset = 1;
  
  alloc.width = alloc.width + (offset * 2);
  alloc.height = alloc.height + (offset * 2) - 7;
  alloc.x = alloc.x - offset;
  alloc.y = alloc.y - offset + 3;

  CairoColorRGB bg_normal, fg_normal;

  bg_normal.r = style->bg[0].red/65535.0;
  bg_normal.g = style->bg[0].green/65535.0;
  bg_normal.b = style->bg[0].blue/65535.0;

  gint state = selected ? 5 : 0;
  
  fg_normal.r = style->fg[state].red/65535.0;
  fg_normal.g = style->fg[state].green/65535.0;
  fg_normal.b = style->fg[state].blue/65535.0;

  CairoColorRGB dark_top_color;
  CairoColorRGB light_bottom_color;
  CairoColorRGB background_color;
  
  _color_shade ( &bg_normal, 0.93, &background_color );
  _color_shade ( &bg_normal, 0.23, &dark_top_color );
  _color_shade ( &fg_normal, 0.55, &light_bottom_color );
  
  cairo_rectangle (cr,
                   alloc.x, alloc.y,
                   alloc.width, alloc.height);

  cairo_set_line_width (cr, 1.0);
  
  cairo_clip ( cr );

  cairo_move_to (cr, alloc.x, alloc.y );
  cairo_line_to (cr, alloc.x + alloc.width,
                alloc.y );
  cairo_line_to ( cr, alloc.x + alloc.width,
                  alloc.y + alloc.height );
  cairo_line_to ( cr, alloc.x, alloc.y + alloc.height );
  cairo_line_to ( cr, alloc.x, alloc.y);
  cairo_close_path (cr);

  cairo_set_source_rgba ( cr,
                          background_color.r,
                          background_color.g,
                          background_color.b,
                          1.0 );
  
  cairo_fill ( cr );
  
  cairo_move_to (cr, alloc.x, alloc.y );
  cairo_line_to (cr, alloc.x + alloc.width,
                alloc.y );

  cairo_close_path (cr);
  cairo_set_source_rgba ( cr,
                          dark_top_color.r,
                          dark_top_color.g,
                          dark_top_color.b,
                          1.0 );
  
  cairo_stroke ( cr );
  
  cairo_move_to ( cr, alloc.x + alloc.width,
                  alloc.y + alloc.height );
  cairo_line_to ( cr, alloc.x, alloc.y + alloc.height );

  cairo_close_path (cr);
  cairo_set_source_rgba ( cr,
                         light_bottom_color.r,
                         light_bottom_color.g,
                         light_bottom_color.b,
                         1.0);
  
  cairo_stroke ( cr );
  cairo_destroy (cr);   
}

static void
draw_album_art_placeholder(GtkWidget *metadata)
{       
  cairo_t *cr;  
  cr = gdk_cairo_create (gtk_widget_get_window (metadata));
  GtkStyle *style;
  style = gtk_widget_get_style (metadata);
  
  GtkAllocation alloc;
  gtk_widget_get_allocation (metadata, &alloc);

  PangoLayout *layout;
  PangoFontDescription *desc;
  layout = pango_cairo_create_layout(cr);
  PangoContext* pcontext = pango_cairo_create_context(cr); 
  pango_cairo_context_set_resolution (pcontext, 96);

  GString* string = g_string_new("");
  gssize size = -1;
  gunichar code = g_utf8_get_char_validated("\342\231\253", size);  
  g_string_append_unichar (string, code);
  
  pango_layout_set_text(layout, string->str, -1);
  desc = pango_font_description_from_string("Sans Bold 30");
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  CairoColorRGB fg_normal, light_bottom_color;
  
  fg_normal.r = style->fg[0].red/65535.0;
  fg_normal.g = style->fg[0].green/65535.0;
  fg_normal.b = style->fg[0].blue/65535.0;

  _color_shade ( &fg_normal, 0.78, &light_bottom_color );
  
  cairo_set_source_rgba (cr,
                         light_bottom_color.r, 
                         light_bottom_color.g,
                         light_bottom_color.b,
                         1.0);
  
  pango_cairo_update_layout(cr, layout);
  cairo_move_to (cr, alloc.x + alloc.width/6, alloc.y + 3);  
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout); 
  g_object_unref(pcontext);
  g_string_free (string, TRUE);
  cairo_destroy (cr); 
}

static gboolean
metadata_image_expose (GtkWidget *metadata,
					   GdkEventExpose *event,
					   gpointer user_data)
{
  g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  MetadataWidget* widget = METADATA_WIDGET(user_data);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(widget);  

  if ( TRUE == dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(priv->twin_item),
                                                    DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS))
  {
    return FALSE;
  }
  
  draw_album_border(metadata, FALSE);
  
  if(priv->image_path->len > 0){
    if(g_string_equal(priv->image_path, priv->old_image_path) == FALSE ||
       priv->theme_change_occured == TRUE){
      priv->theme_change_occured = FALSE;
      GdkPixbuf* pixbuf;
      pixbuf = gdk_pixbuf_new_from_file_at_size(priv->image_path->str, 60, 60, NULL);

      if(GDK_IS_PIXBUF(pixbuf) == FALSE){
        gtk_image_clear ( GTK_IMAGE(priv->album_art));          
        gtk_widget_set_size_request(GTK_WIDGET(priv->album_art), 60, 60);
        draw_album_art_placeholder(metadata);
        return FALSE;
      }

      gtk_image_set_from_pixbuf(GTK_IMAGE(priv->album_art), pixbuf);
      gtk_widget_set_size_request(GTK_WIDGET(priv->album_art),
                                  gdk_pixbuf_get_width(pixbuf),
                                  gdk_pixbuf_get_height(pixbuf));

      g_string_erase (priv->old_image_path, 0, -1);
      g_string_overwrite (priv->old_image_path, 0, priv->image_path->str);
      g_object_unref(pixbuf);
    }
    return FALSE;       
  }
  gtk_image_clear (GTK_IMAGE(priv->album_art));  
  gtk_widget_set_size_request(GTK_WIDGET(priv->album_art), 60, 60);
  draw_album_art_placeholder(metadata);
  return FALSE;
}


// Draw the triangle if the player is running ...
static gboolean
metadata_widget_icon_triangle_draw_cb (GtkWidget *widget,
                                       GdkEventExpose *event,
                                       gpointer user_data)
{
  g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  MetadataWidget* meta = METADATA_WIDGET(user_data);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(meta);  

  GtkStyle *style;
  cairo_t *cr;
  int x, y, arrow_width, arrow_height;

  arrow_width = 5; 
  arrow_height = 9;
  
  style = gtk_widget_get_style (widget);

  cr = (cairo_t*) gdk_cairo_create (gtk_widget_get_window (widget));  
  
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);
  x = allocation.x;
  y = allocation.y;

  gint offset = (allocation.height - gdk_pixbuf_get_height (priv->icon_buf)) / 2;

  // Draw player icon  
  if (priv->icon_buf != NULL){  
    gdk_cairo_set_source_pixbuf (cr,
                                 priv->icon_buf,
                                 x + arrow_width + 1,
                                 y + offset);
    cairo_paint (cr);
  }

  // Draw triangle but only if the player is running.
  if (dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                             DBUSMENU_METADATA_MENUITEM_PLAYER_RUNNING)){
    y += allocation.height/2.0 - (double)arrow_height/2.0;
    cairo_set_line_width (cr, 1.0);

    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x, y + arrow_height);
    cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
                              style->fg[gtk_widget_get_state(widget)].green/65535.0,
                              style->fg[gtk_widget_get_state(widget)].blue/65535.0);
    cairo_fill (cr);                                             
  }
  
  cairo_destroy (cr);
  return FALSE;  
}
#endif

static void
metadata_widget_selection_received_event_callback (  GtkWidget        *widget,
                                                     GtkSelectionData *data,
                                                     guint             time,
                                                     gpointer          user_data )

{
#if GTK_CHECK_VERSION(3, 0, 0)
	cairo_t *cr = (cairo_t*) gdk_cairo_create (gtk_widget_get_window (widget));
	/// TODO: need to translate at the widget's position ...
	draw_album_border_gtk_3(widget, TRUE, cr);
	cairo_destroy (cr);
#else
	draw_album_border(widget, TRUE);    
#endif
}

/* Suppress/consume keyevents */
static gboolean
metadata_widget_button_release_event (GtkWidget *menuitem, 
                                      GdkEventButton *event)
{
  g_return_val_if_fail (IS_METADATA_WIDGET (menuitem), FALSE);
  MetadataWidgetPrivate* priv = METADATA_WIDGET_GET_PRIVATE(METADATA_WIDGET(menuitem));
  // For the left raise/launch the player
  if (event->button == 1){
    GVariant* new_title_event = g_variant_new_boolean(TRUE);
    dbusmenu_menuitem_handle_event (priv->twin_item,
                                    "Title menu event",
                                    new_title_event,
                                    0);
  }
  // For the right copy track details to clipboard only if the player is running
  // and there is something there
  else if (event->button == 3){
    gboolean running = dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                                            DBUSMENU_METADATA_MENUITEM_PLAYER_RUNNING);
    gboolean hidden = dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                                           DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS);
    g_return_val_if_fail ( running, FALSE );

    g_return_val_if_fail ( !hidden, FALSE );
    
    GtkClipboard* board = gtk_clipboard_get (GDK_NONE); 
    gchar* contents = g_strdup_printf("artist: %s \ntitle: %s \nalbum: %s",
                                      dbusmenu_menuitem_property_get(priv->twin_item,
                                                          DBUSMENU_METADATA_MENUITEM_ARTIST),
                                      dbusmenu_menuitem_property_get(priv->twin_item,
                                                          DBUSMENU_METADATA_MENUITEM_TITLE),
                                      dbusmenu_menuitem_property_get(priv->twin_item,
                                                          DBUSMENU_METADATA_MENUITEM_ALBUM));
    gtk_clipboard_set_text (board, contents, -1);
    gtk_clipboard_store (board);
    g_free(contents);
  }
  return FALSE;
}

static void 
metadata_widget_property_update(DbusmenuMenuitem* item, gchar* property, 
                                       GVariant* value, gpointer userdata)
{
  g_return_if_fail (IS_METADATA_WIDGET (userdata)); 

  if(g_variant_is_of_type(value, G_VARIANT_TYPE_INT32) == TRUE && 
     g_variant_get_int32(value) == DBUSMENU_PROPERTY_EMPTY){
    GVariant* new_value = g_variant_new_string ("");
    value = new_value;
  }
  
  MetadataWidget* mitem = METADATA_WIDGET(userdata);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(mitem);
  
  if(g_ascii_strcasecmp(DBUSMENU_METADATA_MENUITEM_ARTIST, property) == 0){  
    gtk_label_set_text(GTK_LABEL(priv->artist_label), g_variant_get_string(value, NULL));
    metadata_widget_style_labels(mitem, GTK_LABEL(priv->artist_label));
  }
  else if(g_ascii_strcasecmp(DBUSMENU_METADATA_MENUITEM_TITLE, property) == 0){  
    gtk_label_set_text(GTK_LABEL(priv->piece_label), g_variant_get_string(value, NULL));    
    metadata_widget_style_labels(mitem, GTK_LABEL(priv->piece_label));
  } 
  else if(g_ascii_strcasecmp(DBUSMENU_METADATA_MENUITEM_ALBUM, property) == 0){  
    gtk_label_set_text(GTK_LABEL(priv->container_label), g_variant_get_string(value, NULL));
    metadata_widget_style_labels(mitem, GTK_LABEL(priv->container_label));
  } 
  else if(g_ascii_strcasecmp(DBUSMENU_METADATA_MENUITEM_ARTURL, property) == 0){
    g_string_erase(priv->image_path, 0, -1);
    g_string_overwrite(priv->image_path, 0, g_variant_get_string (value, NULL));
    gtk_widget_queue_draw(GTK_WIDGET(mitem));
  }
  else if (g_ascii_strcasecmp (DBUSMENU_METADATA_MENUITEM_PLAYER_NAME, property) == 0){
    gtk_label_set_label (GTK_LABEL (priv->player_label),
                         g_variant_get_string(value, NULL));
  }
  else if (g_ascii_strcasecmp (DBUSMENU_METADATA_MENUITEM_PLAYER_ICON, property) == 0){
    metadata_widget_set_icon (mitem);
  }
  else if(g_ascii_strcasecmp(DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS, property) == 0){
    metadata_widget_handle_resizing (mitem);
  }
}

static void 
metadata_widget_handle_resizing (MetadataWidget* self)
{
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(self);
    
  if (dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                           DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS) == TRUE){
    gtk_widget_hide (priv->meta_data_h_box);
  }
  else{
    gtk_widget_show (priv->meta_data_h_box);     
  }
  gtk_widget_queue_draw(GTK_WIDGET(self));      
}

static void
metadata_widget_style_labels(MetadataWidget* self, GtkLabel* label)
{
  char* markup;
  markup = g_markup_printf_escaped ("<span size=\"smaller\">%s</span>",
                                    gtk_label_get_text(GTK_LABEL(label)));
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free(markup); 
}

static void
metadata_widget_set_style(GtkWidget* metadata, GtkStyle* style)
{
  g_return_if_fail(IS_METADATA_WIDGET(metadata));
  MetadataWidget* widg = METADATA_WIDGET(metadata);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(widg); 
  priv->theme_change_occured = TRUE;    
  gtk_widget_queue_draw(GTK_WIDGET(metadata));  
}

static void 
metadata_widget_set_icon (MetadataWidget *self)
{
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(self); 

  if (priv->icon_buf != NULL){
    g_object_unref (priv->icon_buf);
    priv->icon_buf = NULL;    
  }
  
  gint padding = 0;
  gtk_widget_style_get(GTK_WIDGET(self), "horizontal-padding", &padding, NULL);
  gint width, height;
  gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
  
  GString* banshee_string = g_string_new ( "banshee" );
  gchar * tmp = g_utf8_strdown (dbusmenu_menuitem_property_get(priv->twin_item, DBUSMENU_METADATA_MENUITEM_PLAYER_NAME), -1);
  GString* app_panel = g_string_new (tmp);
  g_free (tmp);
  
  GdkPixbuf* icon_buf;
  
  // Banshee Special case!  
  // Not ideal but apparently we want the banshee icon to be the greyscale one
  // and any others to be the icon from the desktop file => colour.
  if ( g_string_equal ( banshee_string, app_panel ) == TRUE &&
      gtk_icon_theme_has_icon ( gtk_icon_theme_get_default(), app_panel->str ) ){
    g_string_append ( app_panel, "-panel" );
  }
  else{
    // Otherwise use what is stored in the props
    g_string_erase (app_panel, 0, -1);
    g_string_overwrite (app_panel,
                        0,
                        dbusmenu_menuitem_property_get ( priv->twin_item,
                                                         DBUSMENU_METADATA_MENUITEM_PLAYER_ICON ));  
  }
  icon_buf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default(),
                                        app_panel->str,
                                        (width > height) ? width : height,
                                        GTK_ICON_LOOKUP_GENERIC_FALLBACK, 
                                        NULL );  
  priv->icon_buf = icon_buf;
  g_string_free ( app_panel, TRUE);
  g_string_free ( banshee_string, TRUE);
}

static void
metadata_widget_set_twin_item (MetadataWidget* self,
                               DbusmenuMenuitem* twin_item)
{
  MetadataWidgetPrivate* priv = METADATA_WIDGET_GET_PRIVATE(self);
  priv->twin_item = twin_item;
  g_signal_connect( G_OBJECT(priv->twin_item), "property-changed", 
                            G_CALLBACK(metadata_widget_property_update), self);
  gtk_label_set_text( GTK_LABEL(priv->container_label), 
                      dbusmenu_menuitem_property_get( priv->twin_item,
                                                      DBUSMENU_METADATA_MENUITEM_ALBUM));
  metadata_widget_style_labels( self, GTK_LABEL(priv->container_label));

  gtk_label_set_text( GTK_LABEL(priv->piece_label), 
                      dbusmenu_menuitem_property_get( priv->twin_item,
                                                      DBUSMENU_METADATA_MENUITEM_TITLE));
  metadata_widget_style_labels( self, GTK_LABEL(priv->piece_label));
  gtk_label_set_text( GTK_LABEL(priv->artist_label), 
                      dbusmenu_menuitem_property_get( priv->twin_item,
                                                      DBUSMENU_METADATA_MENUITEM_ARTIST));
  metadata_widget_style_labels( self, GTK_LABEL(priv->artist_label));

  g_string_erase(priv->image_path, 0, -1);
  const gchar *arturl = dbusmenu_menuitem_property_get( priv->twin_item,
                                                        DBUSMENU_METADATA_MENUITEM_ARTURL );
  
  gtk_label_set_label (GTK_LABEL(priv->player_label),
                       dbusmenu_menuitem_property_get(priv->twin_item,
                                                      DBUSMENU_METADATA_MENUITEM_PLAYER_NAME));
  
  metadata_widget_set_icon(self);
  
  if (arturl != NULL){
    g_string_overwrite( priv->image_path,
                        0,
                        arturl);
    // if its a remote image queue a redraw incase the download took too long
    if (g_str_has_prefix (arturl, g_get_user_cache_dir())){
      gtk_widget_queue_draw(GTK_WIDGET(self));                                                          
    }
  }
  metadata_widget_handle_resizing (self);  
}

 /**
 * transport_new:
 * @returns: a new #MetadataWidget.
 **/
GtkWidget* 
metadata_widget_new(DbusmenuMenuitem *item)
{
  GtkWidget* widget =  g_object_new(METADATA_WIDGET_TYPE, NULL);
  metadata_widget_set_twin_item ( METADATA_WIDGET(widget),
                                  item );
  return widget;                  
}


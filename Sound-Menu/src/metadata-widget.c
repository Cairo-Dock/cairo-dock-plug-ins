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
  GtkWidget* hbox;
  GtkWidget* album_art;
  GString*   image_path;
  GString*   old_image_path;
  GtkWidget* artist_label;
  GtkWidget* piece_label;
  GtkWidget* container_label;
  DbusmenuMenuitem* twin_item;      
};

#define METADATA_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), METADATA_WIDGET_TYPE, MetadataWidgetPrivate))

/* Prototypes */
static void metadata_widget_class_init    (MetadataWidgetClass *klass);
static void metadata_widget_init          (MetadataWidget *self);
static void metadata_widget_dispose       (GObject *object);
static void metadata_widget_finalize      (GObject *object);
static gboolean metadata_image_expose     (GtkWidget *image,
#if (GTK_MAJOR_VERSION < 3)
GdkEventExpose *event,
#else
cairo_t *cr,
#endif
gpointer user_data);
static void metadata_widget_set_style     (GtkWidget* button, GtkStyle* style);
static void metadata_widget_set_twin_item (MetadataWidget* self, DbusmenuMenuitem* twin_item);
static void
draw_album_border_ctx(GtkWidget *metadata, gboolean selected, cairo_t *cr);

// keyevent consumers
static gboolean metadata_widget_button_press_event (GtkWidget *menuitem, 
                                                    GdkEventButton *event);
// Dbusmenuitem properties update callback
static void metadata_widget_property_update (DbusmenuMenuitem* item,
                                             gchar* property, 
                                             GVariant* value,
                                             gpointer userdata);
static void metadata_widget_style_labels ( MetadataWidget* self,
                                           GtkLabel* label);
static void
draw_album_art_placeholder(GtkWidget *metadata
#if (GTK_MAJOR_VERSION >= 3)
, cairo_t *cr
#endif
);
static void draw_album_border ( GtkWidget *metadata, gboolean selected);
static void metadata_widget_selection_received_event_callback( GtkWidget        *widget,
                                                                GtkSelectionData *data,
                                                                guint             time,
                                                                gpointer          user_data);
G_DEFINE_TYPE (MetadataWidget, metadata_widget, GTK_TYPE_MENU_ITEM);


static void
metadata_widget_class_init (MetadataWidgetClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->button_press_event = metadata_widget_button_press_event;
  
  g_type_class_add_private (klass, sizeof (MetadataWidgetPrivate));

  gobject_class->dispose = metadata_widget_dispose;
  gobject_class->finalize = metadata_widget_finalize;
}

static void
metadata_widget_init (MetadataWidget *self)
{
  //g_debug("MetadataWidget::metadata_widget_init");

  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(self);
  GtkWidget *hbox;

  hbox = _gtk_hbox_new (0);
  priv->hbox = hbox;

  // image
  priv->album_art = gtk_image_new();
  priv->image_path = g_string_new("");
  priv->old_image_path = g_string_new("");
  
	g_signal_connect(priv->album_art,
		#if (GTK_MAJOR_VERSION < 3)
		"expose-event",
		#else
		"draw",
		#endif
		G_CALLBACK(metadata_image_expose),
		GTK_WIDGET(self));  
  gtk_widget_set_size_request(GTK_WIDGET(priv->album_art), 60, 60); 
  
  gtk_box_pack_start (GTK_BOX (priv->hbox),
                      priv->album_art,
                      FALSE,
                      FALSE,
                      1); 
  priv->theme_change_occured = FALSE;
  GtkWidget* vbox = _gtk_vbox_new (0);
  
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
  
  gtk_box_pack_start (GTK_BOX (priv->hbox), vbox, FALSE, FALSE, 0); 

  g_signal_connect(self, "style-set", 
                   G_CALLBACK(metadata_widget_set_style), GTK_WIDGET(self));    
  g_signal_connect (self, "selection-received",
                   G_CALLBACK(metadata_widget_selection_received_event_callback),
                   GTK_WIDGET(self));   
  gtk_widget_set_size_request(GTK_WIDGET(self), 200, 75); 
  gtk_container_add (GTK_CONTAINER (self), hbox);
}

static void
metadata_widget_dispose (GObject *object)
{
  G_OBJECT_CLASS (metadata_widget_parent_class)->dispose (object);
}

static void
metadata_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (metadata_widget_parent_class)->finalize (object);
}


/**
 * We override the expose method to enable primitive drawing of the 
 * empty album art image and rounded rectangles on the album art.
 */
static gboolean
metadata_image_expose (GtkWidget *metadata,
#if (GTK_MAJOR_VERSION < 3)
GdkEventExpose *event,
#else
cairo_t *cr,
#endif
gpointer user_data)
{
  g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  MetadataWidget* widget = METADATA_WIDGET(user_data);
  MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(widget);   
  #if (GTK_MAJOR_VERSION < 3)
  draw_album_border(metadata, FALSE);
  #else
  draw_album_border_ctx(metadata, FALSE, cr);
  #endif
  if(priv->image_path->len > 0){
    if(g_string_equal(priv->image_path, priv->old_image_path) == FALSE ||
       priv->theme_change_occured == TRUE){
      priv->theme_change_occured = FALSE;         
      GdkPixbuf* orig_pixbuf;
      orig_pixbuf = gdk_pixbuf_new_from_file(priv->image_path->str, NULL);
      //g_debug("metadata_load_new_image -> pixbuf from %s",
      //        priv->image_path->str); 
      if(GDK_IS_PIXBUF(orig_pixbuf) == FALSE){
        //g_debug("problem loading the downloaded image just use the placeholder instead");
        draw_album_art_placeholder(metadata
        #if (GTK_MAJOR_VERSION >= 3)
        ,cr
        #endif
        );
        return TRUE;
      }
      GdkPixbuf* pixbuf;         
      pixbuf = gdk_pixbuf_scale_simple(orig_pixbuf,60, 60, GDK_INTERP_BILINEAR);
      gtk_image_set_from_pixbuf(GTK_IMAGE(priv->album_art), pixbuf);
      g_string_erase(priv->old_image_path, 0, -1);
      g_string_overwrite(priv->old_image_path, 0, priv->image_path->str);

      g_object_unref(pixbuf);       
      g_object_unref(orig_pixbuf);       
    }
    return FALSE;       
  }
	draw_album_art_placeholder(metadata
	#if (GTK_MAJOR_VERSION >= 3)
	,cr
	#endif
	);
	return TRUE;
}

static void
draw_album_border_ctx(GtkWidget *metadata, gboolean selected, cairo_t *cr)
{
	gint offset = 1;
	CairoColorRGB bg_normal, fg_normal;
	gint state = selected ? 5 : 0;
	int x, y, width, height;
	
	#if (GTK_MAJOR_VERSION < 3)
	GtkStyle *style = gtk_widget_get_style (metadata);
	bg_normal.r = style->bg[0].red/65535.0;
	bg_normal.g = style->bg[0].green/65535.0;
	bg_normal.b = style->bg[0].blue/65535.0;
	
	fg_normal.r = style->fg[state].red/65535.0;
	fg_normal.g = style->fg[state].green/65535.0;
	fg_normal.b = style->fg[state].blue/65535.0;
	
	GtkAllocation alloc;
	gtk_widget_get_allocation (metadata, &alloc);
	
	width = alloc.width + (offset * 2);
	height = alloc.height + (offset * 2) - 7;
	x = alloc.x - offset;
	y = alloc.y - offset + 3;
	#else
	GtkStyleContext *style = gtk_widget_get_style_context (metadata);
	gtk_style_context_get_background_color (style, 0, (GdkRGBA*)&bg_normal);
	gtk_style_context_get_color (style, state, (GdkRGBA*)&fg_normal);
	x = - offset;
	y = - offset + 3;
	width = gtk_widget_get_allocated_width (metadata) + (offset * 2);
	height = gtk_widget_get_allocated_height (metadata) + (offset * 2) - 7;
	#endif
	
	CairoColorRGB dark_top_color;
	CairoColorRGB light_bottom_color;
	CairoColorRGB background_color;

	_color_shade ( &bg_normal, 0.93, &background_color );
	_color_shade ( &bg_normal, 0.23, &dark_top_color );
	_color_shade ( &fg_normal, 0.55, &light_bottom_color );
	
	  cairo_rectangle (cr,
                   x, y,
                   width, height);
	cairo_clip ( cr );
  cairo_set_line_width (cr, 1.0);
  

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
draw_album_border(GtkWidget *metadata, gboolean selected)
{
  cairo_t *cr;  
  cr = gdk_cairo_create (gtk_widget_get_window (metadata));
  
  draw_album_border_ctx(metadata, selected, cr);

  cairo_destroy (cr);   
}

static void
draw_album_art_placeholder(GtkWidget *metadata
#if (GTK_MAJOR_VERSION >= 3)
, cairo_t *cr
#endif
)
{   
	int x, y, width, height;
	CairoColorRGB fg_normal, light_bottom_color;
	#if (GTK_MAJOR_VERSION < 3)
	cairo_t *cr = gdk_cairo_create (gtk_widget_get_window (metadata));
	GtkStyle *style = gtk_widget_get_style (metadata);
	GtkAllocation alloc;
	gtk_widget_get_allocation (metadata, &alloc);
	x = alloc.x;
	y = alloc.y;
	width = alloc.width;
	height = alloc.height;
	fg_normal.r = style->fg[0].red/65535.0;
	fg_normal.g = style->fg[0].green/65535.0;
	fg_normal.b = style->fg[0].blue/65535.0;
	#else
	GtkStyleContext *style = gtk_widget_get_style_context (metadata);
	gtk_style_context_get_color (style, 0, (GdkRGBA*)&fg_normal);
	x = y = 0;
	width = gtk_widget_get_allocated_width (metadata);
	height = gtk_widget_get_allocated_height (metadata);
	#endif

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

  _color_shade ( &fg_normal, 0.78, &light_bottom_color );
  

  cairo_set_source_rgba (cr,
                         light_bottom_color.r, 
                         light_bottom_color.g,
                         light_bottom_color.b,
                         1.0);
  
  pango_cairo_update_layout(cr, layout);
  cairo_move_to (cr, x + width/6, y + height/8);  
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout); 
  g_object_unref(pcontext);
  g_string_free (string, TRUE);
  
  #if (GTK_MAJOR_VERSION < 3)
  cairo_destroy (cr); 
  #endif
}

static void
metadata_widget_selection_received_event_callback (  GtkWidget        *widget,
                                                     GtkSelectionData *data,
                                                     guint             time,
                                                     gpointer          user_data )

{
  //g_return_val_if_fail(IS_METADATA_WIDGET(user_data), FALSE);
  //MetadataWidget* widget = METADATA_WIDGET(user_data);
  //MetadataWidgetPrivate * priv = METADATA_WIDGET_GET_PRIVATE(widget);   
  g_debug("metadata_widget_selection_request_event_callback");
  draw_album_border(widget, TRUE);    
}

/* Suppress/consume keyevents */
static gboolean
metadata_widget_button_press_event (GtkWidget *menuitem, 
                                  GdkEventButton *event)
{
  GtkClipboard* board = gtk_clipboard_get (GDK_NONE); 

  MetadataWidgetPrivate* priv = METADATA_WIDGET_GET_PRIVATE(METADATA_WIDGET(menuitem)); 
  
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
  return FALSE;
}

static void 
metadata_widget_property_update(DbusmenuMenuitem* item, gchar* property, 
                                       GVariant* value, gpointer userdata)
{
  g_return_if_fail (IS_METADATA_WIDGET (userdata)); 

  if(g_variant_is_of_type(value, G_VARIANT_TYPE_INT32) == TRUE && 
     g_variant_get_int32(value) == DBUSMENU_PROPERTY_EMPTY){
    //g_debug("Metadata widget: property update - reset");
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
    // if its a remote image queue a redraw incase the download took too long
    if (g_str_has_prefix(g_variant_get_string (value, NULL), g_get_user_cache_dir())){
      //g_debug("the image update is a download so redraw");
      gtk_widget_queue_draw(GTK_WIDGET(mitem));
    }
  }   
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
metadata_widget_set_twin_item(MetadataWidget* self,
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
  if (arturl != NULL){
    g_string_overwrite( priv->image_path,
                        0,
                        arturl);

    // if its a remote image queue a redraw incase the download took too long
    if (g_str_has_prefix (arturl, g_get_user_cache_dir())){
      gtk_widget_queue_draw(GTK_WIDGET(self));                                                          
    }
  }
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


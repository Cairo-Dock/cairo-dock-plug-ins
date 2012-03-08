/*
Copyright 2010 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>
    Mirco Müller <mirco.mueller@canonical.com>
    Andrea Cimitan <andrea.cimitan@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.

Uses code from ctk
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "transport-widget.h"


#define RECT_WIDTH 130.0f
#define Y 7.0f
#define X 70.0f
#define INNER_RADIUS 12.5
#define MIDDLE_RADIUS 13.0f
#define OUTER_RADIUS  14.5f
#define CIRCLE_RADIUS 21.0f
#define PREV_WIDTH  25.0f
#define PREV_HEIGHT 17.0f
#define NEXT_WIDTH  25.0f //PREV_WIDTH
#define NEXT_HEIGHT 17.0f //PREV_HEIGHT
#define TRI_WIDTH  11.0f
#define TRI_HEIGHT 13.0f
#define TRI_OFFSET  6.0f
#define PREV_X 68.0f
#define PREV_Y 13.0f
#define NEXT_X 146.0f
#define NEXT_Y 13.0f //prev_y
#define PAUSE_WIDTH 21.0f
#define PAUSE_HEIGHT 27.0f
#define BAR_WIDTH 4.5f
#define BAR_HEIGHT 24.0f
#define BAR_OFFSET 10.0f
#define PAUSE_X 111.0f
#define PAUSE_Y 7.0f
#define PLAY_WIDTH 28.0f
#define PLAY_HEIGHT 29.0f
#define PLAY_PADDING 5.0f
#define INNER_START_SHADE 0.98
#define INNER_END_SHADE 0.98
#define MIDDLE_START_SHADE 1.0
#define MIDDLE_END_SHADE 1.0
#define OUTER_START_SHADE 0.75
#define OUTER_END_SHADE 1.3
#define SHADOW_BUTTON_SHADE 0.8
#define OUTER_PLAY_START_SHADE 0.7
#define OUTER_PLAY_END_SHADE 1.38
#define BUTTON_START_SHADE 1.1
#define BUTTON_END_SHADE 0.9
#define BUTTON_SHADOW_SHADE 0.8
#define INNER_COMPRESSED_START_SHADE 1.0
#define INNER_COMPRESSED_END_SHADE 1.0

typedef struct _TransportWidgetPrivate TransportWidgetPrivate;

struct _TransportWidgetPrivate
{
  TransportAction     current_command;
  TransportAction     key_event;
  TransportAction     motion_event;
  TransportState      current_state;
  GHashTable*         command_coordinates;
  DbusmenuMenuitem*   twin_item;   
  gboolean            has_focus;
  gint                hold_timer;
  gint                skip_frequency;
};

#if GTK_CHECK_VERSION(3, 0, 0)
static GList *transport_widget_list = NULL;
static GtkStyleContext *spinner_style_context = NULL;
static GtkWidgetPath *spinner_widget_path = NULL;
#endif

// TODO refactor the UI handlers, consolidate functionality between key press /release
// and button press / release.
#define TRANSPORT_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRANSPORT_WIDGET_TYPE, TransportWidgetPrivate))

/* Gobject boiler plate */
static void transport_widget_class_init (TransportWidgetClass *klass);
static void transport_widget_init       (TransportWidget *self);
static void transport_widget_dispose    (GObject *object);
static void transport_widget_finalize   (GObject *object);
G_DEFINE_TYPE (TransportWidget, transport_widget, GTK_TYPE_MENU_ITEM);

/* essentials */
static void transport_widget_set_twin_item ( TransportWidget* self,
                                             DbusmenuMenuitem* twin_item);
#if ! GTK_CHECK_VERSION(3, 0, 0)
static gboolean transport_widget_expose ( GtkWidget *button, GdkEventExpose *event);
#endif
static gboolean draw (GtkWidget* button, cairo_t *cr);

/* UI and dbusmenu callbacks */
static gboolean transport_widget_button_press_event   (GtkWidget      *menuitem,
                                                      GdkEventButton  *event);
static gboolean transport_widget_button_release_event   (GtkWidget      *menuitem,
                                                      GdkEventButton  *event);
static gboolean transport_widget_motion_notify_event    (GtkWidget      *menuitem,
                                                      GdkEventMotion *event);                
static gboolean transport_widget_leave_notify_event    (GtkWidget      *menuitem,
                                                      GdkEventCrossing *event);  
static void transport_widget_property_update ( DbusmenuMenuitem* item,
                                               gchar * property, 
                                               GVariant * value,
                                               gpointer userdata );
static void transport_widget_menu_hidden ( GtkWidget        *menu,
                                           TransportWidget *transport);
static void transport_widget_notify ( GObject *item,
                                      GParamSpec       *pspec,
                                      gpointer          user_data );
static TransportAction transport_widget_determine_button_event ( TransportWidget* button,
                                                                      GdkEventButton* event);
static TransportAction transport_widget_determine_motion_event ( TransportWidget* button,
                                                                      GdkEventMotion* event);
static void transport_widget_react_to_button_release ( TransportWidget* button,
                                                       TransportAction command);
static void transport_widget_toggle_play_pause ( TransportWidget* button,
                                                 TransportState update);
static void transport_widget_select (GtkWidget* menu, gpointer Userdata);
static void transport_widget_deselect (GtkWidget* menu, gpointer Userdata);
static TransportAction transport_widget_collision_detection (gint x, gint y);
static void transport_widget_start_timing (TransportWidget* widget);
static gboolean transport_widget_trigger_seek (gpointer userdata);
static gboolean transport_widget_seek (gpointer userdata);


/// Init functions //////////////////////////////////////////////////////////
static void
transport_widget_class_init (TransportWidgetClass *klass)
{ 
  GObjectClass  *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TransportWidgetPrivate));

  widget_class->button_press_event = transport_widget_button_press_event;
  widget_class->button_release_event = transport_widget_button_release_event;
  widget_class->motion_notify_event = transport_widget_motion_notify_event;
  widget_class->leave_notify_event = transport_widget_leave_notify_event;
#if GTK_CHECK_VERSION(3, 0, 0)
  widget_class->draw = draw;
#else
  widget_class->expose_event = transport_widget_expose;
#endif
  
  gobject_class->dispose = transport_widget_dispose;
  gobject_class->finalize = transport_widget_finalize;
}

static void
transport_widget_init (TransportWidget *self)
{
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(self);
  #if GTK_CHECK_VERSION(3, 0, 0)
  if (transport_widget_list == NULL){
    /* append the object to the static linked list. */
    transport_widget_list = g_list_append (transport_widget_list, self);

    /* create widget path */
    spinner_widget_path = gtk_widget_path_new();

    gtk_widget_path_append_type (spinner_widget_path, GTK_TYPE_MENU);
    gtk_widget_path_append_type (spinner_widget_path, GTK_TYPE_MENU_ITEM);
    gint pos = gtk_widget_path_append_type (spinner_widget_path, GTK_TYPE_SPINNER);
    gtk_widget_path_iter_set_name (spinner_widget_path, pos, "IndicatorSoundSpinner");

    /* create style context and append path */
    spinner_style_context = gtk_style_context_new();

    gtk_style_context_set_path (spinner_style_context, spinner_widget_path);
    gtk_style_context_add_class (spinner_style_context, GTK_STYLE_CLASS_MENU);
    gtk_style_context_add_class (spinner_style_context, GTK_STYLE_CLASS_MENUITEM);
    gtk_style_context_add_class (spinner_style_context, GTK_STYLE_CLASS_SPINNER);
  }
  #endif
  priv->current_command = TRANSPORT_ACTION_NO_ACTION;
  priv->current_state = TRANSPORT_STATE_PAUSED;
  priv->key_event = TRANSPORT_ACTION_NO_ACTION;
  priv->motion_event = TRANSPORT_ACTION_NO_ACTION;
  priv->has_focus = FALSE;
  priv->hold_timer = 0;
  priv->skip_frequency = 0;
  priv->command_coordinates =  g_hash_table_new_full(g_direct_hash,
                                                      g_direct_equal,
                                                      NULL,
                                                      (GDestroyNotify)g_list_free);
  GList* previous_list = NULL;
  previous_list = g_list_insert(previous_list, GINT_TO_POINTER(15), 0);
  previous_list = g_list_insert(previous_list, GINT_TO_POINTER(5), 1);
  previous_list = g_list_insert(previous_list, GINT_TO_POINTER(60), 2);
  previous_list = g_list_insert(previous_list, GINT_TO_POINTER(34), 3);
  g_hash_table_insert(priv->command_coordinates,
                      GINT_TO_POINTER(TRANSPORT_ACTION_PREVIOUS),
                      previous_list);
                     
  GList* play_list = NULL;
  play_list = g_list_insert(play_list, GINT_TO_POINTER(58), 0);
  play_list = g_list_insert(play_list, GINT_TO_POINTER(0), 1);
  play_list = g_list_insert(play_list, GINT_TO_POINTER(50), 2);
  play_list = g_list_insert(play_list, GINT_TO_POINTER(43), 3);

  g_hash_table_insert(priv->command_coordinates,
                      GINT_TO_POINTER(TRANSPORT_ACTION_PLAY_PAUSE),
                      play_list);

  GList* next_list = NULL;
  next_list = g_list_insert(next_list, GINT_TO_POINTER(100), 0);
  next_list = g_list_insert(next_list, GINT_TO_POINTER(5), 1);
  next_list = g_list_insert(next_list, GINT_TO_POINTER(60), 2);
  next_list = g_list_insert(next_list, GINT_TO_POINTER(34), 3);

  g_hash_table_insert(priv->command_coordinates,
                      GINT_TO_POINTER(TRANSPORT_ACTION_NEXT),
                      next_list);
  gtk_widget_set_size_request(GTK_WIDGET(self), 200, 43);
  g_signal_connect (G_OBJECT(self),
                    "notify",
                    G_CALLBACK (transport_widget_notify),
                    NULL);
  g_signal_connect (G_OBJECT(self),
                    "select",
                    G_CALLBACK (transport_widget_select),
                    NULL);
  g_signal_connect (G_OBJECT(self),
                    "deselect",
                    G_CALLBACK (transport_widget_deselect),
                    NULL);  
  gtk_widget_realize ( GTK_WIDGET (self) );

}

static void
transport_widget_dispose (GObject *object)
{
  #if GTK_CHECK_VERSION(3, 0, 0)  
  transport_widget_list = g_list_remove (transport_widget_list, object);

  if (transport_widget_list == NULL){
    if (spinner_widget_path != NULL){
      gtk_widget_path_free (spinner_widget_path);
      spinner_widget_path = NULL;
    }

    if (spinner_style_context != NULL){
      g_object_unref (spinner_style_context);
      spinner_style_context = NULL;
    }
  }
  #endif

  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(object);
  if (priv->command_coordinates != NULL) {
    g_hash_table_destroy (priv->command_coordinates);
    priv->command_coordinates = NULL;
  }

  G_OBJECT_CLASS (transport_widget_parent_class)->dispose (object);
}

static void
transport_widget_finalize (GObject *object)
{
 
  
  G_OBJECT_CLASS (transport_widget_parent_class)->finalize (object);
}

#if ! GTK_CHECK_VERSION(3, 0, 0)
static gboolean
transport_widget_expose (GtkWidget *button, GdkEventExpose *event)
{
  cairo_t *cr;
  cr = gdk_cairo_create (gtk_widget_get_window (button));

  cairo_rectangle (cr,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  cairo_clip(cr);
  draw (button, cr);

  cairo_destroy (cr);
  return FALSE;
}
#endif

gboolean
transport_widget_is_selected ( TransportWidget* widget )
{
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(widget);
  return priv->has_focus;
}

static void
transport_widget_toggle_play_pause(TransportWidget* button,
                                   TransportState update)
{
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(button);
  priv->current_state = update;
  gtk_widget_queue_draw (GTK_WIDGET(button));
}

static void
transport_widget_notify ( GObject *item,
                          GParamSpec       *pspec,
                          gpointer          user_data )
{
  if (g_strcmp0 (pspec->name, "parent")){
    GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (item));
    if (parent){
      g_signal_connect ( parent, "hide",
                         G_CALLBACK (transport_widget_menu_hidden),
                         item );
    }
  }
}

static void
transport_widget_menu_hidden ( GtkWidget        *menu,
                               TransportWidget *transport)
{
  g_return_if_fail(IS_TRANSPORT_WIDGET(transport));
  transport_widget_react_to_button_release(transport, TRANSPORT_ACTION_NO_ACTION);
}

static gboolean
transport_widget_motion_notify_event (GtkWidget *menuitem, 
                                      GdkEventMotion *event)
{
  //g_debug("transport_widget_motion_notify_event()");
  
  g_return_val_if_fail ( IS_TRANSPORT_WIDGET(menuitem), FALSE );
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE ( TRANSPORT_WIDGET(menuitem) );
  TransportAction result = transport_widget_determine_motion_event ( TRANSPORT_WIDGET(menuitem),
                                                                            event);
  priv->motion_event = result;
  gtk_widget_queue_draw (menuitem);
  if (priv->hold_timer != 0){
    g_source_remove (priv->hold_timer);
    priv->hold_timer = 0;
  }
  if(priv->skip_frequency != 0){
    g_source_remove (priv->skip_frequency);
    priv->skip_frequency = 0;
  }
  return TRUE;
}

static gboolean
transport_widget_leave_notify_event (GtkWidget *menuitem, 
                                     GdkEventCrossing *event)
{
  //g_debug("transport_widget_leave_notify_event()");
  
  g_return_val_if_fail ( IS_TRANSPORT_WIDGET(menuitem), FALSE );
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE ( TRANSPORT_WIDGET(menuitem) );
  
  priv->motion_event = TRANSPORT_ACTION_NO_ACTION;
  priv->current_command = TRANSPORT_ACTION_NO_ACTION;
  gtk_widget_queue_draw (GTK_WIDGET(menuitem));
  
  return TRUE;
}

static gboolean
transport_widget_button_press_event (GtkWidget *menuitem, 
                                     GdkEventButton *event)
{
  //g_debug("transport_widget_button_press_event()");
  
  g_return_val_if_fail ( IS_TRANSPORT_WIDGET(menuitem), FALSE );
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE ( TRANSPORT_WIDGET(menuitem) );
  TransportAction result = transport_widget_determine_button_event ( TRANSPORT_WIDGET(menuitem),
                                                                            event);
  if(result != TRANSPORT_ACTION_NO_ACTION){
    priv->current_command = result;
    gtk_widget_queue_draw (GTK_WIDGET(menuitem));
    if (priv->current_command == TRANSPORT_ACTION_PREVIOUS ||
        priv->current_command == TRANSPORT_ACTION_NEXT){
      transport_widget_start_timing (TRANSPORT_WIDGET(menuitem));
    }
  }
  return TRUE;
}
/**
 * TODO rename or merge
 * @param widget
 */
static void
transport_widget_start_timing (TransportWidget* widget)
{
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE (widget);
  if (priv->hold_timer == 0){
    priv->hold_timer = g_timeout_add (800,
                                      transport_widget_trigger_seek,
                                      widget);
  }
}

static gboolean
transport_widget_trigger_seek (gpointer userdata)
{
  g_return_val_if_fail ( IS_TRANSPORT_WIDGET(userdata), FALSE );
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE (TRANSPORT_WIDGET(userdata));
  if (priv->skip_frequency == 0){
    priv->skip_frequency = g_timeout_add (100,
                                          transport_widget_seek,
                                          userdata);
  }
  priv->hold_timer = 0;
  return FALSE;
}

/**
 * This will be called repeatedly until a key/button release is received
 * @param userdata
 * @return 
 */
static gboolean
transport_widget_seek (gpointer userdata)
{
  g_return_val_if_fail ( IS_TRANSPORT_WIDGET(userdata), FALSE );
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE (TRANSPORT_WIDGET(userdata));
  GVariant* new_transport_state;
  if(priv->current_command ==  TRANSPORT_ACTION_NEXT){
    //g_debug ("we should be skipping forward");
    new_transport_state = g_variant_new_int32 ((int)TRANSPORT_ACTION_FORWIND);

    dbusmenu_menuitem_handle_event ( priv->twin_item,
                                     "Transport state change",
                                     new_transport_state,
                                     0 );

  }
  else if(priv->current_command ==  TRANSPORT_ACTION_PREVIOUS){
    //g_debug ("we should be skipping back");
    new_transport_state = g_variant_new_int32 ((int)TRANSPORT_ACTION_REWIND);

    dbusmenu_menuitem_handle_event ( priv->twin_item,
                                     "Transport state change",
                                     new_transport_state,
                                     0 );
  }

  return TRUE;
}

static gboolean
transport_widget_button_release_event (GtkWidget *menuitem, 
                                       GdkEventButton *event)
{
  g_return_val_if_fail(IS_TRANSPORT_WIDGET(menuitem), FALSE);
  TransportWidget* transport = TRANSPORT_WIDGET(menuitem);
  TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport ); 
  TransportAction result = transport_widget_determine_button_event ( transport,
                                                                          event );
  if (result != TRANSPORT_ACTION_NO_ACTION &&
      priv->current_command == result &&
      priv->skip_frequency == 0){
    GVariant* new_transport_state = g_variant_new_int32 ((int)result);
    dbusmenu_menuitem_handle_event ( priv->twin_item,
                                     "Transport state change",
                                     new_transport_state,
                                     0 );
  }
  transport_widget_react_to_button_release ( transport,
                                             result );
  return TRUE;
}

static void 
transport_widget_select (GtkWidget* item, gpointer Userdata)
{
  TransportWidget* transport = TRANSPORT_WIDGET(item);
  TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport ); 
  priv->has_focus = TRUE;
}

static void 
transport_widget_deselect (GtkWidget* item, gpointer Userdata)
{
  TransportWidget* transport = TRANSPORT_WIDGET(item);
  TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport ); 
  priv->has_focus = FALSE;
}

void
transport_widget_react_to_key_press_event ( TransportWidget* transport,
                                            TransportAction transport_event )
{
  if(transport_event != TRANSPORT_ACTION_NO_ACTION){
    TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport );
    priv->current_command = transport_event;
    priv->key_event = transport_event;
    gtk_widget_realize ( GTK_WIDGET(transport) );
    gtk_widget_queue_draw (GTK_WIDGET(transport) );
    if (priv->current_command == TRANSPORT_ACTION_PREVIOUS ||
        priv->current_command == TRANSPORT_ACTION_NEXT){
      transport_widget_start_timing (transport);
    }
  } 
}

void
transport_widget_react_to_key_release_event ( TransportWidget* transport,
                                              TransportAction transport_event )
{
  if(transport_event != TRANSPORT_ACTION_NO_ACTION){
    TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport );     
    GVariant* new_transport_event = g_variant_new_int32((int)transport_event);
    if (priv->skip_frequency == 0){
      dbusmenu_menuitem_handle_event ( priv->twin_item,
                                       "Transport state change",
                                       new_transport_event,
                                       0 );
    }
  }
  transport_widget_react_to_button_release ( transport,
                                             transport_event );  
}

void
transport_widget_focus_update ( TransportWidget* transport, gboolean focus )
{
  TransportWidgetPrivate * priv = TRANSPORT_WIDGET_GET_PRIVATE ( transport );     
  priv->has_focus = focus;  
}

static TransportAction
transport_widget_determine_button_event( TransportWidget* button,
                                         GdkEventButton* event )
{
  return transport_widget_collision_detection (event->x, event->y);
}

static TransportAction
transport_widget_determine_motion_event( TransportWidget* button,
                                         GdkEventMotion* event )
{
  return transport_widget_collision_detection (event->x, event->y);
}

static TransportAction
transport_widget_collision_detection ( gint x,
                                       gint y )
{
  TransportAction event = TRANSPORT_ACTION_NO_ACTION;
  
  if (x > 57 && x < 102
      && y > 12 && y < 40){
    event = TRANSPORT_ACTION_PREVIOUS;
  }
  else if (x > 101 && x < 143
           && y > 5 && y < 47){
    event = TRANSPORT_ACTION_PLAY_PAUSE;
  }
  else if (x > 142 && x < 187
           && y > 12 && y < 40){
    event = TRANSPORT_ACTION_NEXT;
  }   
  return event;
}

static void 
transport_widget_react_to_button_release ( TransportWidget* button,
                                           TransportAction command )
{
  g_return_if_fail(IS_TRANSPORT_WIDGET(button));
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(button);  

  priv->current_command = TRANSPORT_ACTION_NO_ACTION;
  priv->key_event = TRANSPORT_ACTION_NO_ACTION;

  gtk_widget_queue_draw (GTK_WIDGET(button));
  if (priv->hold_timer != 0){
    g_source_remove (priv->hold_timer);
    priv->hold_timer = 0;
  }
  if(priv->skip_frequency != 0){
    g_source_remove (priv->skip_frequency);
    priv->skip_frequency = 0;
  }
}

/// internal helper functions //////////////////////////////////////////////////

static void
draw_gradient (cairo_t* cr,
               double   x,
               double   y,
               double   w,
               double   r,
               double*  rgba_start,
               double*  rgba_end)
{
  cairo_pattern_t* pattern = NULL;

  cairo_move_to (cr, x, y);
  cairo_line_to (cr, x + w - 2.0f * r, y);
  cairo_arc (cr,
       x + w - 2.0f * r,
       y + r,
       r,
       -90.0f * G_PI / 180.0f,
       90.0f * G_PI / 180.0f);
  cairo_line_to (cr, x, y + 2.0f * r);
  cairo_arc (cr,
       x,
       y + r,
       r,
       90.0f * G_PI / 180.0f,
       270.0f * G_PI / 180.0f);
  cairo_close_path (cr);
  
  pattern = cairo_pattern_create_linear (x, y, x, y + 2.0f * r);
  cairo_pattern_add_color_stop_rgba (pattern,
                                     0.0f,
                                     rgba_start[0],
                                     rgba_start[1],
                                     rgba_start[2],
                                     rgba_start[3]);
  cairo_pattern_add_color_stop_rgba (pattern,
                                     1.0f,
                                     rgba_end[0],
                                     rgba_end[1],
                                     rgba_end[2],
                                     rgba_end[3]);
  cairo_set_source (cr, pattern);
  cairo_fill (cr);
  cairo_pattern_destroy (pattern);
}

static void
draw_circle (cairo_t* cr,
       double   x,
       double   y,
       double   r,
       double*  rgba_start,
       double*  rgba_end)
{
  cairo_pattern_t* pattern = NULL;

  cairo_move_to (cr, x, y);
  cairo_arc (cr,
       x + r,
       y + r,
       r,
       0.0f * G_PI / 180.0f,
       360.0f * G_PI / 180.0f);

  pattern = cairo_pattern_create_linear (x, y, x, y + 2.0f * r);
  cairo_pattern_add_color_stop_rgba (pattern,
                                     0.0f,
                                     rgba_start[0],
                                     rgba_start[1],
                                     rgba_start[2],
                                     rgba_start[3]);
  cairo_pattern_add_color_stop_rgba (pattern,
                                     1.0f,
                                     rgba_end[0],
                                     rgba_end[1],
                                     rgba_end[2],
                                     rgba_end[3]);
  cairo_set_source (cr, pattern);
  cairo_fill (cr);
  cairo_pattern_destroy (pattern);
}

static void
_setup (cairo_t**         cr,
  cairo_surface_t** surf,
  gint              width,
  gint              height)
{
  if (!cr || !surf)
    return;

  *surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  *cr = cairo_create (*surf);
  cairo_scale (*cr, 1.0f, 1.0f);
  cairo_set_operator (*cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (*cr);
  cairo_set_operator (*cr, CAIRO_OPERATOR_OVER);
}

static void
_mask_prev (cairo_t* cr,
      double   x,
      double   y,
      double   tri_width,
      double   tri_height,
      double   tri_offset)
{
  if (!cr)
    return;

  cairo_move_to (cr, x,             y + tri_height / 2.0f);
  cairo_line_to (cr, x + tri_width, y);
  cairo_line_to (cr, x + tri_width, y + tri_height);
  x += tri_offset;
  cairo_move_to (cr, x,             y + tri_height / 2.0f);
  cairo_line_to (cr, x + tri_width, y);
  cairo_line_to (cr, x + tri_width, y + tri_height);
  x -= tri_offset;
  cairo_rectangle (cr, x, y, 2.5f, tri_height);
  cairo_close_path (cr);  
}

static void
_mask_next (cairo_t* cr,
      double   x,
      double   y,
      double   tri_width,
      double   tri_height,
      double   tri_offset)
{
  if (!cr)
    return;

  cairo_move_to (cr, x,             y);
  cairo_line_to (cr, x + tri_width, y + tri_height / 2.0f);
  cairo_line_to (cr, x,             y + tri_height);
  x += tri_offset;
  cairo_move_to (cr, x,             y);
  cairo_line_to (cr, x + tri_width, y + tri_height / 2.0f);
  cairo_line_to (cr, x,             y + tri_height);
  x -= tri_offset;
  x += 2.0f * tri_width - tri_offset - 1.0f;
  cairo_rectangle (cr, x, y, 2.5f, tri_height);

  cairo_close_path (cr);  
}

static void
_mask_pause (cairo_t* cr,
       double   x,
       double   y,
       double   bar_width,
       double   bar_height,
       double   bar_offset)
{
  if (!cr)
    return;

  cairo_set_line_width (cr, bar_width);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

  x += bar_width;
  y += bar_width;
  cairo_move_to (cr, x,              y);
  cairo_line_to (cr, x,              y + bar_height);
  cairo_move_to (cr, x + bar_offset, y);
  cairo_line_to (cr, x + bar_offset, y + bar_height);

}

static void
_mask_play (cairo_t* cr,
       double   x,
       double   y,
       double   tri_width,
       double   tri_height)
{
  if (!cr)
    return;

  cairo_move_to (cr, x,             y);
  cairo_line_to (cr, x + tri_width, y + tri_height / 2.0f);
  cairo_line_to (cr, x,             y + tri_height);
  cairo_close_path (cr);  
  
}

static void
_fill (cairo_t* cr,
       double   x_start,
       double   y_start,
       double   x_end,
       double   y_end,
       double*  rgba_start,
       double*  rgba_end,
       gboolean stroke)
{
  cairo_pattern_t* pattern = NULL;

  if (!cr || !rgba_start || !rgba_end)
    return;

  pattern = cairo_pattern_create_linear (x_start, y_start, x_end, y_end);
  cairo_pattern_add_color_stop_rgba (pattern,
             0.0f,
             rgba_start[0],
             rgba_start[1],
             rgba_start[2],
             rgba_start[3]);
  cairo_pattern_add_color_stop_rgba (pattern,
             1.0f,
             rgba_end[0],
             rgba_end[1],
             rgba_end[2],
             rgba_end[3]);
  cairo_set_source (cr, pattern);
  if (stroke)
    cairo_stroke (cr);
  else
    cairo_fill (cr);
  cairo_pattern_destroy (pattern);
}

static void
_finalize (cairo_t*          cr,
     cairo_t**         cr_surf,
     cairo_surface_t** surf,
     double            x,
     double            y)
{
  if (!cr || !cr_surf || !surf)
    return;

  cairo_set_source_surface (cr, *surf, x, y);
  cairo_paint (cr);
  cairo_surface_destroy (*surf);
  cairo_destroy (*cr_surf);
}

static void
_finalize_repaint (cairo_t*          cr,
             cairo_t**         cr_surf,
             cairo_surface_t** surf,
             double            x,
             double            y,
             int               repaints)
{
  if (!cr || !cr_surf || !surf)
    return;

  while (repaints > 0)
  {
    cairo_set_source_surface (cr, *surf, x, y);
    cairo_paint (cr);
    repaints--;
  }

  cairo_surface_destroy (*surf);
  cairo_destroy (*cr_surf);
}

static void
_color_rgb_to_hls (gdouble *r,
                   gdouble *g,
                   gdouble *b)
{
  gdouble min;
  gdouble max;
  gdouble red;
  gdouble green;
  gdouble blue;
  gdouble h = 0;
  gdouble l;
  gdouble s;
  gdouble delta;

  red = *r;
  green = *g;
  blue = *b;

  if (red > green)
  {
    if (red > blue)
      max = red;
    else
      max = blue;

    if (green < blue)
      min = green;
    else
    min = blue;
  }
  else
  {
    if (green > blue)
      max = green;
    else
    max = blue;

    if (red < blue)
      min = red;
    else
      min = blue;
  }
  l = (max+min)/2;
  if (fabs (max-min) < 0.0001)
  {
    h = 0;
    s = 0;
  }
  else
  {
    if (l <= 0.5)
    s = (max-min)/(max+min);
    else
    s = (max-min)/(2-max-min);

    delta = (max -min) != 0 ? (max -min) : 1;
    
    if(delta == 0)
      delta = 1;
    if (red == max)
      h = (green-blue)/delta;
    else if (green == max)
      h = 2+(blue-red)/delta;
    else if (blue == max)
      h = 4+(red-green)/delta;

    h *= 60;
    if (h < 0.0)
      h += 360;
  }

  *r = h;
  *g = l;
  *b = s;
}

static void
_color_hls_to_rgb (gdouble *h,
                   gdouble *l, 
                   gdouble *s)
{
  gdouble hue;
  gdouble lightness;
  gdouble saturation;
  gdouble m1, m2;
  gdouble r, g, b;

  lightness = *l;
  saturation = *s;

  if (lightness <= 0.5)
    m2 = lightness*(1+saturation);
  else
    m2 = lightness+saturation-lightness*saturation;

  m1 = 2*lightness-m2;

  if (saturation == 0)
  {
    *h = lightness;
    *l = lightness;
    *s = lightness;
  }
  else
  {
    hue = *h+120;
    while (hue > 360)
      hue -= 360;
    while (hue < 0)
      hue += 360;

    if (hue < 60)
      r = m1+(m2-m1)*hue/60;
    else if (hue < 180)
      r = m2;
    else if (hue < 240)
      r = m1+(m2-m1)*(240-hue)/60;
    else
      r = m1;

    hue = *h;
    while (hue > 360)
      hue -= 360;
    while (hue < 0)
      hue += 360;

    if (hue < 60)
      g = m1+(m2-m1)*hue/60;
    else if (hue < 180)
      g = m2;
    else if (hue < 240)
      g = m1+(m2-m1)*(240-hue)/60;
    else
      g = m1;

    hue = *h-120;
    while (hue > 360)
      hue -= 360;
    while (hue < 0)
      hue += 360;

    if (hue < 60)
      b = m1+(m2-m1)*hue/60;
    else if (hue < 180)
      b = m2;
    else if (hue < 240)
      b = m1+(m2-m1)*(240-hue)/60;
    else
      b = m1;

    *h = r;
    *l = g;
    *s = b;
  }
}

void
_color_shade (const CairoColorRGB *a, float k, CairoColorRGB *b)
{
  double red;
  double green;
  double blue;

  red   = a->r;
  green = a->g;
  blue  = a->b;

  if (k == 1.0)
  {
    b->r = red;
    b->g = green;
    b->b = blue;
    return;
  }

  _color_rgb_to_hls (&red, &green, &blue);

  green *= k;
  if (green > 1.0)
    green = 1.0;
  else if (green < 0.0)
    green = 0.0;

  blue *= k;
  if (blue > 1.0)
    blue = 1.0;
  else if (blue < 0.0)
    blue = 0.0;

  _color_hls_to_rgb (&red, &green, &blue);

  b->r = red;
  b->g = green;
  b->b = blue;
}

static inline void
_blurinner (guchar* pixel,
      gint*   zR,
      gint*   zG,
      gint*   zB,
      gint*   zA,
      gint    alpha,
      gint    aprec,
      gint    zprec)
{
  gint R;
  gint G;
  gint B;
  guchar A;

  R = *pixel;
  G = *(pixel + 1);
  B = *(pixel + 2);
  A = *(pixel + 3);

  *zR += (alpha * ((R << zprec) - *zR)) >> aprec;
  *zG += (alpha * ((G << zprec) - *zG)) >> aprec;
  *zB += (alpha * ((B << zprec) - *zB)) >> aprec;
  *zA += (alpha * ((A << zprec) - *zA)) >> aprec;

  *pixel       = *zR >> zprec;
  *(pixel + 1) = *zG >> zprec;
  *(pixel + 2) = *zB >> zprec;
  *(pixel + 3) = *zA >> zprec;
}

static inline void
_blurrow (guchar* pixels,
    gint    width,
    gint    height,
    gint    channels,
    gint    line,
    gint    alpha,
    gint    aprec,
    gint    zprec)
{
  gint    zR;
  gint    zG;
  gint    zB;
  gint    zA;
  gint    index;
  guchar* scanline;

  scanline = &(pixels[line * width * channels]);

  zR = *scanline << zprec;
  zG = *(scanline + 1) << zprec;
  zB = *(scanline + 2) << zprec;
  zA = *(scanline + 3) << zprec;

  for (index = 0; index < width; index ++)
    _blurinner (&scanline[index * channels],
          &zR,
          &zG,
          &zB,
          &zA,
          alpha,
          aprec,
          zprec);

  for (index = width - 2; index >= 0; index--)
    _blurinner (&scanline[index * channels],
          &zR,
          &zG,
          &zB,
          &zA,
          alpha,
          aprec,
          zprec);
}

static inline void
_blurcol (guchar* pixels,
    gint    width,
    gint    height,
    gint    channels,
    gint    x,
    gint    alpha,
    gint    aprec,
    gint    zprec)
{
  gint zR;
  gint zG;
  gint zB;
  gint zA;
  gint index;
  guchar* ptr;

  ptr = pixels;

  ptr += x * channels;

  zR = *((guchar*) ptr    ) << zprec;
  zG = *((guchar*) ptr + 1) << zprec;
  zB = *((guchar*) ptr + 2) << zprec;
  zA = *((guchar*) ptr + 3) << zprec;

  for (index = width; index < (height - 1) * width; index += width)
    _blurinner ((guchar*) &ptr[index * channels],
          &zR,
          &zG,
          &zB,
          &zA,
          alpha,
          aprec,
          zprec);

  for (index = (height - 2) * width; index >= 0; index -= width)
    _blurinner ((guchar*) &ptr[index * channels],
          &zR,
          &zG,
          &zB,
          &zA,
          alpha,
          aprec,
          zprec);
}

void
_expblur (guchar* pixels,
    gint    width,
    gint    height,
    gint    channels,
    gint    radius,
    gint    aprec,
    gint    zprec)
{
  gint alpha;
  gint row = 0;
  gint col = 0;

  if (radius < 1)
    return;

  // calculate the alpha such that 90% of 
  // the kernel is within the radius.
  // (Kernel extends to infinity)
  alpha = (gint) ((1 << aprec) * (1.0f - expf (-2.3f / (radius + 1.f))));

  for (; row < height; row++)
    _blurrow (pixels,
        width,
        height,
        channels,
        row,
        alpha,
        aprec,
        zprec);

  for(; col < width; col++)
    _blurcol (pixels,
        width,
        height,
        channels,
        col,
        alpha,
        aprec,
        zprec);

  return;
}

void
_surface_blur (cairo_surface_t* surface,
               guint            radius)
{
  guchar*        pixels;
  guint          width;
  guint          height;
  cairo_format_t format;

  // before we mess with the surface execute any pending drawing
  cairo_surface_flush (surface);

  pixels = cairo_image_surface_get_data (surface);
  width  = cairo_image_surface_get_width (surface);
  height = cairo_image_surface_get_height (surface);
  format = cairo_image_surface_get_format (surface);

  switch (format)
  {
    case CAIRO_FORMAT_ARGB32:
      _expblur (pixels, width, height, 4, radius, 16, 7);
    break;

    case CAIRO_FORMAT_RGB24:
      _expblur (pixels, width, height, 3, radius, 16, 7);
    break;

    case CAIRO_FORMAT_A8:
      _expblur (pixels, width, height, 1, radius, 16, 7);
    break;

    default :
      // do nothing
    break;
  }

  // inform cairo we altered the surfaces contents
  cairo_surface_mark_dirty (surface); 
}

static gboolean
draw (GtkWidget* button, cairo_t *cr)
{
  g_return_val_if_fail(IS_TRANSPORT_WIDGET(button), FALSE);
  g_return_val_if_fail(cr != NULL, FALSE);
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(button);  
  
  //g_debug("transport-widget draw()");
  
  cairo_surface_t*  surf = NULL;
  cairo_t*       cr_surf = NULL;

#if ! GTK_CHECK_VERSION(3, 0, 0)
  GtkAllocation allocation;
  gtk_widget_get_allocation (button, &allocation);
  cairo_translate (cr, allocation.x, allocation.y);  
#endif

  
  
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_style_context_add_class (gtk_widget_get_style_context (button),
                               GTK_STYLE_CLASS_MENU);
#endif
  CairoColorRGB bg_color, fg_color, bg_selected, bg_prelight;
  CairoColorRGB color_middle[2], color_middle_prelight[2], color_outer[2], color_outer_prelight[2],
                color_play_outer[2], color_play_outer_prelight[2],
                color_button[4], color_button_shadow, color_inner[2], color_inner_compressed[2];

	#if (GTK_MAJOR_VERSION < 3)
  GtkStyle *style = gtk_widget_get_style (button);

  bg_color.r = style->bg[0].red/65535.0;
  bg_color.g = style->bg[0].green/65535.0;
  bg_color.b = style->bg[0].blue/65535.0;

  bg_prelight.r = style->bg[GTK_STATE_PRELIGHT].red/65535.0;
  bg_prelight.g = style->bg[GTK_STATE_PRELIGHT].green/65535.0;
  bg_prelight.b = style->bg[GTK_STATE_PRELIGHT].blue/65535.0;

  bg_selected.r = style->bg[GTK_STATE_SELECTED].red/65535.0;
  bg_selected.g = style->bg[GTK_STATE_SELECTED].green/65535.0;
  bg_selected.b = style->bg[GTK_STATE_SELECTED].blue/65535.0;

  fg_color.r = style->fg[0].red/65535.0;
  fg_color.g = style->fg[0].green/65535.0;
  fg_color.b = style->fg[0].blue/65535.0;
	#else
	GtkStyleContext *style = gtk_widget_get_style_context (button);
	gtk_style_context_get_background_color (style, 0, (GdkRGBA*)&bg_color);
	gtk_style_context_get_background_color (style, GTK_STATE_PRELIGHT, (GdkRGBA*)&bg_prelight);
	gtk_style_context_get_background_color (style, GTK_STATE_SELECTED, (GdkRGBA*)&bg_selected);
	gtk_style_context_get_color (style, 0, (GdkRGBA*)&fg_color);
	#endif
	
  _color_shade (&bg_color,    MIDDLE_START_SHADE, &color_middle[0]);
  _color_shade (&bg_color,    MIDDLE_END_SHADE, &color_middle[1]);
  _color_shade (&bg_prelight, MIDDLE_START_SHADE, &color_middle_prelight[0]);
  _color_shade (&bg_prelight, MIDDLE_END_SHADE, &color_middle_prelight[1]);
  _color_shade (&bg_color,    OUTER_START_SHADE, &color_outer[0]);
  _color_shade (&bg_color,    OUTER_END_SHADE, &color_outer[1]);
  _color_shade (&bg_prelight, OUTER_START_SHADE, &color_outer_prelight[0]);
  _color_shade (&bg_prelight, OUTER_END_SHADE, &color_outer_prelight[1]);
  _color_shade (&bg_color,    OUTER_PLAY_START_SHADE, &color_play_outer[0]);
  _color_shade (&bg_color,    OUTER_PLAY_END_SHADE, &color_play_outer[1]);
  _color_shade (&bg_prelight, OUTER_PLAY_START_SHADE, &color_play_outer_prelight[0]);
  _color_shade (&bg_prelight, OUTER_PLAY_END_SHADE, &color_play_outer_prelight[1]);
  _color_shade (&bg_color, INNER_START_SHADE, &color_inner[0]);
  _color_shade (&bg_color, INNER_END_SHADE, &color_inner[1]);
  _color_shade (&fg_color, BUTTON_START_SHADE, &color_button[0]);
  _color_shade (&fg_color, BUTTON_END_SHADE, &color_button[1]);
  _color_shade (&bg_color, BUTTON_SHADOW_SHADE, &color_button[2]);
  _color_shade (&bg_color, SHADOW_BUTTON_SHADE, &color_button_shadow);
  _color_shade (&bg_selected, 1.0, &color_button[3]);
  _color_shade (&bg_color, INNER_COMPRESSED_START_SHADE, &color_inner_compressed[0]);
  _color_shade (&bg_color, INNER_COMPRESSED_END_SHADE, &color_inner_compressed[1]);

  double MIDDLE_END[]   = {color_middle[0].r, color_middle[0].g, color_middle[0].b, 1.0f};
  double MIDDLE_START[] = {color_middle[1].r, color_middle[1].g, color_middle[1].b, 1.0f};
  double MIDDLE_END_PRELIGHT[]   = {color_middle_prelight[0].r, color_middle_prelight[0].g, color_middle_prelight[0].b, 1.0f};
  double MIDDLE_START_PRELIGHT[] = {color_middle_prelight[1].r, color_middle_prelight[1].g, color_middle_prelight[1].b, 1.0f};
  double OUTER_END[]   = {color_outer[0].r, color_outer[0].g, color_outer[0].b, 1.0f};
  double OUTER_START[] = {color_outer[1].r, color_outer[1].g, color_outer[1].b, 1.0f};
  double OUTER_END_PRELIGHT[]   = {color_outer_prelight[0].r, color_outer_prelight[0].g, color_outer_prelight[0].b, 1.0f};
  double OUTER_START_PRELIGHT[] = {color_outer_prelight[1].r, color_outer_prelight[1].g, color_outer_prelight[1].b, 1.0f};
  double SHADOW_BUTTON[] = {color_button_shadow.r, color_button_shadow.g, color_button_shadow.b, 0.3f};
  double OUTER_PLAY_END[] = {color_play_outer[0].r, color_play_outer[0].g, color_play_outer[0].b, 1.0f};
  double OUTER_PLAY_START[] = {color_play_outer[1].r, color_play_outer[1].g, color_play_outer[1].b, 1.0f};
  double OUTER_PLAY_END_PRELIGHT[] = {color_play_outer_prelight[0].r, color_play_outer_prelight[0].g, color_play_outer_prelight[0].b, 1.0f};
  double OUTER_PLAY_START_PRELIGHT[] = {color_play_outer_prelight[1].r, color_play_outer_prelight[1].g, color_play_outer_prelight[1].b, 1.0f};
  double BUTTON_END[] = {color_button[0].r, color_button[0].g, color_button[0].b, 1.0f};
  double BUTTON_START[] = {color_button[1].r, color_button[1].g, color_button[1].b, 1.0f};
  double BUTTON_SHADOW[] = {color_button[2].r, color_button[2].g, color_button[2].b, 0.75f};
  double BUTTON_SHADOW_FOCUS[] = {color_button[3].r, color_button[3].g, color_button[3].b, 1.0f};
  double INNER_COMPRESSED_END[] = {color_inner_compressed[1].r, color_inner_compressed[1].g, color_inner_compressed[1].b, 1.0f};
  double INNER_COMPRESSED_START[] = {color_inner_compressed[0].r, color_inner_compressed[0].g, color_inner_compressed[0].b, 1.0f};  


  draw_gradient (cr,
                 X,
                 Y,
                 RECT_WIDTH,
                 OUTER_RADIUS,
                 OUTER_START,
                 OUTER_END);

  draw_gradient (cr,
                 X,
                 Y + 1,
                 RECT_WIDTH - 2,
                 MIDDLE_RADIUS,
                 MIDDLE_START,
                 MIDDLE_END);

  draw_gradient (cr,
                 X,
                 Y + 2,
                 RECT_WIDTH - 4,
                 MIDDLE_RADIUS,
                 MIDDLE_START,
                 MIDDLE_END);

  //prev/next button
  if(priv->current_command == TRANSPORT_ACTION_PREVIOUS)
  {
    draw_gradient (cr,
                   X,
                   Y,
                   RECT_WIDTH/2,
                   OUTER_RADIUS,
                   OUTER_END,
                   OUTER_START);

    draw_gradient (cr,
                   X,
                   Y + 1,
                   RECT_WIDTH/2,
                   MIDDLE_RADIUS,
                   INNER_COMPRESSED_START,
                   INNER_COMPRESSED_END);

    draw_gradient (cr,
                   X,
                   Y + 2,
                   RECT_WIDTH/2,
                   MIDDLE_RADIUS,
                   INNER_COMPRESSED_START,
                   INNER_COMPRESSED_END);
  }
  else if(priv->current_command == TRANSPORT_ACTION_NEXT)
  {
    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y,
                   RECT_WIDTH/2,
                   OUTER_RADIUS,
                   OUTER_END,
                   OUTER_START);
  
    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y + 1,
                   (RECT_WIDTH - 4.5)/2,
                   MIDDLE_RADIUS,
                   INNER_COMPRESSED_START,
                   INNER_COMPRESSED_END);

    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y + 2,
                   (RECT_WIDTH - 7)/2,
                   MIDDLE_RADIUS,
                   INNER_COMPRESSED_START,
                   INNER_COMPRESSED_END);
  }
  else if (priv->motion_event == TRANSPORT_ACTION_PREVIOUS)
  {
    draw_gradient (cr,
                   X,
                   Y,
                   RECT_WIDTH/2,
                   OUTER_RADIUS,
                   OUTER_START_PRELIGHT,
                   OUTER_END_PRELIGHT);
  
    draw_gradient (cr,
                   X,
                   Y + 1,
                   RECT_WIDTH/2,
                   MIDDLE_RADIUS,
                   MIDDLE_START_PRELIGHT,
                   MIDDLE_END_PRELIGHT);

    draw_gradient (cr,
                   X,
                   Y + 2,
                   RECT_WIDTH/2,
                   MIDDLE_RADIUS,
                   MIDDLE_START_PRELIGHT,
                   MIDDLE_END_PRELIGHT);
  }
  else if (priv->motion_event == TRANSPORT_ACTION_NEXT)
  {
    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y,
                   RECT_WIDTH/2,
                   OUTER_RADIUS,
                   OUTER_START_PRELIGHT,
                   OUTER_END_PRELIGHT);

    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y + 1,
                   (RECT_WIDTH - 4.5)/2,
                   MIDDLE_RADIUS,
                   MIDDLE_START_PRELIGHT,
                   MIDDLE_END_PRELIGHT);

    draw_gradient (cr,
                   RECT_WIDTH / 2 + X,
                   Y + 2,
                   (RECT_WIDTH - 7)/2,
                   MIDDLE_RADIUS,
                   MIDDLE_START_PRELIGHT,
                   MIDDLE_END_PRELIGHT);
  }

  // play/pause shadow
  if(priv->current_command != TRANSPORT_ACTION_PLAY_PAUSE)
  {
    cairo_save (cr);
    cairo_rectangle (cr, X, Y, RECT_WIDTH, MIDDLE_RADIUS*2);
    cairo_clip (cr);

    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f - 1.0f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) - 1.0f,
                 CIRCLE_RADIUS + 1.0f,
                 SHADOW_BUTTON,
                 SHADOW_BUTTON);

    cairo_restore (cr);
  }

  // play/pause button
  if(priv->current_command == TRANSPORT_ACTION_PLAY_PAUSE)
  {
    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) ,
                 CIRCLE_RADIUS,
                 OUTER_PLAY_END,
                 OUTER_PLAY_START);

    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f + 1.25f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) + 1.25f,
                 CIRCLE_RADIUS - 1.25,
                 INNER_COMPRESSED_START,
                 INNER_COMPRESSED_END);
  }
  else if (priv->motion_event == TRANSPORT_ACTION_PLAY_PAUSE)
  {
    /* this subtle offset is to fix alpha borders, should be removed once this draw routine will be refactored */
    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f + 0.1,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) + 0.1,
                 CIRCLE_RADIUS - 0.1,
                 OUTER_PLAY_START_PRELIGHT,
                 OUTER_PLAY_END_PRELIGHT);

    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f + 1.25f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) + 1.25f,
                 CIRCLE_RADIUS - 1.25,
                 MIDDLE_START_PRELIGHT,
                 MIDDLE_END_PRELIGHT);
  }
  else
  {
    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)),
                 CIRCLE_RADIUS,
                 OUTER_PLAY_START,
                 OUTER_PLAY_END);

    draw_circle (cr,
                 X + RECT_WIDTH / 2.0f - 2.0f * OUTER_RADIUS - 5.5f + 1.25f,
                 Y - ((CIRCLE_RADIUS - OUTER_RADIUS)) + 1.25f,
                 CIRCLE_RADIUS - 1.25,
                 MIDDLE_START,
                 MIDDLE_END);
  }

  // draw previous-button drop-shadow
  if (priv->has_focus && priv->key_event == TRANSPORT_ACTION_PREVIOUS)
  {
    _setup (&cr_surf, &surf, PREV_WIDTH+6, PREV_HEIGHT+6);
    _mask_prev (cr_surf,
                (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
                (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
                TRI_WIDTH,
                TRI_HEIGHT,
                TRI_OFFSET);
    _fill (cr_surf,
           (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
           (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (double) TRI_HEIGHT,
           BUTTON_SHADOW_FOCUS,
           BUTTON_SHADOW_FOCUS,
           FALSE);
    _surface_blur (surf, 3);
    _finalize_repaint (cr, &cr_surf, &surf, PREV_X, PREV_Y + 0.5f, 3);
  }
  else
  {
    _setup (&cr_surf, &surf, PREV_WIDTH, PREV_HEIGHT);
    _mask_prev (cr_surf,
                (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
                (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
                TRI_WIDTH,
                TRI_HEIGHT,
                TRI_OFFSET);
    _fill (cr_surf,
           (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
           (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (double) TRI_HEIGHT,
           BUTTON_SHADOW,
           BUTTON_SHADOW,
           FALSE);
    _surface_blur (surf, 1);
    _finalize (cr, &cr_surf, &surf, PREV_X, PREV_Y + 1.0f);
  }

  // draw previous-button
  _setup (&cr_surf, &surf, PREV_WIDTH, PREV_HEIGHT);
  _mask_prev (cr_surf,
              (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
              (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
              TRI_WIDTH,
              TRI_HEIGHT,
              TRI_OFFSET);
  _fill (cr_surf,
       (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
       (PREV_HEIGHT - TRI_HEIGHT) / 2.0f,
       (PREV_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
       (double) TRI_HEIGHT,
       BUTTON_START,
       BUTTON_END,
       FALSE);
  _finalize (cr, &cr_surf, &surf, PREV_X, PREV_Y);

  // draw next-button drop-shadow
  if (priv->has_focus && priv->key_event == TRANSPORT_ACTION_NEXT)
  {
    _setup (&cr_surf, &surf, NEXT_WIDTH+6, NEXT_HEIGHT+6);
    _mask_next (cr_surf,
                (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
                (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
                TRI_WIDTH,
                TRI_HEIGHT,
                TRI_OFFSET);
    _fill (cr_surf,
           (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
           (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (double) TRI_HEIGHT,
           BUTTON_SHADOW_FOCUS,
           BUTTON_SHADOW_FOCUS,
           FALSE);
    _surface_blur (surf, 3); 
    _finalize_repaint (cr, &cr_surf, &surf, NEXT_X, NEXT_Y + 0.5f, 3);
  }
  else
  {
    _setup (&cr_surf, &surf, NEXT_WIDTH, NEXT_HEIGHT);
    _mask_next (cr_surf,
                (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
                (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
                TRI_WIDTH,
                TRI_HEIGHT,
                TRI_OFFSET);
    _fill (cr_surf,
           (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
           (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
           (double) TRI_HEIGHT,
           BUTTON_SHADOW,
           BUTTON_SHADOW,
           FALSE);
    _surface_blur (surf, 1); 
    _finalize (cr, &cr_surf, &surf, NEXT_X, NEXT_Y + 1.0f);
  }

  // draw next-button
  _setup (&cr_surf, &surf, NEXT_WIDTH, NEXT_HEIGHT);
  _mask_next (cr_surf,
              (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
              (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
              TRI_WIDTH,
              TRI_HEIGHT,
              TRI_OFFSET);
  _fill (cr_surf,
         (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
         (NEXT_HEIGHT - TRI_HEIGHT) / 2.0f,
         (NEXT_WIDTH - (2.0f * TRI_WIDTH - TRI_OFFSET)) / 2.0f,
         (double) TRI_HEIGHT,
         BUTTON_START,
         BUTTON_END,
         FALSE);
  _finalize (cr, &cr_surf, &surf, NEXT_X, NEXT_Y);

  // draw pause-button drop-shadow
  if(priv->current_state == TRANSPORT_STATE_PLAYING)
  {
    if (priv->has_focus && (priv->key_event == TRANSPORT_ACTION_NO_ACTION ||
        priv->key_event == TRANSPORT_ACTION_PLAY_PAUSE))
    {
      _setup (&cr_surf, &surf, PAUSE_WIDTH+6, PAUSE_HEIGHT+6);
      _mask_pause (cr_surf,
                   (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
                   (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
                   BAR_WIDTH,
                   BAR_HEIGHT - 2.0f * BAR_WIDTH,
                   BAR_OFFSET);
      _fill (cr_surf,
             (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
             (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
             (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
             (double) BAR_HEIGHT,
             BUTTON_SHADOW_FOCUS,
             BUTTON_SHADOW_FOCUS,
             TRUE);
      _surface_blur (surf, 3);
      _finalize_repaint (cr, &cr_surf, &surf, PAUSE_X, PAUSE_Y + 0.5f, 3);
    }
    else
    {
      _setup (&cr_surf, &surf, PAUSE_WIDTH, PAUSE_HEIGHT);
      _mask_pause (cr_surf,
                   (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
                   (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
                   BAR_WIDTH,
                   BAR_HEIGHT - 2.0f * BAR_WIDTH,
                   BAR_OFFSET);
      _fill (cr_surf,
             (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
             (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
             (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
             (double) BAR_HEIGHT,
             BUTTON_SHADOW,
             BUTTON_SHADOW,
             TRUE);
      _surface_blur (surf, 1);
      _finalize (cr, &cr_surf, &surf, PAUSE_X, PAUSE_Y + 1.0f);
    }

    // draw pause-button
    _setup (&cr_surf, &surf, PAUSE_WIDTH, PAUSE_HEIGHT);
    _mask_pause (cr_surf,
                 (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
                 (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
                 BAR_WIDTH,
                 BAR_HEIGHT - 2.0f * BAR_WIDTH,
                 BAR_OFFSET);
    _fill (cr_surf,
           (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
           (PAUSE_HEIGHT - BAR_HEIGHT) / 2.0f,
           (PAUSE_WIDTH - (2.0f * BAR_WIDTH + BAR_OFFSET)) / 2.0f,
           (double) BAR_HEIGHT,
           BUTTON_START,
           BUTTON_END,
           TRUE);
    _finalize (cr, &cr_surf, &surf, PAUSE_X, PAUSE_Y);
  }
  else if(priv->current_state == TRANSPORT_STATE_PAUSED)
  {
    if (priv->has_focus && (priv->key_event == TRANSPORT_ACTION_NO_ACTION ||
        priv->key_event == TRANSPORT_ACTION_PLAY_PAUSE))
    {
      _setup (&cr_surf, &surf, PLAY_WIDTH+6, PLAY_HEIGHT+6);
      _mask_play (cr_surf, 
                  PLAY_PADDING,
                  PLAY_PADDING,
                  PLAY_WIDTH - (2*PLAY_PADDING),
                  PLAY_HEIGHT - (2*PLAY_PADDING));
      _fill (cr_surf,
             PLAY_PADDING,
             PLAY_PADDING,
             PLAY_WIDTH - (2*PLAY_PADDING),
             PLAY_HEIGHT - (2*PLAY_PADDING),
             BUTTON_SHADOW_FOCUS,
             BUTTON_SHADOW_FOCUS,
             FALSE);
      _surface_blur (surf, 3);
      _finalize_repaint (cr, &cr_surf, &surf, PAUSE_X-0.5f, PAUSE_Y + 0.5f, 3);
    }
    else
    {
      _setup (&cr_surf, &surf, PLAY_WIDTH, PLAY_HEIGHT);
      _mask_play (cr_surf, 
                  PLAY_PADDING,
                  PLAY_PADDING,
                  PLAY_WIDTH - (2*PLAY_PADDING),
                  PLAY_HEIGHT - (2*PLAY_PADDING));
      _fill (cr_surf,
             PLAY_PADDING,
             PLAY_PADDING,
             PLAY_WIDTH - (2*PLAY_PADDING),
             PLAY_HEIGHT - (2*PLAY_PADDING),
             BUTTON_SHADOW,
             BUTTON_SHADOW,
             FALSE);
      _surface_blur (surf, 1);
      _finalize (cr, &cr_surf, &surf, PAUSE_X-0.75f, PAUSE_Y + 1.0f);
    }

    // draw play-button
    _setup (&cr_surf, &surf, PLAY_WIDTH, PLAY_HEIGHT);
    cairo_set_line_width (cr, 10.5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    _mask_play (cr_surf,
                PLAY_PADDING,
                PLAY_PADDING,
                PLAY_WIDTH - (2*PLAY_PADDING),
                PLAY_HEIGHT - (2*PLAY_PADDING));
    _fill (cr_surf,
           PLAY_PADDING,
           PLAY_PADDING,
           PLAY_WIDTH - (2*PLAY_PADDING),
           PLAY_HEIGHT - (2*PLAY_PADDING),
           BUTTON_START,
           BUTTON_END,
           FALSE);
    _finalize (cr, &cr_surf, &surf, PAUSE_X-0.5f, PAUSE_Y);
  }
  #if GTK_CHECK_VERSION(3, 0, 0)
  else if(priv->current_state == TRANSPORT_STATE_LAUNCHING)
  {
    // the spinner is not aligned, why? because the play button has odd width/height numbers
    gtk_render_activity (spinner_style_context, cr, 106, 6, 30, 30);
  }
  #endif
  return FALSE;
}

static void 
transport_widget_set_twin_item(TransportWidget* self,
                           DbusmenuMenuitem* twin_item)
{
    TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(self);
    priv->twin_item = twin_item;
    g_signal_connect(G_OBJECT(priv->twin_item), "property-changed", 
                              G_CALLBACK(transport_widget_property_update), self);
    gint initial_state = dbusmenu_menuitem_property_get_int (twin_item,
                                                             DBUSMENU_TRANSPORT_MENUITEM_PLAY_STATE );
    //g_debug("TRANSPORT WIDGET - INITIAL UPDATE = %i", initial_state);
    transport_widget_toggle_play_pause (self,
                                       (TransportState)initial_state);
}

/**
* transport_widget_update_state()
* Callback for updates from the other side of dbus
**/ 
static void 
transport_widget_property_update(DbusmenuMenuitem* item, gchar* property, 
                                 GVariant* value, gpointer userdata)
{
  //g_debug("transport_widget_update_state - with property  %s", property);
  TransportWidget* bar = (TransportWidget*)userdata;
  g_return_if_fail(IS_TRANSPORT_WIDGET(bar));
  TransportWidgetPrivate* priv = TRANSPORT_WIDGET_GET_PRIVATE(bar);

  if(g_ascii_strcasecmp(DBUSMENU_TRANSPORT_MENUITEM_PLAY_STATE, property) == 0)
  {
    TransportState new_state = (TransportState)g_variant_get_int32(value);
    //g_debug("transport_widget_update_state - with value  %i", new_state);
    if (new_state == TRANSPORT_STATE_LAUNCHING){
      #if GTK_CHECK_VERSION(3, 0, 0)
      gtk_style_context_notify_state_change (spinner_style_context, 
                                             gtk_widget_get_window ( GTK_WIDGET(userdata)),
                                             NULL,
                                             GTK_STATE_FLAG_ACTIVE,
                                             TRUE);
      gtk_style_context_set_state (spinner_style_context, GTK_STATE_FLAG_ACTIVE);
      #endif

      priv->current_state = TRANSPORT_STATE_LAUNCHING;
      g_debug("TransportWidget::toggle play state : %i", priv->current_state);
    }
    else{
      transport_widget_toggle_play_pause(bar, new_state);
    }
  }
}


/**
* transport_widget_new:
* @returns: a new #TransportWidget.
**/
GtkWidget* 
transport_widget_new ( DbusmenuMenuitem *item )
{ 
  GtkWidget* widget = g_object_new(TRANSPORT_WIDGET_TYPE, NULL);
  gtk_widget_set_app_paintable (widget, TRUE);  
  transport_widget_set_twin_item((TransportWidget*)widget, item);  
  return widget;
}


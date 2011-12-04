
/*
Copyright 2011 Canonical Ltd.

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
#include <math.h>
#include <glib.h>
#include "voip-input-widget.h"
#include "common-defs.h"
#include <libido/idoscalemenuitem.h>

typedef struct _VoipInputWidgetPrivate VoipInputWidgetPrivate;

struct _VoipInputWidgetPrivate
{
  DbusmenuMenuitem* twin_item;
  GtkWidget* ido_voip_input_slider;
  gboolean grabbed;
};

#define VOIP_INPUT_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), VOIP_INPUT_WIDGET_TYPE, VoipInputWidgetPrivate))

/* Prototypes */
static void voip_input_widget_class_init (VoipInputWidgetClass *klass);
static void voip_input_widget_init       (VoipInputWidget *self);
static void voip_input_widget_dispose    (GObject *object);
static void voip_input_widget_finalize   (GObject *object);
static void voip_input_widget_set_twin_item( VoipInputWidget* self,
                                        DbusmenuMenuitem* twin_item);
static void voip_input_widget_property_update( DbusmenuMenuitem* item, gchar* property,
                                           GVariant* value, gpointer userdata );

static gboolean voip_input_widget_change_value_cb (GtkRange     *range,
                                              GtkScrollType scroll,
                                              gdouble       value,
                                              gpointer      user_data);
static gboolean voip_input_widget_value_changed_cb(GtkRange *range, gpointer user_data);
static void voip_input_widget_slider_grabbed(GtkWidget *widget, gpointer user_data);
static void voip_input_widget_slider_released(GtkWidget *widget, gpointer user_data);
static void voip_input_widget_parent_changed (GtkWidget *widget, gpointer user_data);

G_DEFINE_TYPE (VoipInputWidget, voip_input_widget, G_TYPE_OBJECT);


static void
voip_input_widget_class_init (VoipInputWidgetClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VoipInputWidgetPrivate));

  gobject_class->dispose = voip_input_widget_dispose;
  gobject_class->finalize = voip_input_widget_finalize;
}

static void
voip_input_widget_init (VoipInputWidget *self)
{
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(self);

  priv->ido_voip_input_slider = ido_scale_menu_item_new_with_range ("VOLUME", IDO_RANGE_STYLE_DEFAULT,  0, 0, 100, 1);
  g_object_ref (priv->ido_voip_input_slider);
  ido_scale_menu_item_set_primary_label (IDO_SCALE_MENU_ITEM(priv->ido_voip_input_slider), "VOIP");

  ido_scale_menu_item_set_style (IDO_SCALE_MENU_ITEM (priv->ido_voip_input_slider), IDO_SCALE_MENU_ITEM_STYLE_IMAGE);
  g_object_set(priv->ido_voip_input_slider, "reverse-scroll-events", TRUE, NULL);

  g_signal_connect (priv->ido_voip_input_slider,
                    "notify::parent", G_CALLBACK (voip_input_widget_parent_changed),
                    NULL);

  GtkWidget* voip_input_widget = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)priv->ido_voip_input_slider);

  g_signal_connect(voip_input_widget, "change-value", G_CALLBACK(voip_input_widget_change_value_cb), self);
  g_signal_connect(voip_input_widget, "value-changed", G_CALLBACK(voip_input_widget_value_changed_cb), self);
  g_signal_connect(priv->ido_voip_input_slider, "slider-grabbed", G_CALLBACK(voip_input_widget_slider_grabbed), self);
  g_signal_connect(priv->ido_voip_input_slider, "slider-released", G_CALLBACK(voip_input_widget_slider_released), self);

  GtkWidget* primary_image = ido_scale_menu_item_get_primary_image((IdoScaleMenuItem*)priv->ido_voip_input_slider);
  GIcon * primary_gicon = g_themed_icon_new_with_default_fallbacks("audio-input-microphone-low-zero-panel");
  gtk_image_set_from_gicon(GTK_IMAGE(primary_image), primary_gicon, GTK_ICON_SIZE_MENU);
  g_object_unref(primary_gicon);

  GtkWidget* secondary_image = ido_scale_menu_item_get_secondary_image((IdoScaleMenuItem*)priv->ido_voip_input_slider);
  GIcon * secondary_gicon = g_themed_icon_new_with_default_fallbacks("audio-input-microphone-high-panel");
  gtk_image_set_from_gicon(GTK_IMAGE(secondary_image), secondary_gicon, GTK_ICON_SIZE_MENU);
  g_object_unref(secondary_gicon);

  GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE (voip_input_widget));
  gtk_adjustment_set_step_increment(adj, 4);
}

static void
voip_input_widget_dispose (GObject *object)
{
  G_OBJECT_CLASS (voip_input_widget_parent_class)->dispose (object);
}

static void
voip_input_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (voip_input_widget_parent_class)->finalize (object);
}

static void
voip_input_widget_property_update (DbusmenuMenuitem* item, gchar* property,
                                   GVariant* value, gpointer userdata)
{
  g_return_if_fail (IS_VOIP_INPUT_WIDGET (userdata));
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(userdata);
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(mitem);
  if(g_ascii_strcasecmp(DBUSMENU_VOIP_INPUT_MENUITEM_LEVEL, property) == 0){
    g_return_if_fail (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE));
    if (priv->grabbed == FALSE){
      GtkWidget *slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)priv->ido_voip_input_slider);
      GtkRange *range = (GtkRange*)slider;
      gdouble update = g_variant_get_double (value);
      //g_debug("volume-widget - update level with value %f", update);
      gtk_range_set_value(range, update);
    }
  }
  if(g_ascii_strcasecmp(DBUSMENU_VOIP_INPUT_MENUITEM_MUTE, property) == 0){
    if(priv->grabbed == FALSE){
      g_return_if_fail (g_variant_is_of_type (value, G_VARIANT_TYPE_INT32));
      GtkWidget *slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)priv->ido_voip_input_slider);
      GtkRange *range = (GtkRange*)slider;
      gint update = g_variant_get_int32 (value);
      gdouble level;
      if (update == 1){
        level = 0;
      }
      else{
        GVariant* variant = dbusmenu_menuitem_property_get_variant (priv->twin_item,
                                                                    DBUSMENU_VOIP_INPUT_MENUITEM_LEVEL);
        g_return_if_fail (g_variant_is_of_type (variant, G_VARIANT_TYPE_DOUBLE));
        level = g_variant_get_double (variant);
      }
      gtk_range_set_value(range, level);

      g_debug ("voip-item-widget - update mute with value %i", update);
    }
  }
}

static void
voip_input_widget_set_twin_item (VoipInputWidget* self,
                                 DbusmenuMenuitem* twin_item)
{
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(self);
  priv->twin_item = twin_item;
  g_object_ref(priv->twin_item);
  g_signal_connect(G_OBJECT(twin_item), "property-changed",
                   G_CALLBACK(voip_input_widget_property_update), self);
  gdouble initial_level = g_variant_get_double (dbusmenu_menuitem_property_get_variant(twin_item,
                                                DBUSMENU_VOIP_INPUT_MENUITEM_LEVEL));
  //g_debug("voip_input_widget_set_twin_item initial level = %f", initial_level);
  GtkWidget *slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)priv->ido_voip_input_slider);
  GtkRange *range = (GtkRange*)slider;
  gtk_range_set_value(range, initial_level);

  gint mute = g_variant_get_int32 (dbusmenu_menuitem_property_get_variant (priv->twin_item,
                                                                           DBUSMENU_VOIP_INPUT_MENUITEM_MUTE));
  if (mute == 1){
    gtk_range_set_value (range, 0.0);
  }
}

static gboolean
voip_input_widget_change_value_cb (GtkRange     *range,
                                   GtkScrollType scroll,
                                   gdouble       new_value,
                                   gpointer      user_data)
{
  g_return_val_if_fail (IS_VOIP_INPUT_WIDGET (user_data), FALSE);
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(user_data);
  voip_input_widget_update(mitem, new_value);
  return FALSE;
}


/**
 * We only want this callback to catch mouse icon press events which set the
 * slider to 0 or 100. Ignore all other events including the new Mute behaviour
 * (slider to go to 0 on mute without setting the level to 0 and return to
 * previous level on unmute)
 **/
static gboolean
voip_input_widget_value_changed_cb(GtkRange *range, gpointer user_data)
{
  g_return_val_if_fail (IS_VOIP_INPUT_WIDGET (user_data), FALSE);
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(user_data);
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(mitem);
  GtkWidget *slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)priv->ido_voip_input_slider);
  gdouble current_value =  CLAMP(gtk_range_get_value(GTK_RANGE(slider)), 0, 100);

  gint mute = g_variant_get_int32 (dbusmenu_menuitem_property_get_variant (priv->twin_item,
                                                                           DBUSMENU_VOIP_INPUT_MENUITEM_MUTE));
  if ((current_value == 0 && mute != 1) || current_value == 100 ){
    voip_input_widget_update(mitem, current_value);
  }
  return FALSE;
}

void
voip_input_widget_update(VoipInputWidget* self, gdouble update)
{
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(self);
  gdouble clamped = CLAMP(update, 0, 100);
  GVariant* new_volume = g_variant_new_double(clamped);
  dbusmenu_menuitem_handle_event (priv->twin_item, "update", new_volume, 0);
}

GtkWidget*
voip_input_widget_get_ido_slider(VoipInputWidget* self)
{
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(self);
  return priv->ido_voip_input_slider;
}

static void
voip_input_widget_parent_changed (GtkWidget *widget,
                              gpointer   user_data)
{
  gtk_widget_set_size_request (widget, 200, -1);
  //g_debug("voip_input_widget_parent_changed");
}

static void
voip_input_widget_slider_grabbed(GtkWidget *widget, gpointer user_data)
{
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(user_data);
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(mitem);
  priv->grabbed = TRUE;
}

static void
voip_input_widget_slider_released(GtkWidget *widget, gpointer user_data)
{
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(user_data);
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(mitem);
  priv->grabbed = FALSE;
}

void
voip_input_widget_tidy_up (GtkWidget *widget)
{
  VoipInputWidget* mitem = VOIP_INPUT_WIDGET(widget);
  VoipInputWidgetPrivate * priv = VOIP_INPUT_WIDGET_GET_PRIVATE(mitem);
  gtk_widget_destroy (priv->ido_voip_input_slider);
}

/**
 * voip_input_widget_new:
 * @returns: a new #VoipInputWidget.
 **/
GtkWidget*
voip_input_widget_new(DbusmenuMenuitem *item)
{
  GtkWidget* widget = g_object_new(VOIP_INPUT_WIDGET_TYPE, NULL);
  voip_input_widget_set_twin_item((VoipInputWidget*)widget, item);
  return widget;
}



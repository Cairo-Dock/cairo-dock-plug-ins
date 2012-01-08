/*
Copyright 2011 Canonical Ltd.

Authors:
    Marco Trevisan (Treviño) <mail@3v1n0.net>

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
#include <glib.h>
#include "mute-widget.h"
#include "common-defs.h"

typedef struct _MuteWidgetPrivate MuteWidgetPrivate;

struct _MuteWidgetPrivate
{
  DbusmenuMenuitem *item;  
  GtkMenuItem *gitem;
};

#define MUTE_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MUTE_WIDGET_TYPE, MuteWidgetPrivate))

/* Prototypes */
static void mute_widget_class_init (MuteWidgetClass *klass);
static void mute_widget_init       (MuteWidget *self);
static void mute_widget_dispose    (GObject *object);
static void mute_widget_finalize   (GObject *object);

G_DEFINE_TYPE (MuteWidget, mute_widget, G_TYPE_OBJECT);

static void
mute_widget_class_init (MuteWidgetClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = mute_widget_dispose;
  gobject_class->finalize = mute_widget_finalize;
  g_type_class_add_private (klass, sizeof (MuteWidgetPrivate));
}

static void
mute_widget_init (MuteWidget *self)
{
  MuteWidgetPrivate *priv = MUTE_WIDGET_GET_PRIVATE(self);
  priv->item = NULL;
  priv->gitem = GTK_MENU_ITEM(gtk_menu_item_new ());
}

static void
mute_widget_dispose (GObject *object)
{
  G_OBJECT_CLASS (mute_widget_parent_class)->dispose (object);
}

static void
mute_widget_finalize (GObject *object)
{
  MuteWidget *self = MUTE_WIDGET (object);
  MuteWidgetPrivate *priv = MUTE_WIDGET_GET_PRIVATE(self);

  g_object_unref (priv->item);
  g_object_unref (G_OBJECT (priv->gitem));
  G_OBJECT_CLASS (mute_widget_parent_class)->finalize (object);
}

GtkMenuItem *
mute_widget_get_menu_item(MuteWidget *self)
{
  MuteWidgetPrivate *priv = MUTE_WIDGET_GET_PRIVATE(self);
  return priv->gitem;
}

MuteStatus
mute_widget_get_status (MuteWidget *self)
{
  g_return_val_if_fail(self, MUTE_STATUS_UNAVAILABLE);
  MuteStatus status = MUTE_STATUS_UNAVAILABLE;
  MuteWidgetPrivate *priv = MUTE_WIDGET_GET_PRIVATE(self);

  GVariant *vstatus = dbusmenu_menuitem_property_get_variant(priv->item,
                                                  DBUSMENU_MUTE_MENUITEM_VALUE);

  if (g_variant_is_of_type (vstatus, G_VARIANT_TYPE_BOOLEAN))
  {
    if (g_variant_get_boolean (vstatus))
      status = MUTE_STATUS_MUTED;
    else
      status = MUTE_STATUS_UNMUTED;
  }

  return status;
}

void mute_widget_toggle (MuteWidget *self)
{
  g_return_if_fail (self);
  MuteWidgetPrivate *priv = MUTE_WIDGET_GET_PRIVATE(self);
  gtk_menu_item_activate (priv->gitem);
}

/**
 * mute_widget_new:
 * @returns: a new #MuteWidget.
 **/
MuteWidget *
mute_widget_new (DbusmenuMenuitem *item)
{
  MuteWidget* widget = g_object_new(MUTE_WIDGET_TYPE, NULL);
  MuteWidgetPrivate* priv = MUTE_WIDGET_GET_PRIVATE(widget);
  priv->item = g_object_ref(item);

  GVariant *label = dbusmenu_menuitem_property_get_variant(priv->item,
                                                  DBUSMENU_MENUITEM_PROP_LABEL);

  if (g_variant_is_of_type(label, G_VARIANT_TYPE_STRING))
    gtk_menu_item_set_label(priv->gitem, g_variant_get_string(label, NULL));

  return widget;
}

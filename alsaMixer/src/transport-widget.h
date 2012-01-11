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
#ifndef __TRANSPORT_WIDGET_H__
#define __TRANSPORT_WIDGET_H__

#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(3, 0, 0)
#include <libdbusmenu-gtk3/menuitem.h>
#else
#include <libdbusmenu-gtk/menuitem.h>
#endif

#include "common-defs.h"

G_BEGIN_DECLS

#define TRANSPORT_WIDGET_TYPE            (transport_widget_get_type ())
#define TRANSPORT_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRANSPORT_WIDGET_TYPE, TransportWidget))
#define TRANSPORT_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TRANSPORT_WIDGET_TYPE, TransportWidgetClass))
#define IS_TRANSPORT_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRANSPORT_WIDGET_TYPE))
#define IS_TRANSPORT_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TRANSPORT_WIDGET_TYPE))
#define TRANSPORT_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRANSPORT_WIDGET_TYPE, TransportWidgetClass))

typedef struct _TransportWidget      TransportWidget;
typedef struct _TransportWidgetClass TransportWidgetClass;


struct _TransportWidgetClass {
    GtkMenuItemClass parent_class;
};

struct _TransportWidget {
    GtkMenuItem parent;
};

typedef struct
{
  double r;
  double g;
  double b;
  /// make it a GdkRGBA
  double a;
} CairoColorRGB;


void _color_shade (const CairoColorRGB *a, float k, CairoColorRGB *b);
GType transport_widget_get_type (void);
GtkWidget* transport_widget_new (DbusmenuMenuitem *item);
void transport_widget_react_to_key_press_event (TransportWidget* widget,
                                                TransportAction transport_event);
void transport_widget_react_to_key_release_event (TransportWidget* widget,
                                                  TransportAction transport_event);
gboolean transport_widget_is_selected (TransportWidget* widget);
G_END_DECLS

#endif


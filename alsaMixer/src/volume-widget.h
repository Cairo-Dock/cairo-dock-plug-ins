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
#ifndef __VOLUME_WIDGET_H__
#define __VOLUME_WIDGET_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

G_BEGIN_DECLS

#define VOLUME_WIDGET_TYPE            (volume_widget_get_type ())
#define VOLUME_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), VOLUME_WIDGET_TYPE, VolumeWidget))
#define VOLUME_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), VOLUME_WIDGET_TYPE, VolumeWidgetClass))
#define IS_VOLUME_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VOLUME_WIDGET_TYPE))
#define IS_VOLUME_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), VOLUME_WIDGET_TYPE))
#define VOLUME_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), VOLUME_WIDGET_TYPE, VolumeWidgetClass))

typedef struct _VolumeWidget  VolumeWidget;
typedef struct _VolumeWidgetClass VolumeWidgetClass;

struct _VolumeWidgetClass {
  GObjectClass parent_class;
};

struct _VolumeWidget {
  GObject parent;
};

GType volume_widget_get_type (void) G_GNUC_CONST;
GtkWidget* volume_widget_new(DbusmenuMenuitem *item/**, IndicatorObject* io*/);
GtkWidget* volume_widget_get_ido_slider(VolumeWidget* self);
void volume_widget_update(VolumeWidget* self, gdouble update, const gchar* label);
void volume_widget_tidy_up (GtkWidget *widget);
gdouble volume_widget_get_current_volume ( GtkWidget *widget );

G_END_DECLS

#endif


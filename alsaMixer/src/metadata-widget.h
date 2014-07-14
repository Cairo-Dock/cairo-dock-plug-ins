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
#ifndef __METADATA_WIDGET_H__
#define __METADATA_WIDGET_H__

#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

G_BEGIN_DECLS

#define METADATA_WIDGET_TYPE            (metadata_widget_get_type ())
#define METADATA_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), METADATA_WIDGET_TYPE, MetadataWidget))
#define METADATA_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), METADATA_WIDGET_TYPE, MetadataWidgetClass))
#define IS_METADATA_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), METADATA_WIDGET_TYPE))
#define IS_METADATA_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), METADATA_WIDGET_TYPE))
#define METADATA_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), METADATA_WIDGET_TYPE, MetadataWidgetClass))

typedef struct _MetadataWidget      MetadataWidget;
typedef struct _MetadataWidgetClass MetadataWidgetClass;

struct _MetadataWidgetClass {
  GtkMenuItemClass parent_class;
};

struct _MetadataWidget {
  GtkMenuItem parent;
};

GType metadata_widget_get_type (void);
GtkWidget* metadata_widget_new(DbusmenuMenuitem *twin_item);

G_END_DECLS

#endif


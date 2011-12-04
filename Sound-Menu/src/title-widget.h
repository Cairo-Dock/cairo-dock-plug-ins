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
#ifndef __TITLE_WIDGET_H__
#define __TITLE_WIDGET_H__

#include <gtk/gtk.h>
#include <libdbusmenu-glib/menuitem.h>


G_BEGIN_DECLS

#define TITLE_WIDGET_TYPE            (title_widget_get_type ())
#define TITLE_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TITLE_WIDGET_TYPE, TitleWidget))
#define TITLE_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TITLE_WIDGET_TYPE, TitleWidgetClass))
#define IS_TITLE_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TITLE_WIDGET_TYPE))
#define IS_TITLE_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TITLE_WIDGET_TYPE))
#define TITLE_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TITLE_WIDGET_TYPE, TitleWidgetClass))

typedef struct _TitleWidget      TitleWidget;
typedef struct _TitleWidgetClass TitleWidgetClass;

struct _TitleWidgetClass {
  GtkImageMenuItemClass parent_class;
};

struct _TitleWidget {
  GtkImageMenuItem parent;
};

GType title_widget_get_type (void);
GtkWidget* title_widget_new(DbusmenuMenuitem *twin_item);

G_END_DECLS

#endif


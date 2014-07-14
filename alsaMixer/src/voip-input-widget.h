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
#ifndef __VOIP_INPUT_WIDGET_H__
#define __VOIP_INPUT_WIDGET_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

G_BEGIN_DECLS

#define VOIP_INPUT_WIDGET_TYPE            (voip_input_widget_get_type ())
#define VOIP_INPUT_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), VOIP_INPUT_WIDGET_TYPE, VoipInputWidget))
#define VOIP_INPUT_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), VOIP_INPUT_WIDGET_TYPE, VoipInputWidgetClass))
#define IS_VOIP_INPUT_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VOIP_INPUT_WIDGET_TYPE))
#define IS_VOIP_INPUT_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), VOIP_INPUT_WIDGET_TYPE))
#define VOIP_INPUT_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), VOIP_INPUT_WIDGET_TYPE, VoipInputWidgetClass))

typedef struct _VoipInputWidget  VoipInputWidget;
typedef struct _VoipInputWidgetClass VoipInputWidgetClass;

struct _VoipInputWidgetClass {
  GObjectClass parent_class;
};

struct _VoipInputWidget {
  GObject parent;
};

GType voip_input_widget_get_type (void) G_GNUC_CONST;
GtkWidget* voip_input_widget_new(DbusmenuMenuitem* twin_item);
GtkWidget* voip_input_widget_get_ido_slider(VoipInputWidget* self);
void voip_input_widget_update(VoipInputWidget* self, gdouble update);
void voip_input_widget_tidy_up (GtkWidget *widget);

G_END_DECLS

#endif


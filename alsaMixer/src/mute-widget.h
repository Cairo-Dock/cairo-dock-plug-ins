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
#ifndef __MUTE_WIDGET_H__
#define __MUTE_WIDGET_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>
#include <libindicator/indicator-object.h>

G_BEGIN_DECLS

#define MUTE_WIDGET_TYPE            (mute_widget_get_type ())
#define MUTE_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUTE_WIDGET_TYPE, MuteWidget))
#define MUTE_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MUTE_WIDGET_TYPE, MuteWidgetClass))
#define IS_MUTE_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUTE_WIDGET_TYPE))
#define IS_MUTE_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MUTE_WIDGET_TYPE))
#define MUTE_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUTE_WIDGET_TYPE, MuteWidgetClass))

typedef struct _MuteWidget  MuteWidget;
typedef struct _MuteWidgetClass MuteWidgetClass;

struct _MuteWidgetClass {
  GObjectClass parent_class;
};

struct _MuteWidget {
  GObject parent;
};

typedef enum {
  MUTE_STATUS_UNAVAILABLE,
  MUTE_STATUS_MUTED,
  MUTE_STATUS_UNMUTED
} MuteStatus;

GType mute_widget_get_type (void) G_GNUC_CONST;
MuteWidget* mute_widget_new (DbusmenuMenuitem *item);
MuteStatus mute_widget_get_status (MuteWidget *self);
void mute_widget_toggle (MuteWidget *self);
GtkMenuItem *mute_widget_get_menu_item (MuteWidget *self);

G_END_DECLS

#endif


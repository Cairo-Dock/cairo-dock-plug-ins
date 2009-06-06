/*
** systray-struct.h
** Login : <ctaf@ctaf-laptop>
** Started on  Tue Feb 12 14:35:00 2008 GESTES Cedric
** $Id$
**
** Author(s):
**  - GESTES Cedric <ctaf42@gmail.com>
**
** Copyright (C) 2008 GESTES Cedric
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef   	SYSTRAY_STRUCT_H_
# define   	SYSTRAY_STRUCT_H_

#include <cairo-dock.h>
#include "na-tray-manager.h"

struct _AppletConfig {
  gchar *shortcut;
  gint iIconPacking;
  gint iIconSize;
} ;



typedef struct {
  NaTrayManager  *manager;
  GtkWidget      *box;
  GtkWidget      *widget;
  GdkScreen      *screen;

  GList          *icons;
  guint          idle_redraw_id;
} TrayApplet;

struct _AppletData {
  CairoDialog *dialog;
  TrayApplet *tray;
} ;


#endif 	    /* !SYSTRAY_STRUCT_H_ */

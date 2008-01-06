/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Copyright (C) 2007 GESTES Cedric
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

#include <cairo-dock.h>

#include "terminal-config.h"
#include "terminal-init.h"

t_terminal term = {
  35555.,
  FALSE,
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  NULL,
  NULL
};

static void set_color(GdkColor *color, double src[3]) {
  color->red = (guint16)(src[0] * 65535.);
  color->green = (guint16)(src[1] * 65535.);
  color->blue = (guint16)(src[2]* 65535.);
}

CD_APPLET_CONFIG_BEGIN ("terminal", "gnome-terminal")
{
  //0 means completely transparent and 65535 opaque
  term.transparency = (guint16)cairo_dock_get_double_key_value (pKeyFile, "GUI", "terminal transparency", &bFlushConfFileNeeded, 55000., NULL, NULL);

  term.always_on_top = cairo_dock_get_boolean_key_value (pKeyFile, "GUI", "always on top", &bFlushConfFileNeeded, FALSE, NULL, NULL);

  double color_back[3] = {1., 1., 1.};
  cairo_dock_get_double_list_key_value (pKeyFile, "GUI", "background color", &bFlushConfFileNeeded, color_back, 3, color_back, NULL, NULL);
  set_color(&term.backcolor, color_back);

  double color_fore[3] = {0., 0., 0.};
  cairo_dock_get_double_list_key_value (pKeyFile, "GUI", "foreground color", &bFlushConfFileNeeded, color_fore, 3, color_fore, NULL, NULL);
  set_color(&term.forecolor, color_fore);
}
CD_APPLET_CONFIG_END

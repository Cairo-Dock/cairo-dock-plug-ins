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

extern t_terminal term;

static void set_color(GdkColor *color, double src[3]) {
  color->red = (guint16)(src[0] * 65535.);
  color->green = (guint16)(src[1] * 65535.);
  color->blue = (guint16)(src[2]* 65535.);
}

CD_APPLET_CONFIG_BEGIN ("terminal", "gnome-terminal")
	//0 means completely transparent and 65535 opaque
	term.transparency = (guint16) (CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("GUI", "terminal transparency", .84) * 65535);  // 55000

	term.always_on_top = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("GUI", "always on top", FALSE);

	double color_back[3] = {1., 1., 1.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("GUI", "background color", color_back, color_back);
	set_color(&term.backcolor, color_back);

	double color_fore[3] = {0., 0., 0.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("GUI", "foreground color", color_fore, color_fore);
	set_color(&term.forecolor, color_fore);

	term.iNbRows = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("GUI", "nb lines", 25);
	term.iNbColumns = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("GUI", "nb columns", 80);
CD_APPLET_CONFIG_END

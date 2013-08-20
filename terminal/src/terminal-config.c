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

#include <string.h>
#include <cairo-dock.h>

#include "terminal-struct.h"
#include "terminal-init.h"
#include "terminal-widget.h"
#include "terminal-config.h"

static void set_color(GdkColor *color, double src[3]) {
  color->red = (guint16)(src[0] * 65535.);
  color->green = (guint16)(src[1] * 65535.);
  color->blue = (guint16)(src[2]* 65535.);
}

CD_APPLET_GET_CONFIG_BEGIN
	// Appearance
	//0 means completely transparent and 65535 opaque
	myConfig.transparency = (guint16) (CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "terminal transparency", .84) * 65535);  // 55000

	double color_back[3] = {1., 1., 1.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("Configuration", "background color", color_back, color_back);
	set_color(&myConfig.backcolor, color_back);

	double color_fore[3] = {0., 0., 0.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("Configuration", "foreground color", color_fore, color_fore);
	set_color(&myConfig.forecolor, color_fore);

	myConfig.bCustomFont = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "custom font", FALSE);
	if (myConfig.bCustomFont)
		myConfig.cCustomFont = CD_CONFIG_GET_STRING ("Configuration", "font");

	// Behaviour
	myConfig.bScrollOutput = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "scroll output", FALSE);
	myConfig.bScrollKeystroke = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "scroll key", TRUE);
	myConfig.bScrollback = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "scrollback", TRUE);
	if (myConfig.bScrollback)
		myConfig.iScrollback = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "scrollback length", 512);

	// Terminal
	myConfig.shortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Ctrl>F1");
	myConfig.iNbRows = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb lines", 25);
	myConfig.iNbColumns = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb columns", 80);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.shortcut);
	myConfig.shortcut = NULL;
	g_free (myConfig.cCustomFont);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.dialog)
	{
		gldi_object_unref (GLDI_OBJECT(myData.dialog));  // detruit aussi le widget interactif.
		myData.dialog = NULL;
	}
	else if (myData.tab)
	{
		gldi_desklet_steal_interactive_widget (myDesklet);
		g_object_unref (G_OBJECT (myData.tab));
	}
	myData.tab = NULL;
CD_APPLET_RESET_DATA_END

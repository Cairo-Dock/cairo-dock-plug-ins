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
#include <string.h>

#include "systray-config.h"
#include "systray-init.h"
#include "systray-interface.h"
#include "systray-struct.h"


CD_APPLET_GET_CONFIG_BEGIN
	CD_CONFIG_RENAME_GROUP ("GUI", "Configuration");
	myConfig.shortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Ctrl>F2");
	myConfig.iIconPacking = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "icon packing", 0);
	//myConfig.iIconSize = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "icon size", 24);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.shortcut);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.dialog)
	{
		cairo_dock_dialog_unreference (myData.dialog);  // detruit aussi le widget interactif.
		myData.dialog = NULL;
	}
	else if (myDesklet && myData.tray)
	{
		cairo_dock_steal_interactive_widget_from_desklet (myDesklet);
		gtk_widget_destroy (GTK_WIDGET (myData.tray));
		myData.tray = NULL;
	}
CD_APPLET_RESET_DATA_END

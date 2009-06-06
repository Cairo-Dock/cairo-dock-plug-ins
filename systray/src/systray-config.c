/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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
#include "systray-menu-functions.h"
#include "systray-struct.h"


CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.shortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("GUI", "shortkey", "<Ctrl>F2");
	myConfig.iIconPacking = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("GUI", "icon packing", 0);
	myConfig.iIconSize = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("GUI", "icon size", 24);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
  if (myConfig.shortcut)
    cd_keybinder_unbind(myConfig.shortcut, (CDBindkeyHandler)systray_on_keybinding_pull);
  g_free (myConfig.shortcut);
  myConfig.shortcut = NULL;
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	g_print ("CD_APPLET_RESET_DATA_BEGIN\n");
	if (myData.dialog)
	{
		cairo_dock_dialog_unreference (myData.dialog);  // detruit aussi le widget interactif.
		myData.dialog = NULL;
	}
	else if (myData.tray)
	{
		gtk_widget_destroy (myData.tray->widget);
		myData.tray->widget = NULL;
	}
	
	/// detruire la invisible window et la liste des icones ...
	g_object_unref (myData.tray->manager);
	g_print ("bye bye\n");
CD_APPLET_RESET_DATA_END

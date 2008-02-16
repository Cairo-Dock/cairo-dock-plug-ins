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

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN ("systray", "gnome-panel-notification-area")
{
  myConfig.shortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("GUI", "shortkey", "<Ctrl>F2");
}
CD_APPLET_CONFIG_END


void reset_config(void)
{
  if (myConfig.shortcut)
    cd_keybinder_unbind(myConfig.shortcut, (CDBindkeyHandler)systray_on_keybinding_pull);
  g_free (myConfig.shortcut);
  myConfig.shortcut = NULL;
  memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data(void)
{
  if (myData.dialog)
	{
		cairo_dock_dialog_unreference (myData.dialog);  // detruit aussi le widget interactif.
		myData.dialog = NULL;
	}
	else
	{
		gtk_widget_destroy (myData.tray->widget);
	}
	myData.tray = NULL;
	memset (&myData, 0, sizeof (AppletData));
}

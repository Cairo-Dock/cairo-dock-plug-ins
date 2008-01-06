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

#include "systray-config.h"
#include "systray-init.h"

extern t_systray systray;

CD_APPLET_CONFIG_BEGIN ("systray", "gnome-panel-notification-area")
{
  systray.always_on_top = cairo_dock_get_boolean_key_value (pKeyFile, "GUI", "always on top", &bFlushConfFileNeeded, FALSE, NULL, NULL);
}
CD_APPLET_CONFIG_END


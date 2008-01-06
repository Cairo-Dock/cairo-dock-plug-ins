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

#include "stdlib.h"

#include "systray-config.h"
#include "systray-menu-functions.h"
#include "systray-init.h"
#include "cd-tray.h"

//extern t_systray systray;
t_systray systray = {
  NULL,
  NULL,
  FALSE
};

CD_APPLET_DEFINITION ("systray", 0, 0, 6)


CD_APPLET_INIT_BEGIN (erreur)
{
  if (systray.dialog) {
    systray_dialog_apply_settings();
  }
  //check if we can have a tray manager
/*   if (!systray.dialog) */
/*     g_return_val_if_fail(!na_tray_manager_check_running (gtk_widget_get_screen(pDock->pWidget)), NULL); */

  cairo_dock_register_first_notifications (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
                                           CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
                                           -1);
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
  cairo_dock_remove_notification_funcs (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
                                        CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
                                        -1);
}
CD_APPLET_STOP_END



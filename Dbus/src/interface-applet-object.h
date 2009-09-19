/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __INTERFACE_APPLET_OBJECT__
#define  __INTERFACE_APPLET_OBJECT__

#include <cairo-dock.h>
#include "applet-struct.h"


dbusApplet * cd_dbus_get_dbus_applet_from_instance (CairoDockModuleInstance *pModuleInstance);

dbusApplet *cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_delete_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_unregister_notifications (void);


gboolean cd_dbus_applet_is_used (const gchar *cModuleName);

void cd_dbus_launch_distant_applet_in_dir (const gchar *cModuleName, const gchar *cDirPath);

void cd_dbus_launch_distant_applet (const gchar *cModuleName);


#endif

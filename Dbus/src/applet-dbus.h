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

#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_dbus_clean_up_processes (gboolean bAll);

void cd_dbus_launch_service (void);

/**
 * Launch a subprocess (either an external applet or our launcher service).
 * Similar to cairo_dock_launch_command_argv_full(), but sets PYTHONPATH and RUBYLIB
 * to where our interfaces are installed, so that plugins written in Python
 * and Ruby will work properly.
 */
void cd_dbus_launch_subprocess (const gchar * const * args, const gchar *cWorkingDirectory);

gboolean cd_dbus_register_module_in_dir (const gchar *cModuleName, const gchar *cThirdPartyPath);

#endif


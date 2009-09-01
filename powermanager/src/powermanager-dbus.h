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

#ifndef __POWERMANAGER_DBUS__
#define  __POWERMANAGER_DBUS__

#include <glib.h>
#include <dbus/dbus-glib.h>

gboolean dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);

///void get_on_battery(void);
void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data);
gboolean update_stats(void);
///void detect_battery(void);
int get_stats(gchar *dataType);

void power_halt(void);
void power_hibernate(void);
void power_suspend(void);
void power_reboot(void);
#endif

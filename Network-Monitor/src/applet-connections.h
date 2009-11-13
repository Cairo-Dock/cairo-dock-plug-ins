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

#ifndef __APPLET_CONNECTIONS__
#define  __APPLET_CONNECTIONS__

#include <cairo-dock.h>

void cd_NetworkMonitor_get_data (gpointer data);
gboolean cd_NetworkMonitor_update_from_data (gpointer data);

static void cd_NetworkMonitor_quality (void);
static void cd_NetworkMonitor_get_wired_connection_infos (void);
static void cd_NetworkMonitor_get_wireless_connection_infos (void);

gboolean cd_NetworkMonitor_get_active_connection_info (void);
void cd_NetworkMonitor_connect_signals ();
void cd_NetworkMonitor_disconnect_signals();

void onChangeWirelessProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data);
void onChangeDeviceProperties (DBusGProxy *dbus_proxy, GHashTable *properties, gpointer data);
void onChangeActiveAccessPoint (DBusGProxy *dbus_proxy, GHashTable *AP_properties, gpointer data);


#endif

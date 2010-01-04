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
#define __APPLET_CONNECTIONS__

#include <cairo-dock.h>

#define CD_DBUS_TYPE_HASH_TABLE dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE)

#define CD_DBUS_TYPE_HASH_TABLE_OF_HASH_TABLE dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE))

gboolean cd_NetworkMonitor_connect_to_bus (void);

gboolean cd_NetworkMonitor_get_device (void);

gboolean cd_NetworkMonitor_get_connection (void);

void cd_NetworkMonitor_get_wired_connection_infos (void);
void cd_NetworkMonitor_get_wireless_connection_infos (void);


void cd_NetworkMonitor_quality (void);

void cd_NetworkMonitor_fetch_access_point_properties (GHashTable *hProperties);

void cd_NetworkMonitor_get_access_point_properties (void);

void cd_NetworkMonitor_get_new_access_point (void);


gboolean cd_NetworkMonitor_get_active_connection_info (void);


GtkWidget * cd_NetworkMonitor_build_menu_with_access_points (void);

#endif

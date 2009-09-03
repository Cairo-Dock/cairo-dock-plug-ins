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

#ifndef __RHYTHMBOX_DBUS__
#define  __RHYTHMBOX_DBUS__

#include <dbus/dbus-glib.h>


gboolean rhythmbox_dbus_connect_to_bus(void);
void rhythmbox_dbus_disconnect_from_bus (void);

void dbus_detect_rhythmbox(void);

void rhythmbox_getPlaying(void);
void rhythmbox_getPlayingUri(void);
void getSongInfos(gboolean bGetAll);
void onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);
void onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data);
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data);
void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data);


#endif

/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2010 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3, as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __DBUS_SHARED_NAMES_H__
#define __DBUS_SHARED_NAMES_H__

#define INDICATOR_SOUND_DBUS_NAME  "com.canonical.indicators.sound"
#define INDICATOR_SOUND_MENU_DBUS_OBJECT_PATH "/com/canonical/indicators/sound/menu"
#define INDICATOR_SOUND_SERVICE_DBUS_OBJECT_PATH "/com/canonical/indicators/sound/service"
#define INDICATOR_SOUND_DBUS_INTERFACE "com.canonical.indicators.sound"
#define INDICATOR_SOUND_DBUS_VERSION  0

#define INDICATOR_SOUND_SIGNAL_STATE_UPDATE               "SoundStateUpdate"


#endif /* __DBUS_SHARED_NAMES_H__ */

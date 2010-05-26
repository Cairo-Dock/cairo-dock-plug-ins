/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
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
#define __DBUS_SHARED_NAMES_H__ 1

#define INDICATOR_ME_DBUS_NAME  "org.ayatana.indicator.me"
#define INDICATOR_ME_DBUS_VERSION  1
#define INDICATOR_ME_DBUS_OBJECT "/org/ayatana/indicator/me/menu"
#define INDICATOR_ME_SERVICE_DBUS_OBJECT "/org/ayatana/indicator/me/service"
#define INDICATOR_ME_SERVICE_DBUS_INTERFACE "org.ayatana.indicator.me.service"

#define DBUSMENU_ENTRY_MENUITEM_TYPE           "x-canonical-entry-item"
#define DBUSMENU_ENTRY_MENUITEM_PROP_TEXT      "text"

#define DBUSMENU_ABOUT_ME_MENUITEM_TYPE           "x-canonical-about-me-item"
#define DBUSMENU_ABOUT_ME_MENUITEM_PROP_NAME      "name"
#define DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON      "icon"

#endif /* __DBUS_SHARED_NAMES_H__ */

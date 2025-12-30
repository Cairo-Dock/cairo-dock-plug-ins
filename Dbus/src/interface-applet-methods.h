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


#ifndef __INTERFACE_APPLET_METHODS__
#define  __INTERFACE_APPLET_METHODS__

#include <cairo-dock.h>
#include "applet-struct.h"

void cd_dbus_applet_method_call (GDBusConnection *pConn, const gchar *cSender, const gchar *cObj,
	const gchar *cInterface, const gchar *cMethod, GVariant *pPar, GDBusMethodInvocation* pInv, gpointer data);
GVariant *cd_dbus_applet_get_property (GDBusConnection *pConn, const gchar *cSender, const gchar *cObj,
	const gchar *cInterface, const gchar* cProperty, GError** error, gpointer data);

void cd_dbus_sub_applet_method_call (GDBusConnection *pConn, const gchar *cSender, const gchar *cObj,
	const gchar *cInterface, const gchar *cMethod, GVariant *pPar, GDBusMethodInvocation* pInv, gpointer data);
GVariant *cd_dbus_sub_applet_get_property (GDBusConnection *pConn, const gchar *cSender, const gchar *cObj,
	const gchar *cInterface, const gchar* cProp, GError** error, gpointer data);

#endif


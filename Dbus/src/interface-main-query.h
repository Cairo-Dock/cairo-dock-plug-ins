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

#ifndef __APPLET_MAIN_QUERY__
#define  __APPLET_MAIN_QUERY__

#include <cairo-dock.h>
#include "applet-struct.h"

typedef enum {
	CD_MAIN_TYPE_ICON,
	CD_MAIN_TYPE_CONTAINER,
	CD_MAIN_TYPE_MODULE,
	CD_MAIN_TYPE_MODULE_INSTANCE,
	CD_MAIN_TYPE_UNKNOWN
} CDMainType;

#define CD_TYPE_ICON "Icon"
#define CD_TYPE_LAUNCHER "Launcher"
#define CD_TYPE_APPLICATION "Application"
#define CD_TYPE_APPLET "Applet"
#define CD_TYPE_SEPARATOR "Separator"
#define CD_TYPE_STACK_ICON "Stack-icon"
#define CD_TYPE_CLASS_ICON "Class-icon"
#define CD_TYPE_ICON_OTHER "Other"
#define CD_TYPE_CONTAINER "Container"
#define CD_TYPE_DOCK "Dock"
#define CD_TYPE_DESKLET "Desklet"
#define CD_TYPE_MODULE "Module"
#define CD_TYPE_MANAGER "Manager"
#define CD_TYPE_MODULE_INSTANCE "Module-Instance"


CDMainType cd_dbus_get_main_type (const gchar *cType, int n);


GList *cd_dbus_find_matching_objects (gchar *cQuery);


GList *cd_dbus_find_matching_icons (gchar *cQuery);


#endif

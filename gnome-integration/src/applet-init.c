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

#include "stdlib.h"

#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN ("gnome integration",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	"This applet provides functions for a better integration into a GNOME environnement.\n"
	"It is auto-activated, so you don't need to activate it.\n"
	"It is designed for the a GNOME version >= 2.22",
	"Fabounet (Fabrice Rey)")
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
	{
		cd_debug ("GNOME");
		env_backend_init (); // will detect GNOME vs Cinnamon and register the required functions later
		// (this means that if GNOME environment is detected, this applet will always show as "enabled",
		// but will only be active if the DBus session proxy have been found)
	}
	else
		return FALSE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END

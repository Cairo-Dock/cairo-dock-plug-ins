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


CD_APPLET_DEFINE2_BEGIN ("Cosmic integration",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	"This applet provides functions for a better integration into a COSMIC environnement.\n"
	"It is auto-activated, so you don't need to activate it.",
	"Daniel Kondor")
	
	gboolean bEnabled = FALSE;
	
	if (g_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV) // currently, there is no enum defined for COSMIC, but it is not necessary
	{
		const gchar *cEnv = g_getenv ("XDG_CURRENT_DESKTOP");
		if (cEnv && strstr(cEnv, "COSMIC"))
		{
			bEnabled = TRUE;
			CairoDockDesktopEnvBackend VFSBackend = { NULL };
		
			VFSBackend.logout = env_backend_logout;
			VFSBackend.shutdown = env_backend_shutdown;
			VFSBackend.reboot = env_backend_reboot;
			VFSBackend.lock_screen = env_backend_lock_screen;
			VFSBackend.setup_time = env_backend_setup_time;
			cairo_dock_fm_register_vfs_backend (&VFSBackend, TRUE); // TRUE: overwrite previously registered functions
		
			CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
		}
	}
	if (!bEnabled) gldi_module_disable (pModule, _("This applet is only supported in a Cosmic session.\nIf you believe it should work in your setup, please open a bug report at:\nhttps://github.com/Cairo-Dock/cairo-dock-core/issues"));
CD_APPLET_DEFINE2_END

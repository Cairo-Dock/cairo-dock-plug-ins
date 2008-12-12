/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <cairo-dock.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	cairo_dock_launch_command ("gnome-session-save --kill --gui");
}

void env_backend_shutdown (void)
{
	cairo_dock_launch_command ("gnome-session-save --shutdown-dialog");
}

void env_backend_setup_time (void)
{
	cairo_dock_launch_command ("time-admin");  // utilise PolicyKit => pas de gksudo.
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command ("gnome-system-monitor");
}
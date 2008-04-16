/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	cairo_dock_launch_command ("gnome-session-save --kill --gui");
}

void env_backend_setup_time (void)
{
	cairo_dock_launch_command ("gksu time-admin", &erreur);
}

/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to chris.chapuis@gmail.com)

******************************************************************************/
#include <stdlib.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	cairo_dock_launch_command ("xfce4-session-logout");
}

void env_backend_setup_time (void)
{
	cairo_dock_launch_command ("gksu time-admin");
}


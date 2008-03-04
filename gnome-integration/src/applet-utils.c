/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	system ("gnome-session-save --kill --gui");
}

void env_backend_setup_time (void)
{
	GError *erreur = NULL;
	g_spawn_command_line_async ("gksu time-admin", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s", "gksu time-admin", erreur->message);
		g_error_free (erreur);
	}
}

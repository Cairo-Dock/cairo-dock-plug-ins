/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to chris.chapuis@gmail.com)

******************************************************************************/
#include <stdlib.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	GError *erreur = NULL;
	g_spawn_command_line_async ("xfce4-session-logout", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : when trying to execute '%s' : %s\n", "gksu time-admin", erreur->message);
		g_error_free (erreur);
	}
}

void env_backend_setup_time (void)
{
	GError *erreur = NULL;
	g_spawn_command_line_async ("gksu time-admin", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : when trying to execute '%s' : %s\n", "gksu time-admin", erreur->message);
		g_error_free (erreur);
	}
}


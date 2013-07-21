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

#define _BSD_SOURCE
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <string.h>
#include <time.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-dropbox.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link")};

static void upload (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	// Upload the file
	gchar *cCommand;
	if (cLocalDir)
		cCommand = g_strdup_printf ("cp \"%s\" \"%s\"", cFilePath, cLocalDir);
	else
		cCommand= g_strdup_printf ("cp \"%s\" ~/Dropbox/Public", cFilePath);
	cd_debug ("commande dropbox1 : %s", cCommand);
	int r = system (cCommand);
	if (r < 0)
		cd_warning ("Not able to launch this command: %s", cCommand);
	g_free (cCommand);
	
	// get the result URL (available immediately, no need to loop on 'dropbox status' until having 'Idle').
	gchar *cFileName = g_path_get_basename (cFilePath);
	if (cLocalDir)
	{
		gchar *str = g_strstr_len (cLocalDir, -1, "Dropbox");
		if (!str)
		{
			str = strrchr (cLocalDir, '/');
			if (str)
				str ++;
		}
		if (!str)
		{
			cd_warning ("Wrong dropbox dir");
			g_set_error (pError, 1, 1, "%s %s", D_("This directory seems not valid:"), cLocalDir);
			return;
		}

		cCommand = g_strdup_printf ("dropbox puburl \"%s/%s\"", cLocalDir, cFileName);
	}
	else
		cCommand = g_strdup_printf ("dropbox puburl \"%s/Dropbox/Public/%s\"", getenv("HOME"), cFileName); 
	
	cd_debug ("command dropbox2 : %s", cCommand);
	g_free (cFileName);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		cd_warning ("Dropbox did not give use an address :-(");
		DND2SHARE_SET_GENERIC_ERROR_SERVICE ("Dropbox", "dropbox");
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\r')
		cResult[strlen(cResult)-1] = '\0';
	if (cResult[strlen(cResult)-1] == '\n')
		cResult[strlen(cResult)-1] = '\0';
	
	// fill the shared memory with our URLs.
	cResultUrls[0] = cResult;
}


void cd_dnd2share_register_dropbox_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_FILE,
		"DropBox",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

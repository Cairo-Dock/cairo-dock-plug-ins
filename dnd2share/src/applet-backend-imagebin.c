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

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-imagebin.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link")};


static void upload (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	// Upload the file
	gchar *cCommand = g_strdup_printf ("curl -L --connect-timeout 5 --retry 2 --limit-rate %dk http://imagebin.ca/upload.php -F file=@\"%s\"", iLimitRate, cFilePath);
	cd_debug ("%s", cCommand);
	gchar *cContent = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);

	if (! cContent)
	{
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("ImageBin");
		return;
	}

	// We have the content, now we can extract data
	gchar *cURL = NULL;
	gchar *str = strstr (cContent, "url:");
	if (str != NULL)
	{
		str += 4;
		gchar *end;
		if ((end = strchr (str, '\n')) != NULL
		 || (end = strchr (str, '\0')) != NULL)
		{
			*end = '\0';
			cURL = g_strdup (str);
		}
	}
	g_free (cContent);

	if (cURL == NULL)
	{
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("ImageBin");
		return;
	}

	cResultUrls[0] = cURL;
}


void cd_dnd2share_register_imagebin_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_IMAGE,
		"imagebin.ca",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

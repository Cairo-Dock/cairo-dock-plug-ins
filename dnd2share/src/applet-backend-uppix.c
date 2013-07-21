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

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-uppix.h"

#define NB_URLS 3
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link"), N_("Thumbnail"), "BBCode"};


static void upload (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	// Upload the file
	gchar *cCommand = g_strdup_printf ("curl -L --connect-timeout 5 --retry 2 --limit-rate %dk uppix.com/upload -H Expect: -F u_file=@\"%s\" -F u_submit=Upload -F u_agb=yes", iLimitRate, cFilePath);
	cd_debug ("%s", cCommand);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);

	if (! cResult)
	{
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("Uppix.com");
		return;
	}

	gchar *cDirectLink = NULL, *cThumbnail = NULL, *cBBCode = NULL;
	gchar *cDirectLinkStart = strstr (cResult, "http://uppix.com/"); // find the direct link
	if (cDirectLinkStart)
	{
		gchar *cDirectLinkEnd = strstr (cDirectLinkStart, "&quot;");
		if (cDirectLinkEnd)
		{
			*cDirectLinkEnd = '\0';
			gint iLength = cDirectLinkEnd - cDirectLinkStart;
			cDirectLink = g_strdup (cDirectLinkStart); // http://uppix.com/f-(...)
			cThumbnail = g_strdup (cDirectLink); // the same url but with 't' instead of 'f' => http://uppix.com/t-(...)
			if (iLength > 17 && cThumbnail[17] == 'f')
				cThumbnail[17] = 't';
			cBBCode = g_strdup_printf ("[url=%s][img]%s[/img][/url]", cDirectLink, cThumbnail);
		}
	}
	else
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("Uppix.com");
	g_free (cResult);

	cResultUrls[0] = cDirectLink;
	cResultUrls[1] = cThumbnail;
	cResultUrls[2] = cBBCode;
}


void cd_dnd2share_register_uppix_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_IMAGE,
		"Uppix.com",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

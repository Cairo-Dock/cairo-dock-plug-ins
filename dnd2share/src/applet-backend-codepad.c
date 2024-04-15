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
#include "applet-backend-codepad.h"

#define URL "http://codepad.org"
#define FORMAT "Plain Text"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link")};

/*<html>
 <head>
  <title>302 Found</title>
 </head>
 <body>
  <h1>302 Found</h1>
  The resource was found at <a href="http://codepad.org/INmbIYX2">http://codepad.org/INmbIYX2</a>;
you should be redirected automatically.
 </body>
</html>'*/
static void upload (const gchar *cText, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	if (cText == NULL || *cText == '\0')
	{
		g_set_error (pError, 1, 1, D_("Your text is empty and couldn't be uploaded to this server"));
		return;
	}

	GError *erreur = NULL;
	gchar *cResult = cairo_dock_get_url_data_with_post (URL, FALSE, &erreur,
		"code", cText,
		"lang", FORMAT,
		"submit", "Submit",
		NULL);
	if (erreur)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE("Codepad.org");
	}
	else if (cResult)
	{
		// Get the next '"http://(...)"' link after 'Link:' text
		gchar *str, *end;
		if ((str = strstr (cResult, "Link:")) != NULL
		 && (str = strstr (str, "\"http://") + 1) != NULL
		 && (end = strchr (str, '"')) != NULL)
		{
			*end = '\0';
			cResultUrls[0] = g_strdup (str);
		}
		else
			DND2SHARE_SET_GENERIC_ERROR_WEBSITE("Codepad.org");
		g_free (cResult);
	}
	// else -> generic error
}


void cd_dnd2share_register_codepad_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_TEXT,
		"codepad.org",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

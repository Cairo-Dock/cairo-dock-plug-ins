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
#include "applet-backend-pastebin-mozilla.h"

#define URL "http://pastebin.mozilla.org"
#define FORMAT "text"
#define EXPIRE "d"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link")};

/*HTTP/1.1 302 Found
Date: Sun, 23 Jan 2011 00:57:12 GMT
Server: Apache
X-Backend-Server: pm-app-generic03
X-Powered-By: PHP/5.2.9
Location: http://pastebin.mozilla.org/972341
Content-Length: 0
Content-Type: text/html; charset=UTF-8*/


static void upload (const gchar *cText, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	if (cText == NULL || *cText == '\0')
	{
		g_set_error (pError, 1, 1, D_("Your text is empty and couldn't be uploaded to this server"));
		return;
	}
	GError *erreur = NULL;
	gchar *cResult = cairo_dock_get_url_data_with_post (URL, TRUE, &erreur,
		"code2", cText,
		"expiry", EXPIRE,
		"format", FORMAT,
		"paste", "Send",
		"poster", (bAnonymous ? "Anonymous" : g_getenv("USER")),
		"remember", "0",
		"parent_pid", "",
		NULL);
	if (erreur)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
			DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("Pastebin Mozilla");
	}
	else if (cResult)
	{
		cd_debug (" --> got '%s'", cResult);
		gchar *str = strstr (cResult, "Location:");
		if (!str)
		{
			DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("Pastebin Mozilla");
			return;
		}
		str += 9;
		while (*str == ' ')
			str ++;
		gchar *rc = strchr (str, '\r');  // header's lines are separated by CRLF (\r\n).
		if (rc)
			*rc = '\0';
		cResultUrls[0] = g_strdup (str);
		g_free (cResult);
	}
}


void cd_dnd2share_register_pastebin_mozilla_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_TEXT,
		"pastebin-mozilla.org",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

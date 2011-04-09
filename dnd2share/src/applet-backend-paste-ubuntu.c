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

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-paste-ubuntu.h"

#define URL "http://paste.ubuntu.com"
#define FORMAT "text"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};

/*HTTP/1.1 302 Found
Date: Sun, 23 Jan 2011 00:48:37 GMT
Server: Apache/2.2.8 (Ubuntu) mod_python/3.3.1 Python/2.5.2 mod_ssl/2.2.8 OpenSSL/0.9.8g mod_wsgi/1.3 mod_perl/2.0.3 Perl/v5.8.8
Vary: Cookie
Content-Type: text/html; charset=utf-8
Location: http://paste.ubuntu.com/557014/
Content-Length: 0*/

static void upload (const gchar *cText, gchar *cDropboxDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls)
{
	GError *erreur = NULL;
	gchar *cResult = cairo_dock_get_url_data_with_post (URL, TRUE, &erreur,
		"content", cText,
		"poster", (bAnonymous ? "Anonymous" : g_getenv("USER")),
		"syntax", FORMAT,
		"submit", "Paste!",
		NULL);
	if (erreur)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	else if (cResult)
	{
		g_print (" --> got '%s'\n", cResult);
		gchar *str = strstr (cResult, "Location:");
		if (!str)
			return;
		str += 9;
		while (*str == ' ')
			str ++;
		gchar *rc = strchr (str, '\r');  // les lignes du header sont separes par des CRLF (\r\n).
		if (rc)
			*rc = '\0';
		cResultUrls[0] = g_strdup (str);
		g_free (cResult);
	}
}

void cd_dnd2share_register_paste_ubuntu_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_TEXT,
		"paste-ubuntu.com",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

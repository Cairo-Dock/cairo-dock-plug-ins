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


static void upload (const gchar *cText)
{
	GError *erreur = NULL;
	gchar *cContent = cairo_dock_get_url_data_with_post (URL, &erreur,
		"content", cText,
		"poster", (myConfig.bAnonymous ? "Anonymous" : g_getenv("USER")),
		"syntax", FORMAT,
		NULL);
	if (erreur)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	else
	{
		g_print (" --> got '%s'\n", cContent);
		myData.cResultUrls[0] = cContent;
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

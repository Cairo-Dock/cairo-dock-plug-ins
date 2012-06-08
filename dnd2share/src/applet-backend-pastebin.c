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
#include "applet-backend-pastebin.h"

#define URL "http://pastebin.com/api/api_post.php"
#define FORMAT "text"  // pastebin is only used for text.
#define EXPIRE "1M"  // 1 month
#define DEV_KEY "4dacb211338b25bfad20bc6d4358e555"  // if you re-use this code, please make your own key !
#define OPTION "paste"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cText, gchar *cDropboxDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls)
{
	GError *erreur = NULL;
	// http://pastebin.com/api
	gchar *cResult = cairo_dock_get_url_data_with_post (URL, FALSE, &erreur,
		"api_option", OPTION,
		"api_user_key", "", // guest
		"api_paste_private", bAnonymous ? "1" : "0", // unlisted or public
		"api_paste_name", bAnonymous ? "" : g_getenv ("USER"),
		"api_paste_expire_date", EXPIRE,
		"api_paste_format", FORMAT,
		"api_dev_key", DEV_KEY,
		"api_paste_code", cText,
		NULL);
	if (erreur)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
	}
	else
	{
		cd_debug (" --> got '%s'", cResult);
		cResultUrls[0] = cResult;
	}
}


void cd_dnd2share_register_pastebin_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_TEXT,
		"pastebin.com",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

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

#define NB_URLS 2
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link"), N_("Thumbnail")};


static void upload (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	// Upload the file
	gchar *cCommand = g_strdup_printf ("curl -L --connect-timeout 5 --retry 2 --limit-rate %dk http://imageshack.us/upload_api.php -H Expect: -F xml=yes -F public=no -F fileupload=@\"%s\" -F key=ABDGHOQS7d32e206ee33ef8cefb208d55dd030a6", iLimitRate, cFilePath);
	gchar *cContent = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);

	if (! cContent)
	{
		DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("ImageShack");
		return;
	}

	// We have the content, now we can extract data
	gchar *cURL = NULL, *cThumbnail = NULL;

	gchar *str = strstr (cContent, "<image_link>");
	if (str != NULL)
	{
		str += 12;  // <image_link>http://bla/bla/bla.png</image_link>
		gchar *end = strstr (str, "</image_link>");
		if (end != NULL)
			cURL = g_strndup (str, end - str);
		else
		{
			g_free (cContent);
			DND2SHARE_SET_GENERIC_ERROR_WEBSITE ("ImageShack");
			return;
		}
	}

	str = strstr (cContent, "<is_link>");
	if (str != NULL)
	{
		str += 9;  // <thumb_link>http://bla/bla/bla.png</thumb_link>
		gchar *end = strstr (str, "</is_link>");
		if (end != NULL)
			cThumbnail = g_strndup (str, end - str);
	}

	g_free (cContent);

	cResultUrls[0] = cURL;
	cResultUrls[1] = cThumbnail;
}


void cd_dnd2share_register_imageshack_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_IMAGE,
		"imageshack.us",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

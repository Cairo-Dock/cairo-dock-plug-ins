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
*
* Inspired by Imgur Bash Upload Script (http://imgur.com/tools/imgurbash.sh)
*/

#define _BSD_SOURCE
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-imgur.h"

#define NB_URLS 6
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink", "DisplayImage", "Large", "Small", "BBCode", "HTML"};

/*
<?xml version="1.0" encoding="utf-8"?>
<rsp stat="ok"><image_hash>Elqb97k</image_hash><delete_hash>qkIeCD33HUtBA3h</delete_hash><original_image>http://i.imgur.com/Elqb97k.png</original_image><large_thumbnail>http://i.imgur.com/Elqb97kl.jpg</large_thumbnail><small_thumbnail>http://i.imgur.com/Elqb97ks.jpg</small_thumbnail><imgur_page>http://imgur.com/Elqb97k</imgur_page><delete_page>http://imgur.com/delete/qkIeCD33HUtBA3h</delete_page></rsp>
*/

static void upload (const gchar *cFilePath, gchar *cDropboxDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls)
{
	gchar *cCommand = g_strdup_printf ("curl -L --connect-timeout 5 --retry 2 --limit-rate %dk http://imgur.com/api/upload.xml -F key=b3625162d3418ac51a9ee805b1840452 -H \"Expect: \" -F image=@\"%s\"", iLimitRate, cFilePath);
	cd_debug ("%s", cCommand);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);

	if (! cResult)
		return;

	gchar *cHashStart = strstr (cResult, "<image_hash>");
	if (cHashStart)
	{
		cHashStart += 12;
		gchar *cHashEnd = strstr (cHashStart, "</image_hash>");
		if (cHashEnd)
			*cHashEnd = '\0';
	}
	gchar *cHash = g_strdup (cHashStart);
	g_free (cResult);

	const gchar *cExt = strrchr (cFilePath, '.');
	if (cExt == NULL)
		cExt = "";
	cResultUrls[0] = g_strdup_printf ("http://i.imgur.com/%s%s", cHash, cExt);
	cResultUrls[1] = g_strdup_printf ("http://imgur.com/%s", cHash);
	cResultUrls[2] = g_strdup_printf ("http://i.imgur.com/%sl.jpg", cHash);
	cResultUrls[3] = g_strdup_printf ("http://i.imgur.com/%ss.jpg", cHash);
	cResultUrls[4] = g_strdup_printf ("[URL=http://imgur.com/%s][IMG]http://i.imgur.com/%s%s[/IMG][/URL]", cHash, cHash, cExt);
	cResultUrls[5] = g_strdup_printf ("<a href='http://imgur.com/%s'><img src='http://i.imgur.com/%s%s' title='Hosted by imgur.com and sent with Cairo-Dock' /></a>", cHash, cHash, cExt);
}


void cd_dnd2share_register_imgur_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_IMAGE,
		"Imgur.com",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

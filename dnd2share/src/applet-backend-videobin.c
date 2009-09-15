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
#include "applet-backend-videobin.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath)
{
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("curl --connect-timeout 5 --retry 2 --limit-rate %dk http://www.videobin.org/add -F videoFile=@'%s' -F api=1", myConfig.iLimitRate, cFilePath);
	g_print ("%s\n", cCommand);
	gchar *cURL = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);

	if (cURL == NULL)
	{
		return ;
	}

	
	// Enfin on remplit la memoire partagee avec nos URLs.
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = cURL;
}


void cd_dnd2share_register_videobin_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_VIDEO,
		"videobin.org",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

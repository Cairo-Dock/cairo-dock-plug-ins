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
#include "applet-backend-imagebin.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath, gchar *cDropboxDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls)
{
	// On cree un fichier de log temporaire.
	gchar *cLogFile = g_strdup ("/tmp/dnd2share-log.XXXXXX");
	int fds = mkstemp (cLogFile);
	if (fds == -1)
	{
		g_free (cLogFile);
		return ;
	}
	close(fds);
	
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("curl -L --connect-timeout 5 --retry 2 --limit-rate %dk http://imagebin.ca/upload.php -F f=@\"%s\" -F t=file -o \"s\"", iLimitRate, cFilePath, cLogFile);
	cd_debug ("%s", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	
	// On récupère l'URL dans le log :
	gchar *cURL = NULL;
	gchar *cContent = NULL;
	gsize length = 0;
	g_file_get_contents (cLogFile, &cContent, &length, NULL);
	gchar *str = g_strstr_len (cContent, -1, "href='");
	if (str != NULL)
	{
		str += 6;
		gchar *end = strchr (str, '\'');
		if (end != NULL)
		{
			*end = '\0';
			cURL = g_strdup (str);
		}
	}
	g_free (cContent);
	
	g_remove (cLogFile);
	g_free (cLogFile);
	
	if (cURL == NULL)
	{
		return ;
	}
	
	// Enfin on remplit la memoire partagee avec nos URLs.
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

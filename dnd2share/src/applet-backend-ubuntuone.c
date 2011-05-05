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
#include "applet-backend-ubuntuone.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath, gchar *cDropboxDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls)
{
	// On lance la commande d'upload.
	gchar *cFileName = g_path_get_basename (cFilePath);
	gchar *cLocalFilePath;
	if (cDropboxDir)
		cLocalFilePath = g_strdup_printf ("%s/%s", cDropboxDir, cFileName);
	else
		cLocalFilePath = g_strdup_printf ("/home/%s/Ubuntu One/%s", g_getenv ("USER"), cFileName);
	g_free (cFileName);
	
	gchar *cCommand = g_strdup_printf ("cp \"%s\" \"%s\"", cFilePath, cLocalFilePath);
	cd_debug ("commande u1 : %s", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	if (r != 0)
	{
		cd_warning ("couldn't copy the file to %s", cLocalFilePath);
		g_free (cLocalFilePath);
		return;
	}
	
	// We wait for the end of the sync
	cairo_dock_launch_command_sync ("u1sdtool --wait");
	// We publish the file and we read the output message
	cCommand = g_strdup_printf ("u1sdtool --publish-file \"%s\"", cLocalFilePath);
	cd_debug ("commande u2 : %s", cCommand);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	g_free (cLocalFilePath);
	if (cResult == NULL || *cResult == '\0')
	{
		cd_warning ("is u1sdtool installed?");
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\r')
		cResult[strlen(cResult)-1] = '\0';
	if (cResult[strlen(cResult)-1] == '\n')
		cResult[strlen(cResult)-1] = '\0';
	
	// Enfin on remplit la memoire partagee avec nos URLs.
	gchar *str = strstr (cResult, "http");  // File is published at http://ubuntuone.com/x/y
	if (! str)
	{
		cd_warning ("couldn't publish this file: %s", cResult);
		g_free (cResult);
		return ;
	}
	cResultUrls[0] = g_strdup (str);
	g_free (cResult);
}


void cd_dnd2share_register_ubuntuone_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_FILE,
		"UbuntuOne",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

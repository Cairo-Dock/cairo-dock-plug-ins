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
#include "applet-backend-dropbox.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath)
{
	// On lance la commande d'upload.
	gchar *cCommand;
	if (myConfig.cDropboxDir)
		cCommand= g_strdup_printf ("cp '%s' '%s'", cFilePath, myConfig.cDropboxDir);
	else
		cCommand= g_strdup_printf ("cp '%s' ~/Dropbox/Public", cFilePath);
	g_print ("commande dropbox1 : %s\n", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	
	// On recupere l'URL (dispo tout de suite, sinon il faudra boucler en testant 'dropbox status' jusqu'a avoir 'Idle').
	gchar *cFileName = g_path_get_basename (cFilePath);
	if (myConfig.cDropboxDir)
	{
		gchar *str = g_strstr_len (myConfig.cDropboxDir, -1, "Dropbox");
		if (!str)
		{
			str = strrchr (myConfig.cDropboxDir, '/');
			if (str)
				str ++;
		}
		g_return_if_fail (str != NULL);
			
		cCommand = g_strdup_printf ("dropbox puburl '%s/%s'", myConfig.cDropboxDir, cFileName);
	}
	else
		cCommand = g_strdup_printf ("dropbox puburl 'Dropbox/Public/%s'", cFileName); 
	
	g_print ("commande dropbox2 : %s\n", cCommand);
	g_free (cFileName);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		cd_warning ("Dropbox ne nous a pas renvoye d'adresse :-(");
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\r')
		cResult[strlen(cResult)-1] = '\0';
	if (cResult[strlen(cResult)-1] == '\n')
		cResult[strlen(cResult)-1] = '\0';
	
	// Enfin on remplit la memoire partagee avec nos URLs.
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = cResult;
}


void cd_dnd2share_register_dropbox_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_FILE,
		"DropBox",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

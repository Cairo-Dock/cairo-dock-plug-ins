/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

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
	g_return_if_fail (myConfig.cCustomScripts[myData.iCurrentFileType] != NULL);
	
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("dropbox start && cp '%s' '~/Dropbox/Public/%s'", cFilePath, myConfig.cDropboxDir ? myConfig.cDropboxDir : "");
	int r = system (cCommand);
	g_free (cCommand);
	
	// On recupere l'URL (dispo tout de suite, sinon il faudra boucler en testant 'dropbox status' jusqu'a avoir 'Idle').
	gchar *cFileName = g_path_get_basename (cFilePath);
	cCommand = g_strdup_printf ("dropbox puburl 'Dropbox/Public/%s/%s'", myConfig.cDropboxDir ? myConfig.cDropboxDir : "", cFileName);
	
	g_print ("commande dropbox : %s\n", cCommand);
	g_free (cFileName);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		cd_warning ("Dropbox ne nous a pas renvoye d'adresse :-(");
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\n' || cResult[strlen(cResult)-1] == '\r')
		cd_warning ("enlever le retour chariot !");
	
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

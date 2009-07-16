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
#include "applet-backend-videobin.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath)
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
	gchar *cCommand = g_strdup_printf ("curl --connect-timeout 5 --retry 2 http://www.videobin.org/add -F videoFile=@'%s' -o '%s'", cFilePath, cLogFile);
	g_print ("%s\n", cCommand);
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

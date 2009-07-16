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
#include "applet-backend-pastebin.h"

#define API_KEY "rtrtXD3HF2MIAj7k6lrmRScDkTg3U5In"

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
	
	// On remplace les espaces du texte par des %20.
	gchar **cTextParts = g_strsplit (cFilePath, " ", -1);
	GString *sContent = g_string_sized_new (strlen (cFilePath) + 300);  // 100 espaces d'avance.
	int i;
	for (i = 0; cTextParts[i] != NULL; i ++)
	{
		g_string_append (sContent, "%20");
		g_string_append (sContent, cTextParts[i]);
	}
	g_strfreev (cTextParts);
	
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("wget -O \"%s\" --tries=2 --timeout=5 --post-data \"content=%s&api=%s&description=cairo-dock&type=1&expiry=1%%20month&name=%s\" http://pastebin.ca/quiet-paste.php", cLogFile, sContent->str, API_KEY, g_getenv ("USER"));  // apparemment plus facile a faire avec wget qu'avec curl, qui retourne une erreur 417 a chaque fois.
	g_string_free (sContent, TRUE);
	g_print ("%s\n", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	
	// On récupère l'URL dans le log :
	gchar *cURL = NULL;
	gchar *cContent = NULL;
	gsize length = 0;
	g_file_get_contents (cLogFile, &cContent, &length, NULL);
	
	gchar *str = g_strstr_len (cContent, -1, "SUCCESS:");
	if (str != NULL)
	{
		int index = atoi (str+8);
		cURL = g_strdup_printf ("http://pastebin.ca/%d", index);
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


void cd_dnd2share_register_pastebin_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_TEXT,
		"pastebin.ca",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

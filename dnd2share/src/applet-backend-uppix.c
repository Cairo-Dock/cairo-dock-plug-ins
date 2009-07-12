/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Based on a script made by pmd (http://pmdz.info). 
Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#define _BSD_SOURCE
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-uppix.h"

#define NB_URLS 5
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink", "DisplayImage", "BBCode150px", "BBCode600px", "BBCodeFullPic"};  /// franchement le seul lien interessant c'est DirectLink non ?...


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
	gchar *cCommand = g_strdup_printf ("curl --connect-timeout 5 --retry 2 uppix.net -F myimage=@'%s' -F submit=Upload -F formup=1 -o %s", cFilePath, cLogFile);  /// peut-on ajouter le nom de l'auteur dans le formulaire ?...
	g_print ("%s\n", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	
	
	// On récupère toutes les infos dans le fichier de log :
	/// TODO : se passer des g_spawn_command_line_sync !
	//D'abord l'url de DisplayImage
	gchar *cDisplayImage = NULL;
	gchar *cCommandDisplayImage = g_strdup_printf ("grep -oEm 1 '\\[url\\=([^]]*)' %s", cLogFile);
	g_spawn_command_line_sync (cCommandDisplayImage, &cDisplayImage,  NULL, NULL, NULL);
	g_free (cCommandDisplayImage);
	
	if (cDisplayImage == NULL || *cDisplayImage == '\0')
	{
		g_remove (cLogFile);
		g_free (cLogFile);
		return ;
	}
	
	gchar *str = g_strstr_len (cDisplayImage, -1, "http://"); // On retire tout ce qui se trouve avant http://
	if (str != NULL && str != cDisplayImage)
	{
		gchar *tmp = cDisplayImage;
		cDisplayImage = g_strdup (str);
		g_free (tmp);
	}
	cDisplayImage[strlen(cDisplayImage) - 1] = '\0';  // on retire le \n à la fin
	cd_debug ("DND2SHARE : Display Image = %s", cDisplayImage);
	
	// Puis l'url de DirectLink
	gchar *cDirectLink = NULL;
	gchar *cCommandDirectLink = g_strdup_printf ("grep -oEm 1 '\\[img\\]([^[]*)' %s", cLogFile);
	g_spawn_command_line_sync (cCommandDirectLink, &cDirectLink,  NULL, NULL, NULL);
	g_free (cCommandDirectLink);
	
	str = g_strstr_len (cDirectLink, -1, "http://"); // On retire tout ce qui se trouve avant http://
	if (str != NULL && str != cDirectLink)
	{
		gchar *tmp = cDirectLink;
		cDirectLink = g_strdup (str);
		g_free (tmp);
	}
	cDirectLink[strlen(cDirectLink) - 1] = '\0';  // on retire le \n à la fin
	cd_debug ("DND2SHARE : Direct Link = %s", cDirectLink);

	// Puis on créé les autres URLs à la mano ;-) :
	gchar *cBBCodeFullPic = g_strdup_printf ("[url=%s][img]%s[/img][/url]", cDisplayImage, cDirectLink);
	cd_debug ("DND2SHARE : BBCODE_Full = '%s'", cBBCodeFullPic);
	
	gchar *cDirectLinkWithoutExt = g_strdup (cDisplayImage);
	cDirectLinkWithoutExt[strlen(cDirectLinkWithoutExt) - 5] = '\0';  // on retire le .html à la fin
	gchar *cBBCode150px = g_strdup_printf ("[url=%s][img]%st.jpg[/img][/url]", cDisplayImage, cDirectLinkWithoutExt);
	gchar *cBBCode600px = g_strdup_printf ("[url=%s][img]%stt.jpg[/img][/url]", cDisplayImage, cDirectLinkWithoutExt);
	g_free (cDirectLinkWithoutExt);
	cd_debug ("DND2SHARE : BBCODE_150px = '%s'", cBBCode150px);
	cd_debug ("DND2SHARE : BBCODE_600px = '%s'", cBBCode600px);
	
	g_remove (cLogFile);
	g_free (cLogFile);
	
	// Enfin on remplit la memoire partagee avec nos URLs.
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = cDirectLink;
	myData.cResultUrls[1] = cDisplayImage;
	myData.cResultUrls[2] = cBBCode150px;
	myData.cResultUrls[3] = cBBCode600px;
	myData.cResultUrls[4] = cBBCodeFullPic;
}


void cd_dnd2share_register_uppix_backend (void)
{
	cd_dnd2share_register_new_backend (CD_TYPE_IMAGE,
		"Uppix.net",
		NB_URLS,
		s_UrlLabels,
		0,
		upload);
}

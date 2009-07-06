/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#define _BSD_SOURCE
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-backend-free.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};

#define EMAIL "fab@fab.fr"
#define PWD "cairo-dock"

static void upload (const gchar *cFilePath, CDFileType iFileType)
{
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("%s/%s '%s'", MY_APPLET_SHARE_DATA_DIR, "upload2free.sh", cFilePath);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\n' || cResult[strlen(cResult)-1] == '\r')
		g_print ("enlever le dernier char\n");
	
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = cResult;
}


void cd_dnd2share_register_free_backend (void)
{
	myData.backends[CD_FREE].cSiteName = "dl.free.fr";
	myData.backends[CD_FREE].iNbUrls = NB_URLS;
	myData.backends[CD_FREE].cUrlLabels = s_UrlLabels;
	myData.backends[CD_FREE].iPreferedUrlType = 0;
	myData.backends[CD_FREE].upload = upload;
}

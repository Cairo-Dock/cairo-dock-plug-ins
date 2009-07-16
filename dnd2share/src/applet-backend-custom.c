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
#include "applet-backend-custom.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static void upload (const gchar *cFilePath)
{
	g_return_if_fail (myConfig.cCustomScripts[myData.iCurrentFileType] != NULL);
	
	// On lance la commande d'upload.
	gchar *cCommand = g_strdup_printf ("%s '%s'", myConfig.cCustomScripts[myData.iCurrentFileType], cFilePath);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		return ;
	}
	
	if (cResult[strlen(cResult)-1] == '\r')
		cResult[strlen(cResult)-1] = '\0';
	if (cResult[strlen(cResult)-1] == '\n')
		cResult[strlen(cResult)-1] = '\0';
	
	// On prend la derniere ligne, au cas ou le script aurait blablate sur la sortie.
	gchar *str = strrchr (cResult, '\n');
	if (str != NULL)
		str ++;
	else
		str = cResult;
	
	if (! cairo_dock_string_is_adress (str))
		cd_warning ("this adress (%s) seems not valid !\nThe output was : '%s'", str, cResult);
	
	// Enfin on remplit la memoire partagee avec nos URLs.
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = g_strdup (str);
	g_free (cResult);
}


void cd_dnd2share_register_custom_backends (void)
{
	CDFileType t;
	for (t = 0; t < CD_NB_FILE_TYPES; t ++)
	{
		cd_dnd2share_register_new_backend (t,
			"custom",
			NB_URLS,
			s_UrlLabels,
			0,
			upload);
	}
}

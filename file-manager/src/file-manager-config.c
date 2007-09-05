/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-config.h"



void file_manager_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *fconf = g_key_file_new ();
	
	bFlushConfFileNeeded = cairo_dock_read_header_applet_conf_file (fconf, iWidth, iHeight, cName);
	
	
	g_print ("now I will read the rest of my conf file\n");
	
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_write_keys_to_file (fconf, cConfFilePath);
	}
}


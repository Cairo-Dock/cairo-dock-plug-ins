/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-struct.h"
#include "file-manager-config.h"


static gchar *s_tSortTypes[3+1] = {"Name", "Size", "Date", NULL};
static gchar *s_tBackendNames[3+1] = {"Auto", "Gnome", "KDE", NULL};

extern FileManagerSortType my_fm_iSortType;
extern gboolean my_fm_bShowVolumes;
extern gboolean my_fm_bShowNetwork;
extern CairoDockDesktopEnv my_fm_iDesktopEnv;


void file_manager_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gchar **cIconName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	my_fm_bShowVolumes = cairo_dock_get_boolean_key_value (pKeyFile, "ICON", "show volumes", &bFlushConfFileNeeded, FALSE);
	my_fm_bShowNetwork = cairo_dock_get_boolean_key_value (pKeyFile, "ICON", "show network", &bFlushConfFileNeeded, FALSE);
	
	*cIconName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "icon", &bFlushConfFileNeeded, "gnome-background-image.png");
	
	
	gchar *cSortType = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "sort type", &bFlushConfFileNeeded, s_tSortTypes[0]);
	my_fm_iSortType = cairo_dock_get_number_from_name (cSortType, s_tSortTypes);
	g_free (cSortType);
	
	gchar *cVFSBackend = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "force vfs", &bFlushConfFileNeeded, s_tBackendNames[0]);
	my_fm_iDesktopEnv = cairo_dock_get_number_from_name (cVFSBackend, s_tBackendNames);
	g_free (cVFSBackend);
	
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	}
	g_key_file_free (pKeyFile);
}


/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-struct.h"
#include "file-manager-config.h"


extern FileManagerSortType my_fm_iSortType;
extern gboolean my_fm_bShowVolumes;
extern gboolean my_fm_bShowNetwork;
extern CairoDockDesktopEnv my_fm_iDesktopEnv;


void file_manager_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gchar **cIconName)
{
	GError *erreur = NULL;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	//GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	*iWidth = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "width", &bFlushConfFileNeeded, 48, NULL, NULL);
	*iHeight = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "height", &bFlushConfFileNeeded, 48, NULL, NULL);
	*cName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "name", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	my_fm_bShowVolumes = cairo_dock_get_boolean_key_value (pKeyFile, "ICON", "show volumes", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	my_fm_bShowNetwork = cairo_dock_get_boolean_key_value (pKeyFile, "ICON", "show network", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	*cIconName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "icon", &bFlushConfFileNeeded, "gnome-background-image.png", NULL, NULL);
	
	
	my_fm_iSortType = cairo_dock_get_integer_key_value (pKeyFile, "MODULE", "sort type", &bFlushConfFileNeeded, FILE_MANAGER_SORT_BY_NAME, NULL, NULL);
	if (my_fm_iSortType < 0 || my_fm_iSortType >= FILE_MANAGER_NB_SORTS)
		my_fm_iSortType = FILE_MANAGER_SORT_BY_NAME;
	
	my_fm_iDesktopEnv = cairo_dock_get_integer_key_value (pKeyFile, "MODULE", "force_vfs", &bFlushConfFileNeeded, 0, NULL, NULL);  // 0 <=> automatique.
	if (my_fm_iDesktopEnv == 0)
		my_fm_iDesktopEnv = cairo_dock_guess_environment ();
	
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, MY_APPLET_VERSION);
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);
	
	g_key_file_free (pKeyFile);
}



#include <cairo-dock.h>

#include "logout-config.h"


void cd_logout_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gchar **cIconName)
{
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	
	*cIconName = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "main icon", &bFlushConfFileNeeded, "gnome-logout");
	
	
	if (bFlushConfFileNeeded)
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	
	g_key_file_free (pKeyFile);
}

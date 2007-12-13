
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "dustbin-struct.h"
#include "dustbin-draw.h"
#include "dustbin-config.h"

extern double my_dustbin_fCheckInterval;
extern gchar **my_dustbin_cTrashDirectoryList;
extern cairo_surface_t *my_dustbin_pEmptyBinSurface;
extern cairo_surface_t *my_dustbin_pFullBinSurface;
extern GHashTable *my_dustbin_pThemeTable;
extern gchar *my_dustbin_cBrowser;


void cd_dustbin_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gchar **cThemeName)
{
	GError *erreur = NULL;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	*cThemeName = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "theme", &bFlushConfFileNeeded, "default");
	
	my_dustbin_fCheckInterval = cairo_dock_get_double_key_value (pKeyFile, "MODULE", "check interval", &bFlushConfFileNeeded, 2.);
	
	my_dustbin_cBrowser = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "file browser", &bFlushConfFileNeeded, "xdg-open");
	
	gsize length = 0;
	gchar *cDefaultTrashDir = g_strdup_printf ("%s/.Trash", getenv ("HOME"));
	my_dustbin_cTrashDirectoryList = cairo_dock_get_string_list_key_value (pKeyFile, "MODULE", "trash directories", &bFlushConfFileNeeded, &length, cDefaultTrashDir);
	g_free (cDefaultTrashDir);
	
	
	int i = 0;
	if (my_dustbin_cTrashDirectoryList != NULL)
	{
		gchar *cCompletePath;
		while (my_dustbin_cTrashDirectoryList[i] != NULL)
		{
			if (*my_dustbin_cTrashDirectoryList[i] == '~')
			{
				cCompletePath = g_strdup_printf ("%s%s", getenv ("HOME"), my_dustbin_cTrashDirectoryList[i]+1);
				g_free (my_dustbin_cTrashDirectoryList[i]);
				my_dustbin_cTrashDirectoryList[i] = cCompletePath;
			}
			i ++;
		}
	}
	
	
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);
	
	g_key_file_free (pKeyFile);
}


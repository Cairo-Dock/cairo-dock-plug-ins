
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


void cd_dustbin_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gchar **cThemeName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	GKeyFile *fconf = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (fconf != NULL);
	
	*cThemeName = g_key_file_get_string (fconf, "MODULE", "theme", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		*cThemeName = g_strdup ("default");  // valeur par defaut.
		g_key_file_set_string (fconf, "MODULE", "theme", *cThemeName);
		bFlushConfFileNeeded = TRUE;
	}
	if (*cThemeName != NULL && strcmp (*cThemeName, "") == 0)
	{
		g_free (*cThemeName);
		*cThemeName = NULL;
	}
	
	my_dustbin_fCheckInterval = g_key_file_get_double (fconf, "MODULE", "check interval", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_dustbin_fCheckInterval = FALSE;  // valeur par defaut.
		g_key_file_set_double (fconf, "MODULE", "check interval", my_dustbin_fCheckInterval);
		bFlushConfFileNeeded = TRUE;
	}
	
	gsize length = 0;
	my_dustbin_cTrashDirectoryList = g_key_file_get_string_list (fconf, "MODULE", "trash directories", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_dustbin_cTrashDirectoryList = g_new0 (gchar *, 2);
		my_dustbin_cTrashDirectoryList[0] = g_strdup_printf ("%s/.Trash", getenv ("HOME"));  // valeur par defaut.
		g_key_file_set_string (fconf, "MODULE", "trash directories", my_dustbin_cTrashDirectoryList[0]);
		bFlushConfFileNeeded = TRUE;
	}
	if (my_dustbin_cTrashDirectoryList != NULL && my_dustbin_cTrashDirectoryList[0] != NULL && strcmp (my_dustbin_cTrashDirectoryList[0], "") == 0)
	{
		g_strfreev (my_dustbin_cTrashDirectoryList);
		my_dustbin_cTrashDirectoryList = NULL;
	}
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
	{
		cairo_dock_write_keys_to_file (fconf, cConfFilePath);
	}
	
	g_key_file_free (fconf);
}


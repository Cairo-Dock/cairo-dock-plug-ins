/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "cd-clock-struct.h"
#include "cd-clock-draw.h"
#include "cd-clock-config.h"

extern gboolean my_bShowDate;
extern gboolean my_bShowSeconds;
extern gboolean my_bOldStyle;
extern gboolean my_b24Mode;
extern GHashTable *my_pThemeTable;
extern cairo_t *my_pCairoContext;
extern Icon *my_pIcon;

extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];
extern char my_cFileNames[CLOCK_ELEMENTS][30];
extern cairo_surface_t *g_pBackgroundSurface;
extern cairo_surface_t *g_pForegroundSurface;


void cd_clock_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *fconf = g_key_file_new ();
	
	g_key_file_load_from_file (fconf, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	bFlushConfFileNeeded = cairo_dock_read_header_applet_conf_file (fconf, iWidth, iHeight, cName);
	
	
	my_bShowDate = g_key_file_get_boolean (fconf, "MODULE", "show date", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_bShowDate = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "MODULE", "show date", my_bShowDate);
		bFlushConfFileNeeded = TRUE;
	}
	
	my_bShowSeconds = g_key_file_get_boolean (fconf, "MODULE", "show seconds", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_bShowSeconds = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "MODULE", "show seconds", my_bShowSeconds);
		bFlushConfFileNeeded = TRUE;
	}
	
	my_b24Mode = g_key_file_get_boolean (fconf, "MODULE", "24h mode", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_b24Mode = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "MODULE", "24h mode", my_b24Mode);
		bFlushConfFileNeeded = TRUE;
	}
	
	my_bOldStyle = g_key_file_get_boolean (fconf, "MODULE", "old fashion style", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		my_bOldStyle = FALSE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "MODULE", "old fashion style", my_bOldStyle);
		bFlushConfFileNeeded = TRUE;
	}
	
	gchar *cThemeName = g_key_file_get_string (fconf, "MODULE", "theme", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		cThemeName = g_strdup ("default");  // valeur par defaut.
		g_key_file_set_string (fconf, "MODULE", "theme", cThemeName);
		bFlushConfFileNeeded = TRUE;
	}
	if (cThemeName != NULL && strcmp (cThemeName, "") == 0)
	{
		g_free (cThemeName);
		cThemeName = NULL;
	}
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_write_keys_to_file (fconf, cConfFilePath);
	}
	
	//\_______________ On charge le theme choisi.
	if (cThemeName != NULL)
	{
		gchar *cThemePath = g_hash_table_lookup (my_pThemeTable, cThemeName);
		if (cThemePath == NULL)
			cThemePath = g_hash_table_lookup (my_pThemeTable, "default");
		g_return_if_fail (cThemePath != NULL);
		gchar *cElementPath;
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			cElementPath = g_strdup_printf ("%s/%s", cThemePath, my_cFileNames[i]);
			
			my_pSvgHandles[i] = rsvg_handle_new_from_file (cElementPath, NULL);
			//g_print (" + %s\n", cElementPath);
			g_free (cElementPath);
		}
	}
	
	g_key_file_free (fconf);
}


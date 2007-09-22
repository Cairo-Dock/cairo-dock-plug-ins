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

extern RsvgDimensionData my_DimensionData;
extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];
extern char my_cFileNames[CLOCK_ELEMENTS][30];


void cd_clock_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	my_bShowDate = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "show date", &bFlushConfFileNeeded, TRUE);
	
	my_bShowSeconds = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "show seconds", &bFlushConfFileNeeded, TRUE);
	
	my_b24Mode = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "24h mode", &bFlushConfFileNeeded, TRUE);
	
	my_bOldStyle = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "old fashion style", &bFlushConfFileNeeded, FALSE);
	
	gchar *cThemeName = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "theme", &bFlushConfFileNeeded, "default");
	
	
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
		rsvg_handle_get_dimensions (my_pSvgHandles[CLOCK_DROP_SHADOW], &my_DimensionData);
	}
	else
	{
		my_DimensionData.width = 48;  // valeur par defaut si aucun theme.
		my_DimensionData.height = 48;
	}
	
	
	if (bFlushConfFileNeeded)
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	
	g_key_file_free (pKeyFile);
}
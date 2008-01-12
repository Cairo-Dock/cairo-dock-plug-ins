
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"


extern double my_fCheckInterval;
extern gchar **my_cTrashDirectoryList;
extern cairo_surface_t *my_pEmptyBinSurface;
extern cairo_surface_t *my_pFullBinSurface;
extern GHashTable *my_pThemeTable;
//extern gchar *my_theme;
extern gchar *my_cThemePath;
extern gchar *my_cDefaultBrowser;
extern gchar *my_cEmptyUserImage;
extern gchar *my_cFullUserImage;
extern gboolean my_bDisplayNbTrashes;

CD_APPLET_CONFIG_BEGIN ("Poubelle", NULL)
	my_fCheckInterval = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("MODULE", "check interval", 2.);
	
	my_cDefaultBrowser = CD_CONFIG_GET_STRING_WITH_DEFAULT ("MODULE", "file browser", "xdg-open");
	
	//\_______________ On recupere la liste des repertoires faisant office de poubelle.
	gsize length = 0;
	gchar **cAdditionnalDirectoriesList = CD_CONFIG_GET_STRING_LIST ("MODULE", "additionnal directories", &length);
	
	my_cTrashDirectoryList = g_new0 (gchar *, length + 2);  // + 2 pour le repertoire par defaut et le NULL final.
	my_cTrashDirectoryList[0] = cairo_dock_fm_get_trash_path (g_getenv ("HOME"));
	
	int i = 0;
	if (cAdditionnalDirectoriesList != NULL)
	{
		gchar *cCompletePath;
		while (cAdditionnalDirectoriesList[i] != NULL)
		{
			if (*cAdditionnalDirectoriesList[i] == '~')
				my_cTrashDirectoryList[i+1] = g_strdup_printf ("%s%s", getenv ("HOME"), cAdditionnalDirectoriesList[i]+1);
			else
				my_cTrashDirectoryList[i+1] = g_strdup (cAdditionnalDirectoriesList[i]);
			i ++;
		}
		g_strfreev (cAdditionnalDirectoriesList);
	}
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	my_cThemePath = CD_CONFIG_GET_THEME_PATH ("MODULE", "theme", "themes", "Gion");
	
	my_cEmptyUserImage = CD_CONFIG_GET_STRING ("MODULE", "empty image");
	my_cFullUserImage = CD_CONFIG_GET_STRING ("MODULE", "full image");
	
	my_bDisplayNbTrashes = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("MODULE", "display nb trashes", TRUE);
	
CD_APPLET_CONFIG_END

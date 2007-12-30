
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
extern gchar *my_theme;
extern gchar *my_cBrowser;


CD_APPLET_CONFIG_BEGIN ("Poubelle", NULL)
	my_theme = CD_CONFIG_GET_STRING_WITH_DEFAULT ("MODULE", "theme", "default");
	
	my_fCheckInterval = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("MODULE", "check interval", 2.);
	
	my_cBrowser = CD_CONFIG_GET_STRING_WITH_DEFAULT ("MODULE", "file browser", "xdg-open");
	
	//\_______________ On recupere la liste des repertoires faisant office de poubelle.
	gsize length = 0;
	gchar *cDefaultTrashDir = g_strdup_printf ("%s/.Trash", getenv ("HOME"));
	my_cTrashDirectoryList = CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT ("MODULE", "trash directories", &length, cDefaultTrashDir);
	g_free (cDefaultTrashDir);
	int i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		gchar *cCompletePath;
		while (my_cTrashDirectoryList[i] != NULL)
		{
			if (*my_cTrashDirectoryList[i] == '~')
			{
				cCompletePath = g_strdup_printf ("%s%s", getenv ("HOME"), my_cTrashDirectoryList[i]+1);
				g_free (my_cTrashDirectoryList[i]);
				my_cTrashDirectoryList[i] = cCompletePath;
			}
			i ++;
		}
	}
	
	//\_______________ On charge la liste des themes disponibles.
	gchar *cThemesDir = g_strdup_printf ("%s/themes", MY_APPLET_SHARE_DATA_DIR);
	my_pThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
CD_APPLET_CONFIG_END

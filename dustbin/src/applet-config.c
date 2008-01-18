/**********************************************************************************

This file is a part of the cairo-dock project, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"


extern double my_fCheckInterval;
extern gchar **my_cAdditionnalDirectoriesList;
extern cairo_surface_t *my_pEmptyBinSurface;
extern cairo_surface_t *my_pFullBinSurface;
extern GHashTable *my_pThemeTable;
//extern gchar *my_theme;
extern gchar *my_cThemePath;
extern gchar *my_cDefaultBrowser;
extern gchar *my_cEmptyUserImage;
extern gchar *my_cFullUserImage;
extern int my_iQuickInfoType;
extern int my_iSizeLimit, my_iGlobalSizeLimit;


CD_APPLET_CONFIG_BEGIN ("Corbeille", NULL)
	my_fCheckInterval = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("MODULE", "check interval", 2.);
	
	my_cDefaultBrowser = CD_CONFIG_GET_STRING_WITH_DEFAULT ("MODULE", "file browser", "xdg-open");
	
	//\_______________ On recupere la liste des repertoires faisant office de poubelle.
	gsize length = 0;
	my_cAdditionnalDirectoriesList = CD_CONFIG_GET_STRING_LIST ("MODULE", "additionnal directories", &length);
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	my_cThemePath = CD_CONFIG_GET_THEME_PATH ("MODULE", "theme", "themes", "Gion");
	
	my_cEmptyUserImage = CD_CONFIG_GET_STRING ("MODULE", "empty image");
	my_cFullUserImage = CD_CONFIG_GET_STRING ("MODULE", "full image");
	
	my_iSizeLimit = CD_CONFIG_GET_INTEGER ("MODULE", "size limit") << 20;
	my_iGlobalSizeLimit = CD_CONFIG_GET_INTEGER ("MODULE", "global size limit") << 20;
	my_iQuickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("MODULE", "quick info", CD_DUSTBIN_INFO_NB_TRASHES);
CD_APPLET_CONFIG_END

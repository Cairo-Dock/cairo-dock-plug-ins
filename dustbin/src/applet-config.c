/**********************************************************************************

This file is a part of the cairo-dock project, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"


extern AppletConfig myConfig;


CD_APPLET_CONFIG_BEGIN ("Corbeille", NULL)
	myConfig.fCheckInterval = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Module", "check interval", 2.);
	
	myConfig.cDefaultBrowser = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Module", "file browser", "xdg-open");
	
	//\_______________ On recupere la liste des repertoires faisant office de poubelle.
	gsize length = 0;
	myConfig.cAdditionnalDirectoriesList = CD_CONFIG_GET_STRING_LIST ("Module", "additionnal directories", &length);
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Module", "theme", "themes", "Gion");
	
	myConfig.cEmptyUserImage = CD_CONFIG_GET_STRING ("Module", "empty image");
	myConfig.cFullUserImage = CD_CONFIG_GET_STRING ("Module", "full image");
	
	myConfig.iSizeLimit = CD_CONFIG_GET_INTEGER ("Module", "size limit") << 20;
	myConfig.iGlobalSizeLimit = CD_CONFIG_GET_INTEGER ("Module", "global size limit") << 20;
	myConfig.iQuickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Module", "quick info", CD_DUSTBIN_INFO_NB_TRASHES);
CD_APPLET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	g_atomic_int_set (&myConfig.iQuickInfoValue, 0);
	myConfig.iNbTrashes = 0, myConfig.iNbFiles = 0, myConfig.iSize = 0;
	
	g_free (myConfig.pTrashState);
	myConfig.pTrashState = NULL;
	
	g_free (myConfig.cThemePath);
	myConfig.cThemePath = NULL;
	
	if (myConfig.pEmptyBinSurface != NULL)
		cairo_surface_destroy (myConfig.pEmptyBinSurface);
	myConfig.pEmptyBinSurface = NULL;
	if (myConfig.pFullBinSurface != NULL)
		cairo_surface_destroy (myConfig.pFullBinSurface);
	myConfig.pFullBinSurface = NULL;
	
	g_free (myConfig.cDefaultBrowser);
	myConfig.cDefaultBrowser = NULL;
	
	g_free (myConfig.cEmptyUserImage);
	myConfig.cEmptyUserImage = NULL;
	g_free (myConfig.cFullUserImage);
	myConfig.cFullUserImage = NULL;
	g_free (myConfig.cDialogIconPath);
CD_APPLET_RESET_DATA_END

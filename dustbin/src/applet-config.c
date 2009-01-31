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
#include "applet-trashes-manager.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_______________ On recupere la liste des repertoires faisant office de poubelle.
	gsize length = 0;
	myConfig.cAdditionnalDirectoriesList = CD_CONFIG_GET_STRING_LIST ("Module", "additionnal directories", &length);
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Module", "theme", "themes", "Gion");
	
	myConfig.cEmptyUserImage = CD_CONFIG_GET_STRING ("Module", "empty image");
	myConfig.cFullUserImage = CD_CONFIG_GET_STRING ("Module", "full image");
	
	myConfig.iSizeLimit = CD_CONFIG_GET_INTEGER ("Module", "size limit") << 20;  // en Mo.
	myConfig.iGlobalSizeLimit = CD_CONFIG_GET_INTEGER ("Module", "global size limit") << 20;  // en Mo.
	myConfig.iQuickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Module", "quick info", CD_DUSTBIN_INFO_NB_TRASHES);
	
	myConfig.fCheckInterval = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Module", "check interval", 2.);
	
	myConfig.cDefaultBrowser = CD_CONFIG_GET_STRING ("Module", "alternative file browser");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_strfreev (myConfig.cAdditionnalDirectoriesList);
	
	g_free (myConfig.cThemePath);
	g_free (myConfig.cEmptyUserImage);
	g_free (myConfig.cFullUserImage);
	
	g_free (myConfig.cDefaultBrowser);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	g_atomic_int_set (&myData.iQuickInfoValue, 0);
	
	if (myData.pEmptyBinSurface != NULL)
	{
		cairo_surface_destroy (myData.pEmptyBinSurface);
	}
	if (myData.pFullBinSurface != NULL)
	{
		cairo_surface_destroy (myData.pFullBinSurface);
	}
	
	g_free (myData.cDialogIconPath);
	
	cd_dustbin_remove_all_dustbins ();
CD_APPLET_RESET_DATA_END

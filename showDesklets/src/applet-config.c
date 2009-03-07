/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.bShowWidgetLayerDesklet = CD_CONFIG_GET_BOOLEAN ("Configuration", "show widget layer");
	
	gchar *cShowImage = CD_CONFIG_GET_STRING ("Icon", "show image");
	myConfig.cShowImage = (cShowImage != NULL ? cairo_dock_generate_file_path (cShowImage) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/show-desklets.svg", NULL));
	g_free (cShowImage);
	
	gchar *cHideImage = CD_CONFIG_GET_STRING ("Icon", "hide image");
	myConfig.cHideImage = (cHideImage != NULL ? cairo_dock_generate_file_path (cHideImage) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/hide-desklets.svg", NULL));
	g_free (cHideImage);
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Shift><Ctrl>F4");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cShowImage);
	g_free (myConfig.cHideImage);
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull);
	g_free (myConfig.cShortcut);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

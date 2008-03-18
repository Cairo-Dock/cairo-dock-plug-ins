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

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.bShowWidgetLayerDesklet = CD_CONFIG_GET_BOOLEAN ("Configuration", "show widget layer");
	
	gchar *cShowImage = CD_CONFIG_GET_STRING ("Icon", "show image");
	myConfig.cShowImage = (cShowImage != NULL ? cairo_dock_generate_file_path (cShowImage) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/show-desklets.svg", NULL));
	g_free (cShowImage);
	
	gchar *cHideImage = CD_CONFIG_GET_STRING ("Icon", "hide image");
	myConfig.cHideImage = (cHideImage != NULL ? cairo_dock_generate_file_path (cHideImage) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/hide-desklets.svg", NULL));
	g_free (cHideImage);
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Shift><Ctrl>F4");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.cShowImage);
	myConfig.cShowImage = NULL;
	g_free (myConfig.cHideImage);
	myConfig.cHideImage = NULL;
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull);
	g_free (myConfig.cShortcut);
	myConfig.cShortcut = NULL;
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	memset (&myData, 0, sizeof (AppletData));
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-menu.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bHasIcons = CD_CONFIG_GET_BOOLEAN ("Configuration", "has icons");
	myConfig.cMenuShortkey = CD_CONFIG_GET_STRING ("Configuration", "menu shortkey");
	myConfig.cDirPath = CD_CONFIG_GET_STRING ("Configuration", "dir path");
	myConfig.bFoldersFirst = CD_CONFIG_GET_BOOLEAN ("Configuration", "folders first");
	myConfig.bCaseUnsensitive = CD_CONFIG_GET_BOOLEAN ("Configuration", "case unsensitive");
	myConfig.bShowHiddenFiles = CD_CONFIG_GET_BOOLEAN ("Configuration", "show hidden");
	
	// On gere les chemins relatifs.
	if (myConfig.cDirPath != NULL && *myConfig.cDirPath == '~')
	{
		gchar *tmp = myConfig.cDirPath;
		myConfig.cDirPath = g_strdup_printf ("%s%s", g_getenv ("HOME"), myConfig.cDirPath+1);
		g_free (tmp);
	}
	else if (myConfig.cDirPath != NULL && *myConfig.cDirPath != '/')
	{
		gchar *tmp = myConfig.cDirPath;
		myConfig.cDirPath = g_strdup_printf ("%s/%s", g_getenv ("HOME"), myConfig.cDirPath);
		g_free (tmp);
	}
	
	if (myConfig.cDirPath == NULL || ! g_file_test (myConfig.cDirPath, G_FILE_TEST_IS_DIR))
	{
		cd_warning ("Quick Browser : this path (%s) is not a valid folder !\n We'll use your home instead.", myConfig.cDirPath);
		g_free (myConfig.cDirPath);
		myConfig.cDirPath = g_strdup (g_getenv ("HOME"));
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cDirPath);
	if (myConfig.cMenuShortkey)
		cd_keybinder_unbind (myConfig.cMenuShortkey, (CDBindkeyHandler) cd_quick_browser_on_shortkey_menu);
	g_free (myConfig.cMenuShortkey);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_quick_browser_destroy_menu (myApplet);
CD_APPLET_RESET_DATA_END

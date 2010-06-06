/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	myConfig.iNbSubItemsAtOnce = CD_CONFIG_GET_INTEGER ("Configuration", "granularity");
	if (myConfig.iNbSubItemsAtOnce < 1)
		myConfig.iNbSubItemsAtOnce = 1;
	int iIconSize = CD_CONFIG_GET_INTEGER ("Configuration", "icon size");
	switch (iIconSize)
	{
		case 0:
			myConfig.iIconSize = 16;
		break ;
		case 1:
			myConfig.iIconSize = 24;
		break ;
		case 2:
			myConfig.iIconSize = 32;
		break ;
	}
	
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
		cd_debug ("Quick Browser : this path (%s) is not a valid folder\n We'll use the 'home' instead.", myConfig.cDirPath);
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

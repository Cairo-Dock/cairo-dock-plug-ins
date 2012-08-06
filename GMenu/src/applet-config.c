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
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bShowRecent = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show recent", TRUE);
	myConfig.cMenuShortkey = CD_CONFIG_GET_STRING ("Configuration", "menu shortkey");
	myConfig.cQuickLaunchShortkey = CD_CONFIG_GET_STRING ("Configuration", "quick launch shortkey");
	myConfig.cConfigureMenuCommand = CD_CONFIG_GET_STRING ("Configuration", "config menu");
	myConfig.iNbRecentItems = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb recent", 10);
	myConfig.iShowQuit = CD_CONFIG_GET_INTEGER ("Configuration", "show quit");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cConfigureMenuCommand);
	g_free (myConfig.cMenuShortkey);
	g_free (myConfig.cQuickLaunchShortkey);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.pMenu)
		gtk_widget_destroy (myData.pMenu);  // detruit aussi pRecentMenuItem.
	
	if (myData.dir_hash)
		g_hash_table_destroy (myData.dir_hash);
	
	GList *l;
	for (l = myData.possible_executables; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.possible_executables);
	
	for (l = myData.completion_items; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.completion_items);
	
	if (myData.completion)
		g_completion_free (myData.completion);
	
	if (!cairo_dock_dialog_unreference (myData.pQuickLaunchDialog))
		cairo_dock_dialog_unreference (myData.pQuickLaunchDialog);
CD_APPLET_RESET_DATA_END

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

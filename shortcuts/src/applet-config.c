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
#include "applet-disk-usage.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	CD_CONFIG_RENAME_GROUP ("Module", "Configuration");
	myConfig.bListDrives = CD_CONFIG_GET_BOOLEAN ("Configuration", "list drives");
	myConfig.bListNetwork = CD_CONFIG_GET_BOOLEAN ("Configuration", "list network");
	myConfig.bListBookmarks = CD_CONFIG_GET_BOOLEAN ("Configuration", "list bookmarks");
	///myConfig.bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Configuration", "use separator");
	
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "disk usage", CD_SHOW_USED_SPACE_PERCENT);
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 10);
	myConfig.bDrawBar = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "draw bar", TRUE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.iDeskletRendererType = CD_CONFIG_GET_INTEGER ("Configuration", "desklet renderer");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
CD_APPLET_RESET_CONFIG_END


void cd_shortcuts_reset_all_datas (GldiModuleInstance *myApplet)
{
	cd_shortcuts_free_disk_periodic_task (myApplet);
	
	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	if (myData.cDisksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cDisksURI, FALSE, NULL);
		g_free (myData.cDisksURI);
		myData.cDisksURI = NULL;
	}
	if (myData.cNetworkURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cNetworkURI, FALSE, NULL);
		g_free (myData.cNetworkURI);
		myData.cNetworkURI = NULL;
	}
	if (myData.cBookmarksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cBookmarksURI, FALSE, NULL);
		g_free (myData.cBookmarksURI);
		myData.cBookmarksURI = NULL;
	}
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
}

CD_APPLET_RESET_DATA_BEGIN
	cd_shortcuts_reset_all_datas (myApplet);
	g_free (myData.cLastCreatedUri);
	g_free (myData.cLastDeletedUri);
CD_APPLET_RESET_DATA_END

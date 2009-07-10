/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-disk-usage.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bListDrives = CD_CONFIG_GET_BOOLEAN ("Module", "list drives");
	myConfig.bListNetwork = CD_CONFIG_GET_BOOLEAN ("Module", "list network");
	myConfig.bListBookmarks = CD_CONFIG_GET_BOOLEAN ("Module", "list bookmarks");
	myConfig.bUseSeparator = CD_CONFIG_GET_BOOLEAN ("Module", "use separator");
	
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Module", "disk usage", CD_SHOW_USED_SPACE_PERCENT);
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Module", "check interval", 10);
	myConfig.bDrawBar = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Module", "draw bar", TRUE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Module", "renderer");
	myConfig.iDeskletRendererType = CD_CONFIG_GET_INTEGER ("Module", "desklet renderer");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
CD_APPLET_RESET_CONFIG_END


void cd_shortcuts_reset_all_datas (CairoDockModuleInstance *myApplet)
{
	cd_shortcuts_stop_disk_periodic_task (myApplet);
	cairo_dock_free_task (myData.pTask);
	
	if (myData.cDisksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cDisksURI, FALSE, NULL);
		g_free (myData.cDisksURI);
	}
	if (myData.cNetworkURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cNetworkURI, FALSE, NULL);
		g_free (myData.cNetworkURI);
	}
	if (myData.cBookmarksURI != NULL)
	{
		cairo_dock_fm_remove_monitor_full (myData.cBookmarksURI, FALSE, NULL);
		g_free (myData.cBookmarksURI);
	}
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
	memset (myDataPtr, 0, sizeof (AppletData));
}

CD_APPLET_RESET_DATA_BEGIN
	cd_shortcuts_reset_all_datas (myApplet);
CD_APPLET_RESET_DATA_END

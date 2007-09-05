/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-struct.h"
#include "file-manager-load-directory.h"

extern FileManagerGetFileInfoFunc file_manager_get_file_info;
extern FileManagerListDirectoryFunc file_manager_list_directory;
extern FileManagerLaunchUriFunc file_manager_launch_uri;
extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;
extern FileManagerMountFunc file_manager_mount;
extern FileManagerUnmountFunc file_manager_unmount;
extern FileManagerAddMonitorFunc file_manager_add_monitor;


void file_manager_create_dock_from_directory (Icon *pIcon)
{
	CairoDock *pDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, pIcon->acName);
	cairo_dock_reference_dock (pDock);  // on le fait tout de suite pour avoir la bonne reference avant le 'load'.
	
	pDock->icons = file_manager_list_directory (pIcon->acCommand);
	
	cairo_dock_load_buffers_in_one_dock (pDock);
	
	pIcon->pSubDock = pDock;
	
	file_manager_add_monitor (pIcon);
}


void file_monitor_action_on_event (int iEventType, const gchar *cURI, Icon *pIcon)
{
	g_print ("%s (%d sur %s)\n", __func__, iEventType, cURI);
	
}

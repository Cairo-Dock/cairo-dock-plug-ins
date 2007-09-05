/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include <libgnomevfs/gnome-vfs.h>

#include "file-manager-vfs-gnome-volumes.h"


static Icon * file_manager_create_icon_from_volume (GnomeVFSVolume *pVolume)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = gnome_vfs_volume_get_display_name (pVolume);
	icon->acFileName = gnome_vfs_volume_get_icon (pVolume);
	icon->acCommand = gnome_vfs_volume_get_activation_uri (pVolume);  // gnome_vfs_volume_get_device_path (pVolume)
	return icon;
}

static Icon * file_manager_create_icon_from_drive (GnomeVFSDrive *pDrive)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = gnome_vfs_drive_get_display_name (pDrive);
	icon->acFileName = gnome_vfs_drive_get_icon (pDrive);
	icon->acCommand = gnome_vfs_drive_get_activation_uri (pDrive);  // gnome_vfs_drive_get_device_path (pDrive); 
	return icon;
}


GList *file_manager_list_volumes (void)
{
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	GList *pVolumesList = gnome_vfs_volume_monitor_get_mounted_volumes (pVolumeMonitor);
	
	Icon *icon;
	GList *pIconList = NULL;
	
	GnomeVFSVolume *pVolume;
	GList *pListElement;
	for (pListElement = pVolumesList; pListElement != NULL; pListElement = pListElement->next)
	{
		pVolume = GNOME_VFS_VOLUME (pListElement->data);
		
		if (gnome_vfs_volume_is_user_visible (pVolume))
		{
			icon = file_manager_create_icon_from_volume (pVolume);
			pIconList = g_list_prepend (pIconList, icon);
		}
		gnome_vfs_volume_unref (pVolume);
	}
	g_list_free (pVolumesList);
	return pIconList;
}


GList *file_manager_list_drives (void)
{
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	GList *pDrivesList = gnome_vfs_volume_monitor_get_connected_drives (pVolumeMonitor);
	
	Icon *icon;
	GList *pIconList = NULL;
	
	GList *pMountedVolumesList, *pSubListElement;
	GnomeVFSVolume *pMountedVolume;
	Icon *volume_icon;
	GnomeVFSDrive *pDrive;
	GList *pListElement;
	for (pListElement = pDrivesList; pListElement != NULL; pListElement = pListElement->next)
	{
		pDrive = GNOME_VFS_DRIVE (pListElement->data);
		
		if (gnome_vfs_drive_is_user_visible (pDrive))
		{
			icon = file_manager_create_icon_from_drive (pDrive);
			pIconList = g_list_prepend (pIconList, icon);
			
			icon->pSubDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, icon->acName);
			cairo_dock_reference_dock (icon->pSubDock);
			
			pMountedVolumesList = gnome_vfs_drive_get_mounted_volumes (pDrive);
			for (pSubListElement = pMountedVolumesList; pSubListElement != NULL; pSubListElement = pSubListElement->next)
			{
				pMountedVolume = GNOME_VFS_VOLUME (pSubListElement->data);
				
				if (gnome_vfs_volume_is_user_visible (pMountedVolume))
				{
					volume_icon = file_manager_create_icon_from_volume (pMountedVolume);
					icon->pSubDock->icons = g_list_prepend (icon->pSubDock->icons, volume_icon);
				}
				gnome_vfs_volume_unref (pMountedVolume);
			}
			g_list_free (pMountedVolumesList);
		}
		gnome_vfs_drive_unref (pDrive);
	}
	g_list_free (pDrivesList);
	return pIconList;
}


GList *file_pmanager_list_vfs_root (void)
{
	Icon *icon;
	GList *pIconList = NULL;
	
	
	
	return pIconList;
}






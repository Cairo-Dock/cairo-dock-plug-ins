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


#ifndef __APPLET_GNOME_VFS__
#define  __APPLET_GNOME_VFS__


#include <cairo-dock.h>


gboolean init_vfs_backend (void);
void stop_vfs_backend (void);


void vfs_backend_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType);


GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gboolean bListHiddenFiles, int iNbMaxFiles, gchar **cFullURI);


void vfs_backend_launch_uri (const gchar *cURI);


gchar * vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted);

gboolean vfs_backend_can_eject (const gchar *cURI);
gboolean vfs_backend_eject_drive (const gchar *cURI);


void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data);

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data);


void vfs_backend_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer data);
void vfs_backend_remove_monitor (const gchar *cURI);


gboolean vfs_backend_delete_file (const gchar *cURI, gboolean bNoTrash);

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName);

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI);

gboolean vfs_backend_create_file (const gchar *cURI, gboolean bDirectory);


void vfs_backend_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask);

void vfs_backend_empty_trash (void);

gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gchar **cFileInfoPath);

gchar *vfs_backend_get_desktop_path (void);

gsize vfs_backend_measure_directory (const gchar *cBaseURI, gint iCountType, gboolean bRecursive, gint *iCancel);


#endif

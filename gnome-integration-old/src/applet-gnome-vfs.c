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

// Some inspiration was taken from the "xdg" project.

#include <string.h>

#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>

#include "applet-gnome-vfs.h"

static GHashTable *s_fm_MonitorHandleTable = NULL;


static void _vfs_backend_free_monitor_data (gpointer *data)
{
	if (data != NULL)
	{
		GnomeVFSMonitorHandle *pHandle = data[2];
		gnome_vfs_monitor_cancel (pHandle);  // le GnomeVFSMonitorHandle est-il libere lors du gnome_vfs_monitor_cancel () ?
		g_free (data);
	}
}

gboolean init_vfs_backend (void)
{
	if (s_fm_MonitorHandleTable != NULL)
		g_hash_table_destroy (s_fm_MonitorHandleTable);
	
	s_fm_MonitorHandleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		(GDestroyNotify) _vfs_backend_free_monitor_data);
	
	return (gnome_vfs_init ());  // ne fait rien si gnome-vfs est deja initialise.
}

void stop_vfs_backend (void)
{
	if (s_fm_MonitorHandleTable != NULL)
	{
		g_hash_table_destroy (s_fm_MonitorHandleTable);
		s_fm_MonitorHandleTable = NULL;
	}
}



static gboolean file_manager_get_file_info_from_desktop_link (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID)
{
	cd_message ("%s (%s)", __func__, cBaseURI);
	GError *erreur = NULL;
	
	gchar *cFileData = NULL;
	int iFileSize = 0;
	if (gnome_vfs_read_entire_file (cBaseURI, &iFileSize, &cFileData) != GNOME_VFS_OK)
	{
		cd_warning ("Attention : couldn't read %s", cBaseURI);
		return FALSE;
	}
	//g_print (" => %s\n", cFileData);
	
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_data (pKeyFile,
		cFileData,
		-1,
		G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
		&erreur);
	g_free (cFileData);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	gchar *cType = g_key_file_get_value (pKeyFile, "Desktop Entry", "Type", NULL);
	//g_print ("  cType : %s\n", cType);
	if (strncmp (cType, "Link", 4) != 0 && strncmp (cType, "FSDevice", 8) != 0)
	{
		g_free(cType);
		g_key_file_free (pKeyFile);
		return FALSE;
	}
	g_free(cType);

	*cName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Name", NULL);
	
	*cURI = g_key_file_get_string (pKeyFile, "Desktop Entry", "URL", NULL);
	
	*cIconName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Icon", NULL);	
	
	*iVolumeID = g_key_file_get_integer (pKeyFile, "Desktop Entry", "X-Gnome-Drive", NULL);	
	
	*bIsDirectory = TRUE;
	
	g_key_file_free (pKeyFile);
	return TRUE;
}


void vfs_backend_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
{
	g_return_if_fail (cBaseURI != NULL);
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	GnomeVFSResult r;
	GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
	
	gchar *cFullURI;
	if (strncmp (cBaseURI, "x-nautilus-desktop://", 21) == 0)
	{
		gchar *cNautilusFile = g_strdup_printf ("computer://%s", cBaseURI+21);
		if (g_str_has_suffix (cBaseURI, ".volume"))
		{
			cNautilusFile[strlen(cNautilusFile)-7] = '\0';
			cFullURI = g_strdup_printf ("%s.drive", cNautilusFile);
			g_free (cNautilusFile);
			gchar **cSplit = g_strsplit (cFullURI, "%20", -1);
			if (cSplit != NULL && cSplit[0] != NULL)
			{
				int i = 0;
				g_free (cFullURI);
				GString *sURI = g_string_new (cSplit[0]);
				for (i = 1; cSplit[i] != NULL; i ++)
				{
					cd_message ("%d) %s", i, cSplit[i]);
					g_string_append_printf (sURI, "%%2520%s", cSplit[i]);
				}
				cFullURI = g_string_free (sURI, FALSE);
			}
			g_strfreev (cSplit);
		}
	}
	else
		cFullURI = gnome_vfs_make_uri_from_input (cBaseURI);
	cd_message (" -> cFullURI : %s", cFullURI);
	
	GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
	
	r = gnome_vfs_get_file_info (cFullURI, info, infoOpts);
	if (r != GNOME_VFS_OK) 
	{
		cd_warning ("Attention : couldn't get file info for '%s'", cFullURI);
		g_free (cFullURI);
		gnome_vfs_file_info_unref (info);
		return ;
	}
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
		*fOrder = info->mtime;
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
		*fOrder = info->size;
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
		*fOrder = info->type;
	else
		*fOrder = 0;
	
	GnomeVFSFileInfoFields valid = info->valid_fields;
	
	const gchar *cMimeType = gnome_vfs_file_info_get_mime_type (info);
	cd_message ("  cMimeType : %s", cMimeType);
	if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (cMimeType, "application/x-desktop") == 0)
	{
		gnome_vfs_file_info_unref (info);
		file_manager_get_file_info_from_desktop_link (cFullURI, cName, cURI, cIconName, bIsDirectory, iVolumeID);
		*fOrder = 0;
		return ;
	}
	g_free (cFullURI);
	
	*cName = g_strdup (info->name);
	
	*cURI = g_strdup (cBaseURI);
	
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
		*bIsDirectory = (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
	else
		*bIsDirectory = FALSE;
	
	*cIconName = NULL;
	if (strncmp (cMimeType, "image", 5) == 0)
	{
		gchar *cHostname = NULL;
		GError *erreur = NULL;
		gchar *cFilePath = g_filename_from_uri (cBaseURI, &cHostname, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
		}
		else if (cHostname == NULL || strcmp (cHostname, "localhost") == 0)  // on ne recupere la vignette que sur les fichiers locaux.
		{
			*cIconName = g_strdup (cFilePath);
			cairo_dock_remove_html_spaces (*cIconName);
		}
		g_free (cHostname);
		g_free (cFilePath);
	}
	
	if (*cIconName == NULL)
	{
		GnomeIconLookupResultFlags iconLookupResultFlags;
		*cIconName = gnome_icon_lookup (gtk_icon_theme_get_default (),
			NULL,
			NULL /* const char *file_uri */,
			NULL,
			info,
			info->mime_type,
			GNOME_ICON_LOOKUP_FLAGS_NONE,  // GNOME_ICON_LOOKUP_FLAGS_ALLOW_SVG_AS_THEMSELVES
			&iconLookupResultFlags);
	}
	
	*iVolumeID = 0;
	
	gnome_vfs_file_info_unref (info);
}



GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gboolean bListHiddenFiles, int iNbMaxFiles, gchar **cFullURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	GList *pIconList = NULL;
	
	const gchar *cURI;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
		cURI = "computer://";
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
		cURI = "network://";
	else
		cURI = cBaseURI;
	
	*cFullURI = gnome_vfs_make_uri_from_input (cURI);  // pas franchement necessaire ...
	g_return_val_if_fail (*cFullURI != NULL, NULL);
	cd_message (" -> cFullURI : %s", *cFullURI);
	
	GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
	GnomeVFSDirectoryHandle *handle = NULL;
	GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
	GnomeVFSResult r = gnome_vfs_directory_open (&handle, *cFullURI, infoOpts);
	if (r!=GNOME_VFS_OK) 
	{
		return NULL;
	}
	
	GnomeVFSURI* dirUri = gnome_vfs_uri_new (*cFullURI);
	cd_message ("  dirUri : %s", dirUri->text);
	GnomeVFSURI* fileUri;
	gchar *cFileURI;
	int iNbFiles = 0;
	GnomeIconLookupResultFlags iconLookupResultFlags;
	Icon *icon;
	while(iNbFiles < iNbMaxFiles)
	{
		r = gnome_vfs_directory_read_next (handle, info);
		if (r == GNOME_VFS_ERROR_EOF)
			break;
		if (r != GNOME_VFS_OK) 
			continue ;
		
		if (strcmp (info->name, ".") != 0 && strcmp (info->name, "..") != 0 && (bListHiddenFiles || info->name[0] != '.'))
		{
			fileUri = gnome_vfs_uri_append_path (dirUri, info->name);
			cFileURI = gnome_vfs_uri_to_string (fileUri, GNOME_VFS_URI_HIDE_NONE);
			cd_message (" + cFileURI : %s", cFileURI);
			
			GnomeVFSFileInfoFields valid = info->valid_fields;
			if (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
			{
				if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
				{
					
				}
				else if (info->type == GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK)
				{
					
				}
			}
			
			icon = cairo_dock_create_dummy_launcher (NULL, NULL, NULL, NULL, 0);
			icon->cBaseURI = cFileURI;
			icon->iGroup = iNewIconsType;
			if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (info->mime_type, "application/x-desktop") == 0)
			{
				gboolean bIsDirectory = FALSE;
				file_manager_get_file_info_from_desktop_link (cFileURI, &icon->cName, &icon->cCommand, &icon->cFileName, &bIsDirectory, &icon->iVolumeID);
				cd_message ("  bIsDirectory : %d; iVolumeID : %d; cFileName : %s", bIsDirectory, icon->iVolumeID, icon->cFileName);
			}
			else
			{
				icon->cCommand = g_strdup (cFileURI);
				icon->cName = g_strdup (info->name);
				icon->cFileName = NULL;
				if (strncmp (info->mime_type, "image", 5) == 0)  // && strncmp (cFileURI, "file://", 7) == 0
				{
					gchar *cHostname = NULL;
					GError *erreur = NULL;
					gchar *cFilePath = g_filename_from_uri (cFileURI, &cHostname, &erreur);
					if (erreur != NULL)
					{
						g_error_free (erreur);
					}
					else if (cHostname == NULL || strcmp (cHostname, "localhost") == 0)  // on ne recupere la vignette que sur les fichiers locaux.
					{
						icon->cFileName = g_strdup (cFilePath);
						cairo_dock_remove_html_spaces (icon->cFileName);
					}
					g_free (cHostname);
					g_free (cFilePath);
				}
				if (icon->cFileName == NULL)
				{
					icon->cFileName = gnome_icon_lookup (gtk_icon_theme_get_default (),
						NULL,
						NULL, // file_uri.
						NULL,
						info,
						info->mime_type,
						GNOME_ICON_LOOKUP_FLAGS_NONE,
						&iconLookupResultFlags);
				}
			}
			if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE && (valid & GNOME_VFS_FILE_INFO_FIELDS_SIZE))
				icon->fOrder = info->size;
			else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE && (valid & GNOME_VFS_FILE_INFO_FIELDS_MTIME))
				icon->fOrder = info->mtime;
			else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE && (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE))
				icon->fOrder = info->type;
			pIconList = g_list_prepend (pIconList, icon);
			iNbFiles ++;
			
			gnome_vfs_uri_unref (fileUri);
		}
		gnome_vfs_file_info_clear (info);
	}
	gnome_vfs_uri_unref (dirUri);
	
	gnome_vfs_directory_close (handle);
	gnome_vfs_file_info_unref (info);
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
		pIconList = cairo_dock_sort_icons_by_name (pIconList);
	else
		pIconList = cairo_dock_sort_icons_by_order (pIconList);
	
	return pIconList;
}



void vfs_backend_launch_uri (const gchar *cURI)
{
	GnomeVFSResult r = gnome_vfs_url_show (cURI);
}


gchar *vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	cd_message ("%s (%s)", __func__, cURI);
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	gchar *cLocalPath = gnome_vfs_get_local_path_from_uri (cURI);
	cd_message (" cLocalPath : %s", cLocalPath);
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cLocalPath);
	g_free (cLocalPath);
	if (pVolume == NULL)
	{
		cd_warning ("Attention : no volum associated to %s", cURI);
		*bIsMounted = FALSE;
		return NULL;
	}
	else
	{
		gchar *cMountPointID = gnome_vfs_volume_get_activation_uri (pVolume);
		
		*bIsMounted = gnome_vfs_volume_is_mounted (pVolume);
		cd_message ("  bIsMounted <- %d", *bIsMounted);
		
		gnome_vfs_volume_unref (pVolume);
		return cMountPointID;
	}
}


static void _vfs_backend_mount_callback (gboolean succeeded, char *error, char *detailed_error, gpointer *data)
{
	cd_message ("%s (%d)", __func__, succeeded);
	if (! succeeded)
		cd_warning ("Attention : failed to mount (%s ; %s)", error, detailed_error);
	
	CairoDockFMMountCallback pCallback = data[0];
	
	pCallback (GPOINTER_TO_INT (data[1]), succeeded, data[2], data[3], data[4]);
	g_free (data[2]);
	g_free (data);
}

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data)
{
	g_return_if_fail (iVolumeID > 0);
	cd_message ("%s (ID:%d)", __func__, iVolumeID);
	
	///gchar *cLocalPath = gnome_vfs_get_local_path_from_uri (cURI);
	///g_print (" cLocalPath : %s\n", cLocalPath);
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	/**GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cLocalPath);
	g_free (cLocalPath);
	g_return_val_if_fail (pVolume != NULL, NULL);
	
	GnomeVFSDrive *pDrive = gnome_vfs_volume_get_drive (pVolume);*/
	GnomeVFSDrive *pDrive = gnome_vfs_volume_monitor_get_drive_by_id (pVolumeMonitor, iVolumeID);
	g_return_if_fail (pDrive != NULL);
	
	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (TRUE);
	data2[2] = gnome_vfs_drive_get_display_name (pDrive);
	data2[3] = g_strdup (cURI);
	data2[4] = user_data;
	gnome_vfs_drive_mount (pDrive, (GnomeVFSVolumeOpCallback)_vfs_backend_mount_callback, data2);
	
	///gnome_vfs_volume_unref (pVolume);
	gnome_vfs_drive_unref (pDrive);
}

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	gchar *cLocalPath = gnome_vfs_get_local_path_from_uri (cURI);
	cd_message (" cLocalPath : %s", cLocalPath);
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cLocalPath);
	g_free (cLocalPath);
	g_return_if_fail (pVolume != NULL);
	
	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (FALSE);
	data2[2] = gnome_vfs_volume_get_display_name (pVolume);
	data2[3] = g_strdup (cURI);
	data2[4] = user_data;
	gnome_vfs_volume_unmount (pVolume, (GnomeVFSVolumeOpCallback)_vfs_backend_mount_callback, data2);
	
	gnome_vfs_volume_unref (pVolume);
}



static void _vfs_backend_gnome_monitor_callback (GnomeVFSMonitorHandle *handle,
	const gchar *monitor_uri,
	const gchar *info_uri,
	GnomeVFSMonitorEventType event_type,
	gpointer *data)
{
	CairoDockFMMonitorCallback pCallback = data[0];
	gpointer user_data = data[1];
	cd_message ("%s (%d , data : %x)", __func__, event_type, user_data);
	
	CairoDockFMEventType iEventType;
	switch (event_type)
	{
		case GNOME_VFS_MONITOR_EVENT_CHANGED :
			iEventType = CAIRO_DOCK_FILE_MODIFIED;
		break;
		
		case GNOME_VFS_MONITOR_EVENT_DELETED :
			iEventType = CAIRO_DOCK_FILE_DELETED;
		break;
		
		case GNOME_VFS_MONITOR_EVENT_CREATED :
			iEventType = CAIRO_DOCK_FILE_CREATED;
		break;
		
		default :
		return ;
	}
	pCallback (iEventType, info_uri, user_data);
}


void vfs_backend_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer user_data)
{
	GnomeVFSMonitorHandle *pHandle = NULL;
	gpointer *data = g_new0 (gpointer, 3);
	data[0] = pCallback;
	data[1] = user_data;
	GnomeVFSResult r = gnome_vfs_monitor_add (&pHandle,
		cURI,
		(bDirectory ? GNOME_VFS_MONITOR_DIRECTORY : GNOME_VFS_MONITOR_FILE),
		(GnomeVFSMonitorCallback) _vfs_backend_gnome_monitor_callback,
		data);
	if (r != GNOME_VFS_OK)
	{
		cd_warning ("Attention : couldn't add monitor function to %s\n  I will not be able to receive events about this file", cURI);
		g_free (data);
	}
	else
	{
		cd_message (">>> moniteur ajoute sur %s (%x)", cURI, user_data);
		data[2] = pHandle;
		g_hash_table_insert (s_fm_MonitorHandleTable, g_strdup (cURI), data);
	}
}

void vfs_backend_remove_monitor (const gchar *cURI)
{
	if (cURI != NULL)
	{
		cd_message (">>> moniteur supprime sur %s", cURI);
		g_hash_table_remove (s_fm_MonitorHandleTable, cURI);
	}
}



gboolean vfs_backend_delete_file (const gchar *cURI)
{
	GnomeVFSResult r = gnome_vfs_unlink (cURI);
	return (r == GNOME_VFS_OK);
}

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	GnomeVFSURI *pVfsUri = gnome_vfs_uri_new (cOldURI);
	gchar *cPath = gnome_vfs_uri_extract_dirname (pVfsUri);
	gnome_vfs_uri_unref (pVfsUri);
	
	gchar *cNewURI = g_strdup_printf ("%s/%s", cPath, cNewName);
	g_free (cPath);
	cd_message (" %s -> %s", cOldURI, cNewURI);
	
	GnomeVFSResult r= gnome_vfs_move (cOldURI,
		cNewURI,
		FALSE);
	g_free (cNewURI);
	return (r == GNOME_VFS_OK);
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	cd_message (" %s -> %s", cURI, cDirectoryURI);
	
	GnomeVFSURI *pVfsUri = gnome_vfs_uri_new (cURI);
	g_return_val_if_fail (pVfsUri != NULL, FALSE);
	
	gchar *cFileName = gnome_vfs_uri_extract_short_name (pVfsUri);
	cd_message ("  pVfsUri : %s; cFileName : %s", pVfsUri->text, cFileName);
	
	GnomeVFSURI *pVfsDirUri = gnome_vfs_uri_new (cDirectoryURI);
	if (pVfsDirUri == NULL)
	{
		gnome_vfs_uri_unref (pVfsUri);
		g_free (cFileName);
		return FALSE;
	}
	cd_message ("  pVfsDirUri : %s", pVfsDirUri->text);
	
	GnomeVFSURI *pVfsNewUri = gnome_vfs_uri_append_file_name (pVfsDirUri, cFileName);
	cd_message ("  pVfsNewUri : %s", pVfsNewUri->text);
	
	GnomeVFSResult r = gnome_vfs_move_uri (pVfsUri,
		pVfsNewUri,
		FALSE);
	
	gnome_vfs_uri_unref (pVfsUri);
	gnome_vfs_uri_unref (pVfsDirUri);
	gnome_vfs_uri_unref (pVfsNewUri);
	g_free (cFileName);
	return (r == GNOME_VFS_OK);
}

void vfs_backend_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	GnomeVFSResult r;
	GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
	gchar *cFullURI = gnome_vfs_make_uri_from_input (cURI);
	cd_message ("  cFullURI : %s", cFullURI);
	
	GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
	
	r = gnome_vfs_get_file_info (cFullURI, info, infoOpts);
	if (r != GNOME_VFS_OK) 
	{
		cd_warning ("Attention : couldn't get file info for '%s'", cFullURI);
		g_free (cFullURI);
		gnome_vfs_file_info_unref (info);
		return ;
	}
	
	GnomeVFSFileInfoFields valid = info->valid_fields;
	
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
		*iSize = info->size;
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
		*iLastModificationTime = info->mtime;
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE)
		*cMimeType = g_strdup (info->mime_type);
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_IDS)
		*iUID = info->uid;
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_IDS)
		*iGID = info->gid;
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS)
		*iPermissionsMask = info->permissions;
	gnome_vfs_file_info_unref (info);
}


gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gchar **cFileInfoPath)
{
	cd_message ("%s (%s)", __func__, cNearURI);
	
	GnomeVFSURI *near_uri = gnome_vfs_uri_new (cNearURI);
	GnomeVFSURI *result = NULL;
	GnomeVFSResult r = gnome_vfs_find_directory (near_uri,
		GNOME_VFS_DIRECTORY_KIND_TRASH,
		&result,
		TRUE,  // le creer si n'existe pas.
		TRUE,
		7*8*8+5*8+5);
	gnome_vfs_uri_unref (near_uri);
	if (cFileInfoPath != NULL)
		*cFileInfoPath = NULL;
	if (r == GNOME_VFS_OK)
	{
		gchar *cTrashURI = g_strdup (result->text);
		gnome_vfs_uri_unref (result);
		return cTrashURI;
	}
	else
		return NULL;
}

gchar *vfs_backend_get_desktop_path (void)
{
	GnomeVFSURI *near_uri = gnome_vfs_uri_new ("file:///home");
	GnomeVFSURI *result = NULL;
	GnomeVFSResult r = gnome_vfs_find_directory (near_uri,
		GNOME_VFS_DIRECTORY_KIND_DESKTOP,
		&result,
		TRUE,
		TRUE,
		7*8*8+5*8+5);
	gnome_vfs_uri_unref (near_uri);
	if (r == GNOME_VFS_OK)
	{
		gchar *cDesktopURI = g_strdup (result->text);
		gnome_vfs_uri_unref (result);
		return cDesktopURI;
	}
	else
		return NULL;
}

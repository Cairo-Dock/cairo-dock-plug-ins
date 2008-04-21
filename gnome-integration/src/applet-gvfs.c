/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)
Some inspiration was taken from the "xdg" project.

******************************************************************************/
#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include "applet-gvfs.h"

static GHashTable *s_hMonitorHandleTable = NULL;


static void _vfs_backend_free_monitor_data (gpointer *data)
{
	if (data != NULL)
	{
		GFileMonitor *pHandle = data[2];
		g_file_monitor_cancel (pHandle);  // le GFileMonitor est-il libere lors du g_file_monitor_cancel () ?
		g_free (data);
	}
}

gboolean init_vfs_backend (void)
{
	if (s_hMonitorHandleTable != NULL)
		g_hash_table_destroy (s_hMonitorHandleTable);
	
	s_hMonitorHandleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		(GDestroyNotify) _vfs_backend_free_monitor_data);
	
	GVfs *vfs = g_vfs_get_default ();
	return (vfs != NULL && g_vfs_is_active (vfs));  // utile ?
}

void stop_vfs_backend (void)
{
	if (s_hMonitorHandleTable != NULL)
	{
		g_hash_table_destroy (s_hMonitorHandleTable);
		s_hMonitorHandleTable = NULL;
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
					cd_message ("%d) %s\n", i, cSplit[i]);
					g_string_append_printf (sURI, "%%2520%s", cSplit[i]);
				}
				cFullURI = sURI->str;
				g_string_free (sURI, FALSE);
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



GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gboolean bListHiddenFiles, gchar **cFullURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	const gchar *cURI;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
		cURI = "computer://";
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
		cURI = "network://";
	else
		cURI = cBaseURI;
	*cFullURI = g_strdup (cURI);  /// a voir pour les URI bizarres.
	
	GFile *pFile = g_file_new_for_uri (cURI);
	GError *erreur = NULL;
	gchar *cAttributes = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_SIZE, ",",
		G_FILE_ATTRIBUTE_TIME_MODIFIED, ",",
		G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
		G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN, ",",
		G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
		G_FILE_ATTRIBUTE_UNIX_RDEV, NULL);
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_free (cAttributes);
	
	GList *pIconList = NULL;
	Icon *icon;
	GFileInfo *pFileInfo;
	do
	{
		pFileInfo = g_file_enumerator_next_file (pFileEnum, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			if (pFileInfo == NULL)
				break ;
			
			gboolean bIsHidden = g_file_info_set_is_hidden (pFileInfo);
			if (bListHiddenFiles || ! bIsHidden)
			{
				const gchar *cName = g_file_info_get_name (pFileInfo);
				GIcon *pSystemIcon = g_file_info_get_icon (pFileInfo);
				const gchar *cMimeType = g_file_info_get_content_type (pFileInfo);
				
				gchar *cFileURI = g_strconcat (*cFullURI, "/", cName, NULL);
				cd_message (" + cFileURI : %s (mime:%s", cFileURI, cMimeType);
				icon = g_new0 (Icon, 1);
				icon->cBaseURI = cFileURI;
				icon->iType = iNewIconsType;
				
				icon->acCommand = g_strdup (cFileURI);
				icon->acName = g_strdup (cName);
				icon->acFileName = NULL;
				if (cMimeType != NULL && strncmp (cMimeType, "image", 5) == 0)  // && strncmp (cFileURI, "file://", 7) == 0
				{
					gchar *cHostname = NULL;
					gchar *cFilePath = g_filename_from_uri (cFileURI, &cHostname, &erreur);
					if (erreur != NULL)
					{
						g_error_free (erreur);
						erreur = NULL;
					}
					else if (cHostname == NULL || strcmp (cHostname, "localhost") == 0)  // on ne recupere la vignette que sur les fichiers locaux.
					{
						icon->acFileName = g_strdup (cFilePath);
						cairo_dock_remove_html_spaces (icon->acFileName);
					}
					g_free (cHostname);
					g_free (cFilePath);
				}
				if (icon->acFileName == NULL)
				{
					pSystemIcon
					icon->acFileName = gnome_icon_lookup (gtk_icon_theme_get_default (),
						NULL,
						NULL, // file_uri.
						NULL,
						info,
						info->mime_type,
						GNOME_ICON_LOOKUP_FLAGS_NONE,
						&iconLookupResultFlags);
				}
				
				icon->iVolumeID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_RDEV);
				
				if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
					icon->fOrder = g_file_info_get_size (pFileInfo);
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
					icon->fOrder = g_file_info_get_modification_time (pFileInfo);
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
					icon->fOrder = (cMimeType != NULL ? g_str_hash (cMimeType) : 0);
				pIconList = g_list_prepend (pIconList, icon);
			}
		}
	} while (TRUE);  // 'g_file_enumerator_close' est appelee lors du dernier 'g_file_enumerator_next_file'.
	
	
	
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
	GnomeIconLookupResultFlags iconLookupResultFlags;
	Icon *icon;
	while(1)
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
			
			icon = g_new0 (Icon, 1);
			icon->cBaseURI = cFileURI;
			icon->iType = iNewIconsType;
			if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (info->mime_type, "application/x-desktop") == 0)
			{
				gboolean bIsDirectory = FALSE;
				file_manager_get_file_info_from_desktop_link (cFileURI, &icon->acName, &icon->acCommand, &icon->acFileName, &bIsDirectory, &icon->iVolumeID);
				cd_message ("  bIsDirectory : %d; iVolumeID : %d; acFileName : %s", bIsDirectory, icon->iVolumeID, icon->acFileName);
			}
			else
			{
				icon->acCommand = g_strdup (cFileURI);
				icon->acName = g_strdup (info->name);
				icon->acFileName = NULL;
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
						icon->acFileName = g_strdup (cFilePath);
						cairo_dock_remove_html_spaces (icon->acFileName);
					}
					g_free (cHostname);
				}
				if (icon->acFileName == NULL)
				{
					icon->acFileName = gnome_icon_lookup (gtk_icon_theme_get_default (),
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
	
	GError *erreur = NULL;
	gboolean bSuccess = g_app_info_launch_default_for_uri (cURI,
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : couldn't get file info for '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
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

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
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
	data2[3] = icon;
	data2[4] = pDock;
	gnome_vfs_drive_mount (pDrive, (GnomeVFSVolumeOpCallback)_vfs_backend_mount_callback, data2);
	
	///gnome_vfs_volume_unref (pVolume);
	gnome_vfs_drive_unref (pDrive);
}

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
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
	data2[3] = icon;
	data2[4] = pDock;
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
		g_hash_table_insert (s_hMonitorHandleTable, g_strdup (cURI), data);
	}
}

void vfs_backend_remove_monitor (const gchar *cURI)
{
	if (cURI != NULL)
	{
		cd_message (">>> moniteur supprime sur %s", cURI);
		g_hash_table_remove (s_hMonitorHandleTable, cURI);
	}
}



gboolean vfs_backend_delete_file (const gchar *cURI)
{
	GFile *pFile = g_file_new_for_uri (cURI);
	
	GError *erreur = NULL;
	gboolean bSuccess = g_file_delete (pFile, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	
	g_object_unref (pFile);
	return bSuccess;
}

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	GFile *pOldFile = g_file_new_for_uri (cOldURI);
	GError *erreur = NULL;
	GFile *pNewFile =g_file_set_display_name(GFile *file, cNewName, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	gboolean bSuccess = (pNewFile != NULL);
	g_object_unref (pNewFile);
	return bSuccess;
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	cd_message (" %s -> %s", cURI, cDirectoryURI);
	GFile *pFile = g_file_new_for_uri (cURI);
	
	gchar *cFileName = g_file_get_basename (pFile);
	gchar *cNewFileURI = g_strconcat (cDirectoryURI, "/", cFileName, NULL);  // un peu moyen mais bon...
	GFile *pDestinationFile = g_file_new_for_uri (cNewFileURI);
	g_free (cNewFileURI);
	g_free (cFileName);
	
	GError *erreur = NULL;
	gboolean bSuccess = g_file_move (pFile,
		pDestinationFile,
		G_FILE_COPY_NOFOLLOW_SYMLINKS,
		NULL,
		NULL,  // GFileProgressCallback
		NULL,  // data
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	g_object_unref (pFile);
	g_object_unref (pDestinationFile);
	return bSuccess;
}

void vfs_backend_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	GFile *pFile = g_file_new_for_uri (cURI);
	GError *erreur = NULL;
	gchar *cQuery = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_SIZE, ",", G_FILE_ATTRIBUTE_TIME_MODIFIED, ",", G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE, ",", G_FILE_ATTRIBUTE_UNIX_UID, ",", G_FILE_ATTRIBUTE_UNIX_GID, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_READ, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, NULL);
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_free (cQuery);
	if (erreur != NULL)
	{
		cd_warning ("Attention : couldn't get file info for '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
	
	*iSize = g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);
	*iLastModificationTime = (time_t) g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
	*cMimeType = g_file_info_get_attribute_as_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
	*iUID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_UID);
	*iGID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_GID);
	int r = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
	int w = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
	int x = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
	*iPermissionsMask = r * 8 * 8 + w * 8 + x;
	
	g_object_unref (pFileInfo);
	g_object_unref (pFile);
}


gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gboolean bCreateIfNecessary)
{
	GFile *pFile = g_file_new_for_uri ("trash:/");
	gchar *cPath = g_file_get_path (pFile);
	g_object_unref (pFile);
	return cPath;
}

gchar *vfs_backend_get_desktop_path (void)
{
	GFile *pFile = g_file_new_for_uri ("desktop:/");
	gchar *cPath = g_file_get_path (pFile);
	g_object_unref (pFile);
	return cPath;
}

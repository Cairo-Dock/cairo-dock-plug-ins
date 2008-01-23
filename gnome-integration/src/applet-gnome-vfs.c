/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
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
	g_print ("%s (%s)\n", __func__, cBaseURI);
	GError *erreur = NULL;
	
	gchar *cFileData = NULL;
	int iFileSize = 0;
	if (gnome_vfs_read_entire_file (cBaseURI, &iFileSize, &cFileData) != GNOME_VFS_OK)
	{
		g_print ("Attention : couldn't read %s\n", cBaseURI);
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
		g_print ("Attention : %s\n", erreur->message);
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
	g_print ("%s (%s)\n", __func__, cBaseURI);
	
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
					g_print ("%d) %s\n", i, cSplit[i]);
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
	g_print (" -> cFullURI : %s\n", cFullURI);
	
	GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
	
	r = gnome_vfs_get_file_info (cFullURI, info, infoOpts);
	if (r != GNOME_VFS_OK) 
	{
		g_print ("Attention : couldn't get file info for '%s'\n", cFullURI);
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
	g_print ("  cMimeType : %s\n", cMimeType);
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



GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gchar **cFullURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	g_print ("%s (%s)\n", __func__, cBaseURI);
	
	GList *pIconList = NULL;
	
	const gchar *cURI;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
		cURI = "computer://";
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
		cURI = "network://";
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT_NETWORK) == 0)
		cURI = "computer://";
	else
		cURI = cBaseURI;
	
	*cFullURI = gnome_vfs_make_uri_from_input (cURI);  // pas franchement necessaire ...
	g_return_val_if_fail (*cFullURI != NULL, NULL);
	g_print (" -> cFullURI : %s\n", *cFullURI);
	
	/*if (strcmp (*cFullURI, "tvolumes:") == 0)
	{
		pIconList = file_manager_list_volumes ();
	}
	else if (strcmp (*cFullURI, "tdrives:") == 0)
	{
		pIconList = file_manager_list_drives ();
	}
	else if (strcmp (*cFullURI, "vfsroot://") == 0)
	{
		pIconList = file_pmanager_list_vfs_root ();
	}
	else*/
	{
		GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
		GnomeVFSDirectoryHandle *handle = NULL;
		GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
		GnomeVFSResult r = gnome_vfs_directory_open (&handle, *cFullURI, infoOpts);
		if (r!=GNOME_VFS_OK) 
		{
			return NULL;
		}
		
		GnomeVFSURI* dirUri = gnome_vfs_uri_new (*cFullURI);
		g_print ("  dirUri : %s\n", dirUri->text);
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
			
			if (strcmp (info->name, ".") != 0 && strcmp (info->name, "..") != 0)
			{
				fileUri = gnome_vfs_uri_append_path (dirUri, info->name);
				cFileURI = gnome_vfs_uri_to_string (fileUri, GNOME_VFS_URI_HIDE_NONE);
				g_print (" + cFileURI : %s\n", cFileURI);
				
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
					g_print ("  bIsDirectory : %d; iVolumeID : %d\n", bIsDirectory, icon->iVolumeID);
					
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
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE && (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE))
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
	}
	
	return pIconList;
}



void vfs_backend_launch_uri (const gchar *cURI)
{
	GnomeVFSResult r = gnome_vfs_url_show (cURI);
}


gchar *vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	g_print ("%s (%s)\n", __func__, cURI);
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	gchar *cLocalPath = gnome_vfs_get_local_path_from_uri (cURI);
	g_print (" cLocalPath : %s\n", cLocalPath);
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cLocalPath);
	g_free (cLocalPath);
	if (pVolume == NULL)
	{
		g_print ("Attention : no volum associated to %s\n", cURI);
		*bIsMounted = FALSE;
		return NULL;
	}
	else
	{
		gchar *cMountPointID = gnome_vfs_volume_get_activation_uri (pVolume);
		
		*bIsMounted = gnome_vfs_volume_is_mounted (pVolume);
		g_print ("  bIsMounted <- %d\n", *bIsMounted);
		
		gnome_vfs_volume_unref (pVolume);
		return cMountPointID;
	}
}


static void _vfs_backend_mount_callback (gboolean succeeded, char *error, char *detailed_error, gpointer *data)
{
	g_print ("%s (%d)\n", __func__, succeeded);
	if (! succeeded)
		g_print ("Attention : failed to mount (%s ; %s)\n", error, detailed_error);
	
	CairoDockFMMountCallback pCallback = data[0];
	
	pCallback (GPOINTER_TO_INT (data[1]), succeeded, data[2], data[3], data[4]);
	g_free (data[2]);
	g_free (data);
}

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	g_return_if_fail (iVolumeID > 0);
	g_print ("%s (ID:%d)\n", __func__, iVolumeID);
	
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
	g_print ("%s (%s)\n", __func__, cURI);
	
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	gchar *cLocalPath = gnome_vfs_get_local_path_from_uri (cURI);
	g_print (" cLocalPath : %s\n", cLocalPath);
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
	g_print ("%s (%d , data : %x)\n", __func__, event_type, user_data);
	
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
		g_print ("Attention : couldn't add monitor function to %s\n  I will not be able to receive events about this file\n", cURI);
		g_free (data);
	}
	else
	{
		g_print (">>> moniteur ajoute sur %s (%x)\n", cURI, user_data);
		data[2] = pHandle;
		g_hash_table_insert (s_fm_MonitorHandleTable, g_strdup (cURI), data);
	}
}

void vfs_backend_remove_monitor (const gchar *cURI)
{
	if (cURI != NULL)
	{
		g_print (">>> moniteur supprime sur %s\n", cURI);
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
	g_print (" %s -> %s\n", cOldURI, cNewURI);
	
	GnomeVFSResult r= gnome_vfs_move (cOldURI,
		cNewURI,
		FALSE);
	g_free (cNewURI);
	return (r == GNOME_VFS_OK);
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	g_print (" %s -> %s\n", cURI, cDirectoryURI);
	
	GnomeVFSURI *pVfsUri = gnome_vfs_uri_new (cURI);
	g_return_val_if_fail (pVfsUri != NULL, FALSE);
	
	gchar *cFileName = gnome_vfs_uri_extract_short_name (pVfsUri);
	g_print ("  pVfsUri : %s; cFileName : %s\n", pVfsUri->text, cFileName);
	
	GnomeVFSURI *pVfsDirUri = gnome_vfs_uri_new (cDirectoryURI);
	if (pVfsDirUri == NULL)
	{
		gnome_vfs_uri_unref (pVfsUri);
		g_free (cFileName);
		return FALSE;
	}
	g_print ("  pVfsDirUri : %s\n", pVfsDirUri->text);
	
	GnomeVFSURI *pVfsNewUri = gnome_vfs_uri_append_file_name (pVfsDirUri, cFileName);
	g_print ("  pVfsNewUri : %s\n", pVfsNewUri->text);
	
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
	g_print ("  cFullURI : %s\n", cFullURI);
	
	GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
	
	r = gnome_vfs_get_file_info (cFullURI, info, infoOpts);
	if (r != GNOME_VFS_OK) 
	{
		g_print ("Attention : couldn't get file info for '%s'\n", cFullURI);
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


gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gboolean bCreateIfNecessary)
{
	g_print ("%s (%s)\n", __func__, cNearURI);
	
	GnomeVFSURI *near_uri = gnome_vfs_uri_new (cNearURI);
	GnomeVFSURI *result = NULL;
	GnomeVFSResult r = gnome_vfs_find_directory (near_uri,
		GNOME_VFS_DIRECTORY_KIND_TRASH,
		&result,
		bCreateIfNecessary,
		TRUE,
		7*8*8+5*8+5);
	gnome_vfs_uri_unref (near_uri);
	if (r == GNOME_VFS_OK)
	{
		gchar *cTrashURI = g_strdup (result->text);
		gnome_vfs_uri_unref (result);
		return cTrashURI;
	}
	else
		return NULL;
	/*gchar *cTrashPath = g_strdup_printf ("%s/.Trash", g_getenv ("HOME"));
	if (g_file_test (cTrashPath, G_FILE_TEST_EXISTS))
		return cTrashPath;
	else
	{
		g_free (cTrashPath);
		cTrashPath = g_strdup_printf ("%s/.trash", g_getenv ("HOME"));
		if (g_file_test (cTrashPath, G_FILE_TEST_EXISTS))
			return cTrashPath;
		else
		{
			g_free (cTrashPath);
			return NULL;
		}
	}*/
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
	//return g_strdup_printf ("%s/Desktop", g_getenv ("HOME"));
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>

#include <file-manager-load-directory.h>

#include "file-manager-vfs-gnome-volumes.h"
#include "file-manager-vfs-gnome.h"


static GHashTable *s_fm_MonitorHandleTable = NULL;
static FileManagerOnEventFunc s_fm_GnomeOnEventFunc = NULL;


gboolean _file_manager_init_backend (FileManagerOnEventFunc pCallback)
{
	s_fm_GnomeOnEventFunc = pCallback;
	
	if (s_fm_MonitorHandleTable != NULL)
		g_hash_table_destroy (s_fm_MonitorHandleTable);
	
	s_fm_MonitorHandleTable = g_hash_table_new_full (g_direct_hash,
		g_direct_equal,
		g_free,
		(GDestroyNotify) gnome_vfs_monitor_cancel);  // le GnomeVFSMonitorHandle est-il libere lors du gnome_vfs_monitor_cancel () ?
	
	return (gnome_vfs_init ());  // ne fait rien si gnome-vfs est deja initialise.
}


static gboolean file_manager_follow_desktop_link (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint)
{
	g_print ("%s (%s)\n", __func__, cBaseURI);
	GError *erreur = NULL;
	
	//gchar *cFileData = file_manager_read_file (cBaseURI);
	gchar *cFileData = NULL;
	int iFileSize = 0;
	if (gnome_vfs_read_entire_file (cBaseURI, &iFileSize, &cFileData) != GNOME_VFS_OK)
	{
		g_print ("Attention : couldn't read %s\n", cBaseURI);
		return FALSE;
	}
	g_print (" => %s\n", cFileData);
	
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
	g_print ("cType : %s\n", cType);
	if (strncmp (cType, "Link", 4) != 0 && strncmp (cType, "FSDevice", 8) != 0)
	{
		g_free(cType);
		g_key_file_free (pKeyFile);
		return FALSE;
	}
	*bIsMountPoint = (strncmp (cType, "FSDevice", 8) == 0);
	g_free(cType);

	*cName = g_key_file_get_value (pKeyFile, "Desktop Entry", "Name", NULL);
	
	*cURI = g_key_file_get_value (pKeyFile, "Desktop Entry", "URL", NULL);
	
	*cIconName = g_key_file_get_value (pKeyFile, "Desktop Entry", "Icon", NULL);	
	
	*bIsDirectory = TRUE;
	
	g_key_file_free (pKeyFile);
	return TRUE;
}


void _file_manager_get_file_info (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint, double *fOrder, FileManagerSortType iSortType)
{
	g_print ("%s (%s)\n", __func__, cBaseURI);
	
	GnomeVFSResult r;
	GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
	gchar *cFullURI = gnome_vfs_make_uri_from_input (cBaseURI);
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
	
	const gchar *cMimeType = gnome_vfs_file_info_get_mime_type (info);
	g_print ("  cMimeType : %s\n", cMimeType);
	if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (cMimeType, "application/x-desktop") == 0)
	{
		gnome_vfs_file_info_unref (info);
		file_manager_follow_desktop_link (cFullURI, cName, cURI, cIconName, bIsDirectory, bIsMountPoint);
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
	if (strncmp (cMimeType, "image", 5) == 0 && strncmp (cBaseURI, "file://", 7) == 0)
	{
		*cIconName = g_strdup (cBaseURI+7);
	}
	else
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
	
	gboolean bIsMounted;
	gchar *cActivationURI = _file_manager_is_mounting_point (*cURI, &bIsMounted);
	*bIsMountPoint = (cActivationURI != NULL);  // a priori toujours FALSE.
	g_free (cActivationURI);
	
	if (iSortType == FILE_MANAGER_SORT_BY_DATE)
		*fOrder = info->mtime;
	else if (iSortType == FILE_MANAGER_SORT_BY_SIZE)
		*fOrder = info->size;
	else if (iSortType == FILE_MANAGER_SORT_BY_TYPE)
		*fOrder = info->type;
	else
		*fOrder = 0;
	
	gnome_vfs_file_info_unref (info);
}



GList *_file_manager_list_directory (gchar *cBaseURI, FileManagerSortType iSortType)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	g_print ("%s ()\n", __func__);
	
	GList *pIconList = NULL;
	
	gchar *cURI;
	if (strcmp (cBaseURI, FILE_MANAGER_VFS_ROOT) == 0)
		cURI = "computer://";
	else if (strcmp (cBaseURI, FILE_MANAGER_NETWORK) == 0)
		cURI = "network://";
	else
		cURI = cBaseURI;
	
	gchar * cFullURI = gnome_vfs_make_uri_from_input (cURI);
	g_print ("cFullURI : %s\n", cFullURI);
	
	if (strcmp (cFullURI, "tvolumes:") == 0)
	{
		pIconList = file_manager_list_volumes ();
	}
	else if (strcmp (cFullURI, "tdrives:") == 0)
	{
		pIconList = file_manager_list_drives ();
	}
	else if (strcmp (cFullURI, "vfsroot://") == 0)
	{
		pIconList = file_pmanager_list_vfs_root ();
	}
	else
	{
		GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
		GnomeVFSDirectoryHandle *handle = NULL;
		GnomeVFSFileInfoOptions infoOpts = GNOME_VFS_FILE_INFO_FOLLOW_LINKS | GNOME_VFS_FILE_INFO_GET_MIME_TYPE;
		GnomeVFSResult r = gnome_vfs_directory_open (&handle, cFullURI, infoOpts);
		if (r!=GNOME_VFS_OK) 
		{
			return NULL;
		}
		
		GnomeVFSURI* dirUri = gnome_vfs_uri_new (cFullURI);
		g_print ("dirUri : %s\n", dirUri);
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
				icon->iType = CAIRO_DOCK_LAUNCHER;
				if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (info->mime_type, "application/x-desktop") == 0)
				{
					gboolean bIsDirectory = FALSE;
					file_manager_follow_desktop_link (cFileURI, &icon->acName, &icon->acCommand, &icon->acFileName, &bIsDirectory, &icon->bIsMountingPoint);
					g_print ("  bIsDirectory : %d\n", bIsDirectory);
					
				}
				else
				{
					icon->acCommand = g_strdup (cFileURI);
					icon->acName = g_strdup (info->name);
					if (strncmp (info->mime_type, "image", 5) == 0 && strncmp (cFileURI, "file://", 7) == 0)
					{
						icon->acFileName = g_strdup (cFileURI+7);
					}
					else
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
				if (iSortType == FILE_MANAGER_SORT_BY_SIZE && (valid & GNOME_VFS_FILE_INFO_FIELDS_SIZE))
					icon->fOrder = info->size;
				else if (iSortType == FILE_MANAGER_SORT_BY_DATE && (valid & GNOME_VFS_FILE_INFO_FIELDS_MTIME))
					icon->fOrder = info->mtime;
				else if (iSortType == FILE_MANAGER_SORT_BY_DATE && (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE))
					icon->fOrder = info->type;
				pIconList = g_list_prepend (pIconList, icon);
				
				gnome_vfs_uri_unref (fileUri);
			}
			gnome_vfs_file_info_clear (info);
		}
		gnome_vfs_uri_unref (dirUri);
		
		gnome_vfs_directory_close (handle);
		gnome_vfs_file_info_unref (info);
		
		pIconList = file_manager_sort_files (pIconList, iSortType);
	}
	
	return pIconList;
}



static void file_manager_just_launch_uri (gchar *cURI)
{
	GnomeVFSResult r = gnome_vfs_url_show (cURI);
}

void _file_manager_launch_uri (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	g_return_if_fail (cURI != NULL);
	
	gboolean bIsMounted;
	gchar *cNeededMountPointID = _file_manager_is_mounting_point (cURI, &bIsMounted);
	if (cNeededMountPointID != NULL && ! bIsMounted)
	{
		GtkWidget *dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Do you want to mount this point ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer != GTK_RESPONSE_YES)
			return ;
		
		gchar *cActivatedURI = _file_manager_mount (cNeededMountPointID);
		g_free (cNeededMountPointID);
		if (cActivatedURI != NULL)
			file_manager_just_launch_uri (cActivatedURI);
		g_free (cActivatedURI);
	}
	else
		file_manager_just_launch_uri (cURI);
}




gchar *_file_manager_is_mounting_point (gchar *cURI, gboolean *bIsMounted)
{
	g_print ("%s (%s)\n", __func__, cURI);
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cURI);
	if (pVolume == NULL)
	{
		*bIsMounted = FALSE;
		return NULL;
	}
	else
	{
		gchar *cMountPointID = gnome_vfs_volume_get_activation_uri (pVolume);
		*bIsMounted = gnome_vfs_volume_is_mounted (pVolume);
		
		gnome_vfs_volume_unref (pVolume);
		return cMountPointID;
	}
}


gchar * _file_manager_mount (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cURI);
	GnomeVFSDrive *pDrive = gnome_vfs_volume_get_drive (pVolume);
	
	gnome_vfs_drive_mount (pDrive, NULL, NULL);
	gchar *cActivatedURI = gnome_vfs_volume_get_activation_uri (pVolume);
	
	gnome_vfs_volume_unref (pVolume);
	gnome_vfs_drive_unref (pDrive);
	
	return cActivatedURI;
}


void _file_manager_unmount (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	
	GnomeVFSVolumeMonitor *pVolumeMonitor = gnome_vfs_get_volume_monitor();  // c'est un singleton.
	GnomeVFSVolume *pVolume = gnome_vfs_volume_monitor_get_volume_for_path (pVolumeMonitor, cURI);
	gnome_vfs_volume_unmount (pVolume, NULL, NULL);
	gnome_vfs_volume_unref (pVolume);
}



static void file_manager_gnome_monitor_callback (GnomeVFSMonitorHandle *handle,
	const gchar *monitor_uri,
	const gchar *info_uri,
	GnomeVFSMonitorEventType event_type,
	Icon *pIcon)
{
	g_print ("%s (%d)\n", __func__, event_type);
	
	FileManagerEventType iEventType;
	switch (event_type)
	{
		case GNOME_VFS_MONITOR_EVENT_CHANGED :
			iEventType = FILE_MANAGER_ICON_MODIFIED;
		break;
		
		case GNOME_VFS_MONITOR_EVENT_DELETED :
			iEventType = FILE_MANAGER_ICON_DELETED;
		break;
		
		case GNOME_VFS_MONITOR_EVENT_CREATED :
			iEventType = FILE_MANAGER_ICON_CREATED;
		break;
		
		default :
		return ;
	}
	s_fm_GnomeOnEventFunc (iEventType, info_uri, pIcon);
}



void _file_manager_add_monitor (Icon *pIcon)
{
	GnomeVFSMonitorHandle *pHandle = NULL;
	GnomeVFSResult r = gnome_vfs_monitor_add (&pHandle,
		pIcon->acCommand,
		(pIcon->pSubDock != NULL ? GNOME_VFS_MONITOR_DIRECTORY : GNOME_VFS_MONITOR_FILE),
		(GnomeVFSMonitorCallback) file_manager_gnome_monitor_callback,
		pIcon);
	if (r != GNOME_VFS_OK)
	{
		g_print ("Attention : couldn't add monitor function to %s\n  I will not be able to receive events about this file\n", pIcon->acCommand);
	}
	else
	{
		g_hash_table_insert (s_fm_MonitorHandleTable, g_strdup (pIcon->acCommand), pHandle);
	}
}

void _file_manager_remove_monitor (Icon *pIcon)
{
	g_hash_table_remove (s_fm_MonitorHandleTable, pIcon->acCommand);
}


void _file_manager_delete_file (gchar *cURI)
{
	GnomeVFSResult r = gnome_vfs_unlink (cURI);
	if (r != GNOME_VFS_OK)
		g_print ("Attention : couldn't delete this file.\nnCheck that you have writing rights on this file.\n");
}

void _file_manager_rename_file (gchar *cOldURI, gchar *cNewName)
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
	if (r != GNOME_VFS_OK)
		g_print ("Attention : couldn't reame this file.\nCheck that you have writing rights, and that new name does not already exist.\n");
	g_free (cNewURI);
}

void _file_manager_move_file (gchar *cURI, gchar *cDirectoryURI)
{
	g_print (" %s -> %s\n", cURI, cDirectoryURI);
	
	GnomeVFSResult r= gnome_vfs_move (cURI,
		cDirectoryURI,
		FALSE);
	if (r != GNOME_VFS_OK)
		g_print ("Attention : couldn't move this file.\nCheck that you have writing rights, and that the name does not already exist.\n");
}

void _file_manager_get_file_properties (gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
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

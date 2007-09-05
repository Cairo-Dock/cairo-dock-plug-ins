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

#include "file-manager-vfs-gnome-volumes.h"
#include "file-manager-vfs-gnome.h"


FileManagerOnEventFunc s_fm_GnomeOnEventFunc = NULL;


gboolean _file_manager_init_backend (FileManagerOnEventFunc pCallback)
{
	s_fm_GnomeOnEventFunc = pCallback;
	
	return (gnome_vfs_init ());
}


static gchar * file_manager_read_file (gchar *cURI)
{
	g_print ("%s (%s)\n", __func__, cURI);
	
	GString *sFileData = g_string_new ("");
	gchar *cBuffer = g_new0 (gchar, 1024 + 1);
	GnomeVFSFileSize iNbBytesRead = 0;
	/*gchar *cBuffer = g_new0 (gchar, 1024);
	size_t bufSize = FILE_MANAGER_INITIAL_BUFFER_SIZE;
	GnomeVFSFileSize bytes_read;
	int pos=0;*/
	gchar * cFullURI = gnome_vfs_make_uri_from_input (cURI);
	g_print ("cFullURI : %s\n", cFullURI);
	GnomeVFSHandle *handle = NULL;
	
	GnomeVFSResult r = gnome_vfs_open (&handle, cFullURI, GNOME_VFS_OPEN_READ);
	g_free (cFullURI);
	g_return_val_if_fail (r == GNOME_VFS_OK, NULL);
	
	while (1)
	{
		r = gnome_vfs_read (handle, cBuffer, 1024, &iNbBytesRead);
		if (r == GNOME_VFS_ERROR_EOF)
			break ;
		if (r!=GNOME_VFS_OK) 
		{
			g_free (cBuffer);
			g_string_free (sFileData, TRUE);
			gnome_vfs_close (handle);
			return NULL;
		}
		g_string_append (sFileData, cBuffer);
		memset (cBuffer, 0, 1024);
	}
	
	g_string_append (sFileData, cBuffer);
	g_free (cBuffer);
	gnome_vfs_close (handle);
	
	gchar *cFileData = sFileData->str;
	g_string_free (sFileData, FALSE);
	return cFileData;
}


static gboolean file_manager_follow_desktop_link (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint)
{
	g_print ("%s (%s)\n", __func__, cBaseURI);
	GError *erreur = NULL;
	
	gchar *cFileData = file_manager_read_file (cBaseURI);
	g_print (" => %s\n", cFileData);
	
	GKeyFile *pKeyFile = g_key_file_new ();
	//g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	//g_free (cDesktopFilePath);
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


void _file_manager_get_file_info (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint)
{
	g_print ("%s (%s)\n", __func__, cBaseURI);
	
	GnomeVFSResult r;
	GnomeVFSFileInfo * info = gnome_vfs_file_info_new ();
	gchar *cFullURI = gnome_vfs_make_uri_from_input (cBaseURI);
	
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
	if ( (valid & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) && strcmp (cMimeType, "application/x-desktop") == 0)
	{
		gnome_vfs_file_info_unref (info);
		file_manager_follow_desktop_link (cFullURI, cName, cURI, cIconName, bIsDirectory, bIsMountPoint);
		return ;
	}
	g_free (cFullURI);
	
	*cName = g_strdup (info->name);
	
	*cURI = g_strdup (cBaseURI);
	
	if (valid & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
		*bIsDirectory = (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
	else
		*bIsDirectory = FALSE;
	
	GnomeIconLookupResultFlags iconLookupResultFlags;
	*cIconName = gnome_icon_lookup (gtk_icon_theme_get_default (),
		NULL,
		NULL /* const char *file_uri */,
		NULL,
		info,
		info->mime_type,
		GNOME_ICON_LOOKUP_FLAGS_NONE,  // GNOME_ICON_LOOKUP_FLAGS_ALLOW_SVG_AS_THEMSELVES
		&iconLookupResultFlags);
	
	gboolean bIsMounted;
	gchar *cActivationURI = _file_manager_is_mounting_point (*cURI, &bIsMounted);
	*bIsMountPoint = (cActivationURI != NULL);  // a priori toujours FALSE.
	g_free (cActivationURI);
	
	gnome_vfs_file_info_unref (info);
}


GList *_file_manager_list_directory (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	
	GList *pIconList = NULL;
	
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
		int iOrder = 0;
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
				icon->bIsURI = TRUE;
				icon->iType = CAIRO_DOCK_LAUNCHER;
				icon->acCommand = g_strdup (cFileURI);
				icon->acName = g_strdup (info->name);
				icon->acFileName = gnome_icon_lookup (gtk_icon_theme_get_default (),
					NULL,
					NULL /* const char *file_uri */,
					NULL,
					info,
					info->mime_type,
					GNOME_ICON_LOOKUP_FLAGS_NONE,
					&iconLookupResultFlags);
				icon->fOrder = iOrder ++;
				pIconList = g_list_prepend (pIconList, icon);
				
				gnome_vfs_uri_unref (fileUri);
				g_free (cFileURI);
			}
			gnome_vfs_file_info_clear (info);
		}
		gnome_vfs_uri_unref (dirUri);
		
		gnome_vfs_directory_close (handle);
		gnome_vfs_file_info_unref (info);
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
}


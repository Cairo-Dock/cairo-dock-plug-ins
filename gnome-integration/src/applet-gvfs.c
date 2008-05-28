/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

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


static gchar *_cd_get_icon_path (GIcon *pIcon)
{
	gchar *cIconPath = NULL;
	if (G_IS_THEMED_ICON (pIcon))
	{
		const gchar * const *cFileNames = g_themed_icon_get_names (G_THEMED_ICON (pIcon));
		//cd_message ("icones possibles : %s\n", g_strjoinv (":", (gchar **) cFileNames));
		int i;
		for (i = 0; cFileNames[i] != NULL && cIconPath == NULL; i ++)
		{
			//cd_message (" une icone possible est : %s\n", cFileNames[i]);
			cIconPath = cairo_dock_search_icon_s_path (cFileNames[i]);
			//cd_message ("  chemin trouve : %s\n", cIconPath);
		}
	}
	else if (G_IS_FILE_ICON (pIcon))
	{
		GFile *pFile = g_file_icon_get_file (G_FILE_ICON (pIcon));
		cIconPath = g_file_get_basename (pFile);
		//cd_message (" file_icon => %s\n", cIconPath);
	}
	return cIconPath;
}


static void _cd_find_mount_from_volume_name (const gchar *cVolumeName, GMount **pFoundMount, gchar **cURI, gchar **cIconName)
{
	g_return_if_fail (cVolumeName != NULL);
	cd_message ("%s (%s)", __func__, cVolumeName);
	GFile *pFile = g_file_new_for_uri ("computer://");
	GError *erreur = NULL;
	gchar *cAttributes = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
		G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, ",",
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE, NULL);
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_free (cAttributes);
	//g_object_unref (pFile);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
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
			GFileType iFileType = g_file_info_get_file_type (pFileInfo);
			if (iFileType == G_FILE_TYPE_MOUNTABLE)
			{
				const gchar *cFileName = g_file_info_get_name (pFileInfo);
				cd_message ("  test de  %s...", cFileName);
				const gchar *cTargetURI = g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
				
				GMount *pMount = NULL;
				if (cTargetURI != NULL)
				{
					GFile *file = g_file_new_for_uri (cTargetURI);
					pMount = g_file_find_enclosing_mount (file, NULL, NULL);
					//g_object_unref (file);
				}
				if (pMount != NULL)
				{
					gchar *cName = g_mount_get_name (pMount);
					cd_message ("    mount : %s", cName);
					if (cName != NULL && strcmp (cName, cVolumeName) == 0)
					{
						cd_message ("TROUVE");
						*pFoundMount = pMount;
						*cURI = g_strconcat ("computer:///", cFileName, NULL);
						GIcon *pSystemIcon = g_mount_get_icon (pMount);
						*cIconName = _cd_get_icon_path (pSystemIcon);
						g_free (cName);
						break ;
					}
					g_free (cName);
				}
			}
		}
	} while (TRUE);
	//g_object_unref (pFileEnum);
}

static GDrive *_cd_find_drive_from_name (const gchar *cName)
{
	g_return_val_if_fail (cName != NULL, NULL);
	cd_message ("%s (%s)", __func__, cName);
	GVolumeMonitor *pVolumeMonitor = g_volume_monitor_get ();
	GDrive *pFoundDrive = NULL;
	
	//\___________________ On chope les disques connectes (lecteur de CD/disquette/etc) et on liste leurs volumes.
	GList *pDrivesList = g_volume_monitor_get_connected_drives (pVolumeMonitor);
	GList *dl;
	GDrive *pDrive;
	gchar *cDriveName;
	for (dl = pDrivesList; dl != NULL; dl = dl->next)
	{
		pDrive = dl->data;
		if (pFoundDrive == NULL)
		{
			cDriveName = g_drive_get_name  (pDrive);
			cd_message ("  drive '%s'", cDriveName);
			if (cDriveName != NULL && strcmp (cDriveName, cName) == 0)
				pFoundDrive = pDrive;
			else
				g_object_unref (pDrive);
			//g_free (cDriveName);
		}
		else
			g_object_unref (pDrive);
	}
	g_list_free (pDrivesList);
	return pFoundDrive;
}
static gchar *_cd_find_volume_name_from_drive_name (const gchar *cName)
{
	g_return_val_if_fail (cName != NULL, NULL);
	cd_message ("%s (%s)", __func__, cName);
	GDrive *pDrive = _cd_find_drive_from_name (cName);
	g_return_val_if_fail (pDrive != NULL, NULL);
	
	gchar *cVolumeName = NULL;
	GList *pAssociatedVolumes = g_drive_get_volumes (pDrive);
	if (pAssociatedVolumes != NULL)
	{
		GVolume *pVolume;
		GList *av;
		if (pAssociatedVolumes->next != NULL)
		{
			cd_message ("ce disque contient plus d'un volume, on garde le nom du disque plutot que de selectionner le nom d'un volume");
			cd_message ("Pour info, la liste des volumes disponibles sur ce disque est :");
			for (av = pAssociatedVolumes; av != NULL; av = av->next)
			{
				pVolume = av->data;
				cd_message ("  - %s", g_volume_get_name  (pVolume));
				/*if (cVolumeName == NULL)
					cVolumeName = g_volume_get_name  (pVolume);
				else
					cd_warning ("Attention : this drive (%s) has more than 1 volume but we only consider the first one (%s), ignoring %s", cName, cVolumeName, g_volume_get_name  (pVolume));*/
				g_object_unref (pVolume);
			}
		}
		else
		{
			pVolume = pAssociatedVolumes->data;
			cVolumeName = g_volume_get_name  (pVolume);
			g_object_unref (pVolume);
			cd_message ("ce disque contient 1 seul volume (%s), on prend son nom", cVolumeName);
		}
		g_list_free (pAssociatedVolumes);
	}
	//g_object_unref (pDrive);
	return cVolumeName;
}
static gboolean _cd_find_can_eject_from_drive_name (const gchar *cName)
{
	cd_debug ("%s (%s)", __func__, cName);
	GDrive *pDrive = _cd_find_drive_from_name (cName);
	g_return_val_if_fail (pDrive != NULL, FALSE);
	
	gboolean bCanEject = g_drive_can_eject (pDrive);
	//g_object_unref (pDrive);
	return bCanEject;
}

void vfs_backend_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
{
	g_return_if_fail (cBaseURI != NULL);
	GError *erreur = NULL;
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	gchar *cFullURI;
	if (strncmp (cBaseURI, "x-nautilus-desktop://", 21) == 0)
	{
		gchar *cNautilusFile = g_strdup (cBaseURI+14);
		memcpy (cNautilusFile, "file", 4);
		if (g_str_has_suffix (cBaseURI, ".volume"))
		{
			cNautilusFile[strlen(cNautilusFile)-7] = '\0';
		}
		else if (g_str_has_suffix (cBaseURI, ".drive"))
		{
			cNautilusFile[strlen(cNautilusFile)-6] = '\0';
		}
		cFullURI = g_filename_from_uri (cNautilusFile, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return ;
		}
		gchar *cVolumeName = cFullURI + 1;  // on saute le '/'.
		cd_message ("cVolumeName : %s", cVolumeName);
		
		GMount *pMount = NULL;
		_cd_find_mount_from_volume_name (cVolumeName, &pMount, cURI, cIconName);
		g_return_if_fail (pMount != NULL);
		
		*cName = g_strdup (cVolumeName);
		*bIsDirectory = TRUE;
		*iVolumeID = 1;
		*fOrder = 0;
		//g_object_unref (pMount);
		
		g_free (cFullURI);
		//g_free (cNautilusFile);
		return;
	}
	else
	{
		if (*cBaseURI == '/')
			cFullURI = g_filename_to_uri (cBaseURI, NULL, NULL);
		else
			cFullURI = g_strdup (cBaseURI);
	}
	cd_message (" -> cFullURI : %s", cFullURI);
	
	GFile *pFile = g_file_new_for_uri (cFullURI);
	
	gchar *cQuery = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_SIZE, ",",
		G_FILE_ATTRIBUTE_TIME_MODIFIED, ",",
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
		G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, ",",
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE, NULL);
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_free (cQuery);
	//g_object_unref (pFile);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	*cURI = cFullURI;
	const gchar *cFileName = g_file_info_get_name (pFileInfo);
	const gchar *cMimeType = g_file_info_get_content_type (pFileInfo);
	GFileType iFileType = g_file_info_get_file_type (pFileInfo);
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
	{
		GTimeVal t;
		g_file_info_get_modification_time (pFileInfo, &t);
		*fOrder = t.tv_sec;
	}
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
		*fOrder = g_file_info_get_size (pFileInfo);
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
		*fOrder = (cMimeType != NULL ? *((int *) cMimeType) : 0);
	else
		*fOrder = 0;
	
	*bIsDirectory = (iFileType == G_FILE_TYPE_DIRECTORY);
	cd_message (" => '%s' (mime:%s ; bIsDirectory:%d)", cFileName, cMimeType, *bIsDirectory);
	
	if (iFileType == G_FILE_TYPE_MOUNTABLE)
	{
		*cName = NULL;
		*iVolumeID = 1;
		
		const gchar *cTargetURI = g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
		cd_message ("  cTargetURI:%s", cTargetURI);
		GMount *pMount = NULL;
		if (cTargetURI != NULL)
		{
			GFile *file = g_file_new_for_uri (cTargetURI);
			pMount = g_file_find_enclosing_mount (file, NULL, NULL);
			//g_object_unref (file);
		}
		if (pMount != NULL)
		{
			*cName = g_mount_get_name (pMount);
			cd_message ("un GMount existe (%s)",* cName);
		}
		else
		{
			gchar *cMountName = g_strdup (cFileName);
			gchar *str = strrchr (cMountName, '.');  // on vire l'extension ".volume" ou ".drive".
			if (str != NULL)
			{
				*str = '\0';
				if (strcmp (str+1, "link") == 0)  // pour les liens, on prend le nom du lien.
				{
					if (strcmp (cMountName, "root") == 0)  // on remplace 'root' par un nom plus parlant, sinon on prendra le nom du lien.
					{
						*cName = g_strdup ("/");
					}
				}
				else if (strcmp (str+1, "drive") == 0)  // on cherche un nom plus parlant si possible.
				{
					gchar *cVolumeName = _cd_find_volume_name_from_drive_name (cMountName);
					if (cVolumeName != NULL)
					{
						*cName = cVolumeName;
					}
				}
			}
			if (*cName == NULL)
				*cName = cMountName;
			//else
				//g_free (cMountName);
		}
		if (*cName ==  NULL)
			*cName = g_strdup (cFileName);
	}
	else
	{
		*iVolumeID = 0;
		*cName = g_strdup (cFileName);
	}
	
	*cIconName = NULL;
	if (cMimeType != NULL && strncmp (cMimeType, "image", 5) == 0)
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
		//g_free (cHostname);
	}
	if (*cIconName == NULL)
	{
		GIcon *pSystemIcon = g_file_info_get_icon (pFileInfo);
		if (pSystemIcon != NULL)
		{
			*cIconName = _cd_get_icon_path (pSystemIcon);
		}
	}
	cd_message ("cIconName : %s", *cIconName);
	
	//*iVolumeID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE);
	//cd_message ("ID : %d\n", *iVolumeID);
	//g_object_unref (pFileInfo);
}

static Icon *_cd_get_icon_for_volume (GVolume *pVolume, GMount *pMount)
{
	Icon *pNewIcon = NULL;
	GIcon *pIcon;
	GFile *pRootDir;
	
	if (pVolume != NULL)
		pMount = g_volume_get_mount (pVolume);
	else if (pMount == NULL)
		return NULL;
	
	if (pMount != NULL)  // ce volume est monte.
	{
		pNewIcon = g_new0 (Icon, 1);
		pNewIcon->acName = g_mount_get_name (pMount);
		
		pRootDir = g_mount_get_root (pMount);
		pNewIcon->acCommand = g_file_get_uri (pRootDir);
		//g_object_unref (pRootDir);
		
		pIcon = g_mount_get_icon (pMount);
		pNewIcon->acFileName = _cd_get_icon_path (pIcon);
		//g_object_unref (pIcon);
		
		//g_object_unref (pMount);
	}
	else  // ce volume est demonte, on le montre quand meme (l'automount peut etre off).
	{
		pNewIcon = g_new0 (Icon, 1);
		pNewIcon->acName = g_volume_get_name (pVolume);
		
		pIcon = g_volume_get_icon (pVolume);
		pNewIcon->acFileName = _cd_get_icon_path (pIcon);
		//g_object_unref (pIcon);
		
		pNewIcon->acCommand = g_strdup (pNewIcon->acName);
	}
	pNewIcon->iVolumeID = 1;
	pNewIcon->cBaseURI = g_strdup (pNewIcon->acCommand);
	cd_message (" => %s", pNewIcon->acCommand);
	return pNewIcon;
}

GList *vfs_backend_list_volumes (void)
{
	GVolumeMonitor *pVolumeMonitor = g_volume_monitor_get ();
	GList *pIconsList = NULL;
	Icon *pNewIcon;
	
	//\___________________ On chope les disques connectes (lecteur de CD/disquette/etc) et on liste leurs volumes.
	GList *pDrivesList = g_volume_monitor_get_connected_drives (pVolumeMonitor);
	GList *pAssociatedVolumes;
	GList *dl, *av;
	GDrive *pDrive;
	GVolume *pVolume;
	for (dl = pDrivesList; dl != NULL; dl = dl->next)
	{
		pDrive = dl->data;
		cd_message ("drive '%s'", g_drive_get_name  (pDrive));
		
		pAssociatedVolumes = g_drive_get_volumes (pDrive);
		if (pAssociatedVolumes != NULL)
		{
			for (av = pAssociatedVolumes; av != NULL; av = av->next)
			{
				pVolume = av->data;
				cd_message (" + volume '%s'", g_volume_get_name  (pVolume));
				pNewIcon = _cd_get_icon_for_volume (pVolume, NULL);
				if (pNewIcon != NULL)
					pIconsList = g_list_prepend (pIconsList, pNewIcon);
				//g_object_unref (pVolume);
			}
			g_list_free (pAssociatedVolumes);
		}
		else  // le disque n'a aucun volume montable
		{
			cd_message ("  le disque n'a aucun volume montable");
			/*if (g_drive_is_media_removable (pDrive) && ! g_drive_is_media_check_automatic (pDrive))
			{
				g_drive_get_icon (pDrive);
				g_drive_get_name (pDrive);
			}*/
		}
		//g_object_unref (pDrive);
	}
	g_list_free (pDrivesList);

	//\___________________ On chope les volumes qui ne sont pas associes a un disque.
	GList *pVolumesList = g_volume_monitor_get_volumes (pVolumeMonitor);
	GList *v;
	for (v = pVolumesList; v != NULL; v = v->next)
	{
		pVolume = v->data;
		cd_message ("volume '%s'", g_volume_get_name  (pVolume));
		pDrive = g_volume_get_drive (pVolume);
		if (pDrive != NULL)  // on l'a deja liste dans la 1ere boucle.
		{
			cd_message ("  drive '%s' est deja liste", g_drive_get_name (pDrive));
			//g_object_unref (pDrive);
		}
		else
		{
			cd_message (" + volume '%s'\n", g_volume_get_name  (pVolume));
			if (pNewIcon != NULL)
				pNewIcon = _cd_get_icon_for_volume (pVolume, NULL);
			pIconsList = g_list_prepend (pIconsList, pNewIcon);
		}
		//g_object_unref (pVolume);
	}
	g_list_free (pVolumesList);

	//\___________________ On chope les points de montage qui n'ont pas de volumes. (montage de mtab, ftp, etc)
	GList *pMountsList = g_volume_monitor_get_mounts (pVolumeMonitor);
	GMount *pMount;
	GList *m;
	for (m = pMountsList; m != NULL; m = m->next)
	{
		pMount = m->data;
		cd_message ("mount '%s'", g_mount_get_name (pMount));
		pVolume = g_mount_get_volume (pMount);
		if (pVolume != NULL)  // on l'a deja liste precedemment.
		{
			cd_message ("volume '%s' est deja liste", g_volume_get_name  (pVolume));
			//g_object_unref (pVolume);
		}
		else
		{
			cd_message ("+ volume '%s'", g_volume_get_name  (pVolume));
			if (pNewIcon != NULL)
				pNewIcon = _cd_get_icon_for_volume (NULL, pMount);
			pIconsList = g_list_prepend (pIconsList, pNewIcon);
		}
		//g_object_unref (pMount);
	}
	g_list_free (pMountsList);
	
	return pIconsList;
}

GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gboolean bListHiddenFiles, gchar **cFullURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	gchar *cURI;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
	{
		cURI = g_strdup ("computer://");
		///*cFullURI = cURI;
		///return vfs_backend_list_volumes ();
		//vfs_backend_list_volumes ();
	}
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
		cURI = g_strdup ("network://");
	else
		cURI = (*cBaseURI == '/' ? g_strconcat ("file://", cBaseURI, NULL) : g_strdup (cBaseURI));
	*cFullURI = cURI;
	
	GFile *pFile = g_file_new_for_uri (cURI);
	GError *erreur = NULL;
	gchar *cAttributes = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_SIZE, ",",
		G_FILE_ATTRIBUTE_TIME_MODIFIED, ",",
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, ",",
		G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
		G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN, ",",
		G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, ",",
		G_FILE_ATTRIBUTE_UNIX_RDEV, ",",
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE, NULL);
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	//g_free (cAttributes);
	//g_object_unref (pFile);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
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
			
			gboolean bIsHidden = g_file_info_get_is_hidden (pFileInfo);
			if (bListHiddenFiles || ! bIsHidden)
			{
				GFileType iFileType = g_file_info_get_file_type (pFileInfo);
				GIcon *pFileIcon = g_file_info_get_icon (pFileInfo);
				if (pFileIcon == NULL)
				{
					cd_message ("AUCUNE ICONE");
					continue;
				}
				const gchar *cFileName = g_file_info_get_name (pFileInfo);
				const gchar *cMimeType = g_file_info_get_content_type (pFileInfo);
				gchar *cName = NULL;
				
				icon = g_new0 (Icon, 1);
				icon->iType = iNewIconsType;
				icon->cBaseURI = g_strconcat (*cFullURI, "/", cFileName, NULL);
				cd_message ("+ %s (mime:%s)", icon->cBaseURI, cMimeType);
				
				if (iFileType == G_FILE_TYPE_MOUNTABLE)
				{
					const gchar *cTargetURI = g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
					cd_message ("  c'est un point de montage correspondant a %s (ID:%d)", cTargetURI, g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_RDEV));
					
					GMount *pMount = NULL;
					if (cTargetURI != NULL)
					{
						GFile *file = g_file_new_for_uri (cTargetURI);
						pMount = g_file_find_enclosing_mount (file, NULL, NULL);
						//g_object_unref (file);
					}
					if (pMount != NULL)
					{
						cName = g_mount_get_name (pMount);
						cd_message ("un GMount existe (%s)", cName);
						
						GVolume *volume = g_mount_get_volume (pMount);
						if (volume)
							cd_message ("  volume associe : %s", g_volume_get_name (volume));
						GDrive *drive = g_mount_get_drive (pMount);
						if (drive)
							cd_message ("  disque associe : %s", g_drive_get_name (drive));
						
						///pFileIcon = g_mount_get_icon (pMount);
					}
					else
					{
						cName = g_strdup (cFileName);
						gchar *str = strrchr (cName, '.');  // on vire l'extension ".volume" ou ".drive".
						if (str != NULL)
						{
							*str = '\0';
							if (strcmp (str+1, "link") == 0)
							{
								if (strcmp (cName, "root") == 0)
								{
									g_free (cName);
									cName = g_strdup ("/");
								}
							}
							else if (strcmp (str+1, "drive") == 0)  // on cherche un nom plus parlant si possible.
							{
								gchar *cVolumeName = _cd_find_volume_name_from_drive_name (cName);
								if (cVolumeName != NULL)
								{
									g_free (cName);
									cName = cVolumeName;
								}  /// sinon ne pas afficher ce disque, mais uniquement ses volumes ?...
							}
						}
					}
					icon->iVolumeID = 1;
					cd_message ("le nom de ce volume est : %s", cName);
				}
				else
					cName = g_strdup (cFileName);
				
				icon->acCommand = g_strdup (icon->cBaseURI);
				icon->acName = cName;
				icon->acFileName = NULL;
				if (cMimeType != NULL && strncmp (cMimeType, "image", 5) == 0)
				{
					gchar *cHostname = NULL;
					gchar *cFilePath = g_filename_from_uri (icon->cBaseURI, &cHostname, &erreur);
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
					icon->acFileName = _cd_get_icon_path (pFileIcon);
					cd_message ("icon->acFileName : %s", icon->acFileName);
				}
				
				if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
					icon->fOrder = g_file_info_get_size (pFileInfo);
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
				{
					GTimeVal t;
					g_file_info_get_modification_time (pFileInfo, &t);
					icon->fOrder = t.tv_sec;
				}
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
					icon->fOrder = (cMimeType != NULL ? *((int *) cMimeType) : 0);
				pIconList = g_list_prepend (pIconList, icon);
			}
		}
	} while (TRUE);  // 'g_file_enumerator_close' est appelee lors du dernier 'g_file_enumerator_next_file'.
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
		pIconList = cairo_dock_sort_icons_by_name (pIconList);
	else
		pIconList = cairo_dock_sort_icons_by_order (pIconList);
	
	return pIconList;
}



static gchar *_cd_find_target_uri (const gchar *cBaseURI)
{
	GError *erreur = NULL;
	GFile *pFile = g_file_new_for_uri (cBaseURI);
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_object_unref (pFile);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	gchar *cTargetURI = g_strdup (g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI));
	g_object_unref (pFileInfo);
	return cTargetURI;
}
void vfs_backend_launch_uri (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	GError *erreur = NULL;
	gchar *cFullURI = (*cURI == '/' ? g_strconcat ("file://", cURI, NULL) : g_strdup (cURI));
	cd_message ("%s (%s)", __func__, cFullURI);
	
	gchar *cTargetURI = _cd_find_target_uri (cFullURI);
	gboolean bSuccess = g_app_info_launch_default_for_uri (cTargetURI != NULL ? cTargetURI : cFullURI,
		NULL,
		&erreur);
	g_free (cFullURI);
	g_free (cTargetURI);
	if (erreur != NULL)
	{
		cd_warning ("Attention : couldn't launch '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
}

GMount *_cd_find_mount_from_uri (const gchar *cURI, gchar **cTargetURI)
{
	cd_message ("%s (%s)", __func__, cURI);
	gchar *_cTargetURI = _cd_find_target_uri (cURI);
	
	GMount *pMount = NULL;
	if (_cTargetURI != NULL)
	{
		cd_message ("  pointe sur %s", _cTargetURI);
		GFile *file = g_file_new_for_uri (_cTargetURI);
		pMount = g_file_find_enclosing_mount (file, NULL, NULL);
		g_object_unref (file);
	}
	if (cTargetURI != NULL)
		*cTargetURI = _cTargetURI;
	else
		g_free (_cTargetURI);
	return pMount;
}

gchar *vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	cd_message ("%s (%s)", __func__, cURI);
	gchar *cTargetURI = NULL;
	GMount *pMount = _cd_find_mount_from_uri (cURI, &cTargetURI);
	cd_message (" cTargetURI : %s", cTargetURI);
	if (pMount != NULL)
		*bIsMounted = TRUE;
	else
	{
		if (cTargetURI != NULL && strcmp (cTargetURI, "file:///") == 0)  // cas particulier ?
			*bIsMounted = TRUE;
		else
			*bIsMounted = FALSE;
	}
	return cTargetURI;
}

static gchar * _cd_find_drive_name_from_URI (const gchar *cURI)
{
	g_return_val_if_fail (cURI != NULL, NULL);
	if (strncmp (cURI, "computer:///", 12) == 0)
	{
		gchar *cDriveName = g_strdup (cURI+12);
		gchar *str = strrchr (cDriveName, '.');
		if (str != NULL)
		{
			if (strcmp (str+1, "drive") == 0)
			{
				*str = '\0';
				while (1)
				{
					str = strchr (cDriveName, '\\');
					if (str == NULL)
						break;
					*str = '/';
				}
				return cDriveName;
			}
		}
		g_free (cDriveName);
	}
	return NULL;
}
gboolean vfs_backend_can_eject (const gchar *cURI)
{
	cd_message ("%s (%s)", __func__, cURI);
	gchar *cDriveName = _cd_find_drive_name_from_URI (cURI);
	if (cDriveName == NULL)
		return FALSE;
	
	gboolean bCanEject = _cd_find_can_eject_from_drive_name (cDriveName);
	//g_free (cDriveName);
	return bCanEject;
}
gboolean vfs_backend_eject_drive (const gchar *cURI)
{
	cd_message ("%s (%s)", __func__, cURI);
	gchar *cDriveName = _cd_find_drive_name_from_URI (cURI);
	GDrive *pDrive = _cd_find_drive_from_name (cDriveName);
	if (pDrive != NULL)
	{
		g_drive_eject (pDrive,
			G_MOUNT_UNMOUNT_NONE,
			NULL,
			NULL,
			NULL);
	}
	//g_object_unref (pDrive);
	//g_free (cDriveName);
	return TRUE;
}


static void _vfs_backend_mount_callback (gpointer pObject, GAsyncResult *res, gpointer *data)
//static void _vfs_backend_mount_callback (gboolean succeeded, char *error, char *detailed_error, gpointer *data)
{
	cd_message ("%s (%d)", __func__, GPOINTER_TO_INT (data[1]));
	
	CairoDockFMMountCallback pCallback = data[0];
	
	GError *erreur = NULL;
	gboolean bSuccess;
	if (GPOINTER_TO_INT (data[1]) == 1)
		bSuccess = (g_file_mount_mountable_finish (G_FILE (pObject), res, &erreur) != NULL);
		//bSuccess = (g_volume_mount_finish (G_VOLUME (pObject), res, &erreur));
	else if (GPOINTER_TO_INT (data[1]) == 0)
		bSuccess = g_mount_unmount_finish (G_MOUNT (pObject), res, &erreur);
	else
		bSuccess = g_mount_eject_finish (G_MOUNT (pObject), res, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	
	cd_message ("(un)mount fini -> %d", bSuccess);
	pCallback (GPOINTER_TO_INT (data[1]) == 1, bSuccess, data[2], data[3], data[4]);
	//g_free (data[2]);
	//g_object_unref (pObject);
	//g_free (data);
}

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	g_return_if_fail (iVolumeID > 0);
	cd_message ("%s (%s)", __func__, cURI);
	
	/*gchar *cTargetURI = NULL;
	GMount *pMount = _cd_find_mount_from_uri (cURI, &cTargetURI);
	cd_message (" %x / %s\n", pMount, cTargetURI);
	GVolume *pVolume = g_mount_get_volume (pMount);
	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (1);
	data2[2] = g_path_get_basename (cURI);
	data2[3] = icon;
	data2[4] = pDock;
	g_volume_mount (pVolume,
		G_MOUNT_MOUNT_NONE,
		NULL,
		NULL,
		(GAsyncReadyCallback) _vfs_backend_mount_callback,
		data2);*/
	gchar *cTargetURI = _cd_find_target_uri (cURI);
	GFile *pFile = g_file_new_for_uri (cURI);
	
	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (1);
	data2[2] = g_path_get_basename (cTargetURI);
	data2[3] = icon;
	data2[4] = pDock;
	g_file_mount_mountable  (pFile,
		G_MOUNT_MOUNT_NONE,
		NULL,
		NULL,
		(GAsyncReadyCallback) _vfs_backend_mount_callback,
		data2);
	g_free (cTargetURI);
}

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	gchar *cTargetURI = NULL;
	GMount *pMount = _cd_find_mount_from_uri (cURI, &cTargetURI);
	if (pMount == NULL || ! G_IS_MOUNT (pMount))
	{
		return ;
	}
	
	if ( ! g_mount_can_unmount (pMount))
		return ;
	
	gboolean bCanEject = g_mount_can_eject (pMount);
	gboolean bCanUnmount = g_mount_can_unmount (pMount);
	cd_message ("eject:%d / unmount:%d\n", bCanEject, bCanUnmount);
	if (! bCanEject && ! bCanUnmount)
		return ;
	
	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (bCanEject ? 2 : 0);
	data2[2] = g_mount_get_name (pMount);
	data2[3] = icon;
	data2[4] = pDock;
	if (bCanEject)
		g_mount_eject (pMount,
			G_MOUNT_UNMOUNT_NONE,
			NULL,
			(GAsyncReadyCallback) _vfs_backend_mount_callback,
			data2);
	else
		g_mount_unmount (pMount,
			G_MOUNT_UNMOUNT_NONE ,
			NULL,
			(GAsyncReadyCallback) _vfs_backend_mount_callback,
			data2);
}


static void _on_monitor_changed (GFileMonitor *monitor,
	GFile *file,
	GFile *other_file,
	GFileMonitorEvent event_type,
	gpointer  *data)
{
	CairoDockFMMonitorCallback pCallback = data[0];
	gpointer user_data = data[1];
	cd_message ("%s (%d , data : %x)", __func__, event_type, user_data);
	
	CairoDockFMEventType iEventType;
	switch (event_type)
	{
		case G_FILE_MONITOR_EVENT_CHANGED :
		case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT :
		//case G_FILE_MONITOR_EVENT_UNMOUNTED : // pertinent ?...
			iEventType = CAIRO_DOCK_FILE_MODIFIED;
			cd_message ("modification d'un fichier");
		break;
		
		case G_FILE_MONITOR_EVENT_DELETED :
			iEventType = CAIRO_DOCK_FILE_DELETED;
			cd_message ("effacement d'un fichier");
		break;
		
		case G_FILE_MONITOR_EVENT_CREATED :
			iEventType = CAIRO_DOCK_FILE_CREATED;
			cd_message ("creation d'un fichier");
		break;
		
		default :
		return ;
	}
	gchar *cURI = g_file_get_uri (file);
	cd_message (" c'est le fichier %s", cURI);
	gchar *cPath = NULL;
	if (strncmp (cURI, "computer://", 11) == 0)
	{
		if (event_type == G_FILE_MONITOR_EVENT_CHANGED)
		{
			g_free (cURI);
			return ;
		}
		memcpy (cURI+4, "file", 4);
		cPath = g_filename_from_uri (cURI+4, NULL, NULL);
		cd_debug(" (path:%s)", cPath);
		g_free (cURI);
		cURI = g_strdup_printf ("computer://%s", cPath);
		cd_message ("son URI complete est : %s", cURI);
	}
	
	pCallback (iEventType, cURI, user_data);
	g_free (cURI);
}


void vfs_backend_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer user_data)
{
	g_return_if_fail (cURI != NULL);
	GError *erreur = NULL;
	GFileMonitor *pMonitor;
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	if (bDirectory)
		pMonitor = g_file_monitor_directory (pFile,
			G_FILE_MONITOR_WATCH_MOUNTS,
			NULL,
			&erreur);
	else
		pMonitor = g_file_monitor_file (pFile,
			G_FILE_MONITOR_WATCH_MOUNTS,
			NULL,
			&erreur);
	//g_object_unref (pFile);
	if (erreur != NULL)
	{
		cd_warning ("Attention : couldn't add monitor on '%s' (%d) [%s]", cURI, bDirectory, erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	gpointer *data = g_new0 (gpointer, 3);
	data[0] = pCallback;
	data[1] = user_data;
	data[2] = pMonitor;
	g_signal_connect (G_OBJECT (pMonitor), "changed", G_CALLBACK (_on_monitor_changed), data);
	
	g_hash_table_insert (s_hMonitorHandleTable, g_strdup (cURI), data);
	cd_message (">>> moniteur ajoute sur %s (%x)", cURI, user_data);
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
	g_return_val_if_fail (cURI != NULL, FALSE);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	
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
	g_return_val_if_fail (cOldURI != NULL, FALSE);
	GFile *pOldFile = (*cOldURI == '/' ? g_file_new_for_path (cOldURI) : g_file_new_for_uri (cOldURI));
	GError *erreur = NULL;
	GFile *pNewFile = g_file_set_display_name (pOldFile, cNewName, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	gboolean bSuccess = (pNewFile != NULL);
	if (pNewFile != NULL)
		g_object_unref (pNewFile);
	g_object_unref (pOldFile);
	return bSuccess;
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	cd_message (" %s -> %s", cURI, cDirectoryURI);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	
	gchar *cFileName = g_file_get_basename (pFile);
	gchar *cNewFileURI = g_strconcat (cDirectoryURI, "/", cFileName, NULL);  // un peu moyen mais bon...
	GFile *pDestinationFile = (*cNewFileURI == '/' ? g_file_new_for_path (cNewFileURI) : g_file_new_for_uri (cNewFileURI));
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
	g_return_if_fail (cURI != NULL);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	GError *erreur = NULL;
	gchar *cQuery = g_strconcat (G_FILE_ATTRIBUTE_STANDARD_SIZE, ",", G_FILE_ATTRIBUTE_TIME_MODIFIED, ",", G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, ",", G_FILE_ATTRIBUTE_UNIX_UID, ",", G_FILE_ATTRIBUTE_UNIX_GID, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_READ, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, ",", G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, NULL);
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	g_free (cQuery);
	if (erreur != NULL)
	{
		cd_warning ("Attention : couldn't get file properties for '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
	
	*iSize = g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);
	*iLastModificationTime = (time_t) g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
	*cMimeType = g_file_info_get_attribute_as_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	*iUID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_UID);
	*iGID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_GID);
	int r = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
	int w = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
	int x = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
	*iPermissionsMask = r * 8 * 8 + w * 8 + x;
	
	g_object_unref (pFileInfo);
	g_object_unref (pFile);
}


gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gchar **cFileInfoPath)
{
	gchar *cPath = NULL;
	/*GFile *pFile = g_file_new_for_uri ("trash://");
	gchar *cPath = g_file_get_path (pFile);
	g_object_unref (pFile);*/
	const gchar *xdgPath = g_getenv ("XDG_DATA_HOME");
	if (xdgPath != NULL)
	{
		cPath = g_strdup_printf ("%s/Trash/files", xdgPath);
		if (cFileInfoPath != NULL)
			*cFileInfoPath = g_strdup_printf ("%s/Trash/info", xdgPath);
	}
	else
	{
		cPath = g_strdup_printf ("%s/.local/share/Trash/files", g_getenv ("HOME"));
		if (cFileInfoPath != NULL)
			*cFileInfoPath = g_strdup_printf ("%s/.local/share/Trash/info", g_getenv ("HOME"));
	}
	return cPath;
}

gchar *vfs_backend_get_desktop_path (void)
{
	GFile *pFile = g_file_new_for_uri ("desktop://");
	gchar *cPath = g_file_get_path (pFile);
	g_object_unref (pFile);
	return cPath;
}

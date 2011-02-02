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

// If Gio is not detected, do not try to compile this file.
// Note: Gio is present from GLib >= 2.16
#include "cairo-dock-gio-vfs.h"

#ifdef HAVE_LIBGIO
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gio/gio.h>
#define G_VFS_DBUS_DAEMON_NAME "org.gtk.vfs.Daemon"

#include <cairo-dock.h>

static void _cairo_dock_gio_vfs_empty_dir (const gchar *cBaseURI);

static GHashTable *s_hMonitorHandleTable = NULL;

static void _gio_vfs_free_monitor_data (gpointer *data)
{
	if (data != NULL)
	{
		GFileMonitor *pHandle = data[2];
		g_file_monitor_cancel (pHandle);  // le GFileMonitor est-il libere lors du g_file_monitor_cancel () ?
		g_free (data);
	}
}

gboolean cairo_dock_gio_vfs_init (void)
{
	// first, check that the session has gvfs on DBus
	if( !cairo_dock_dbus_is_enabled() ||
	    !cairo_dock_dbus_detect_application (G_VFS_DBUS_DAEMON_NAME) )
	{
		cd_warning("VFS Deamon NOT found on DBus !");
	  return FALSE;
	}
	cd_message("VFS Deamon found on DBus.");
	
	
	if (s_hMonitorHandleTable != NULL)
		g_hash_table_destroy (s_hMonitorHandleTable);
	
	s_hMonitorHandleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		(GDestroyNotify) _gio_vfs_free_monitor_data);
	
	GVfs *vfs = g_vfs_get_default ();
	return (vfs != NULL && g_vfs_is_active (vfs));  // utile ?
}

static void cairo_dock_gio_vfs_stop (void)
{
	if (s_hMonitorHandleTable != NULL)
	{
		g_hash_table_destroy (s_hMonitorHandleTable);
		s_hMonitorHandleTable = NULL;
	}
}


static gchar *_cd_get_icon_path (GIcon *pIcon, const gchar *cTargetURI)  // cTargetURI est l'URI que represente l'icone, pour les cas ou l'icone est contenue dans le repertoire lui-meme (CD ou DVD de jeux notamment)
{
	//g_print ("%s ()\n", __func__);
	gchar *cIconPath = NULL;
	if (G_IS_THEMED_ICON (pIcon))
	{
		const gchar * const *cFileNames = g_themed_icon_get_names (G_THEMED_ICON (pIcon));
		//cd_message ("icones possibles : %s\n", g_strjoinv (":", (gchar **) cFileNames));
		int i;
		for (i = 0; cFileNames[i] != NULL && cIconPath == NULL; i ++)
		{
			//g_print (" une icone possible est : %s\n", cFileNames[i]);
			cIconPath = cairo_dock_search_icon_s_path (cFileNames[i]);
			//g_print ("  chemin trouve : %s\n", cIconPath);
		}
	}
	else if (G_IS_FILE_ICON (pIcon))
	{
		GFile *pFile = g_file_icon_get_file (G_FILE_ICON (pIcon));
		cIconPath = g_file_get_basename (pFile);
		//g_print (" file_icon => %s\n", cIconPath);
		
		if (cTargetURI && cIconPath && g_str_has_suffix (cIconPath, ".ico"))  // cas des montages de CD ou d'iso
		{
			gchar *tmp = cIconPath;
			cIconPath = g_strdup_printf ("%s/%s", cTargetURI, tmp);
			g_free (tmp);
			if (strncmp (cIconPath, "file://", 7) == 0)
			{
				tmp = cIconPath;
				cIconPath = g_filename_from_uri (tmp, NULL, NULL);
				g_free (tmp);
			}
		}
	}
	return cIconPath;
}


static void _cd_find_mount_from_volume_name (const gchar *cVolumeName, GMount **pFoundMount, gchar **cURI, gchar **cIconName)
{
	g_return_if_fail (cVolumeName != NULL);
	cd_message ("%s (%s)", __func__, cVolumeName);
	GFile *pFile = g_file_new_for_uri ("computer://");
	GError *erreur = NULL;
	const gchar *cAttributes = G_FILE_ATTRIBUTE_STANDARD_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_STANDARD_ICON","
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE;
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_error_free (erreur);
		g_object_unref (pFile);
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
			cd_warning ("gnome_integration : %s", erreur->message);
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
						*cIconName = _cd_get_icon_path (pSystemIcon, NULL);
						g_free (cName);
						break ;
					}
					g_free (cName);
				}
			}
			g_object_unref (pFileInfo);
		}
	} while (TRUE);
	g_object_unref (pFileEnum);
	g_object_unref (pFile);
}

static GDrive *_cd_find_drive_from_name (const gchar *cName)
{
	g_return_val_if_fail (cName != NULL, NULL);
	cd_message ("%s (%s)", __func__, cName);
	GVolumeMonitor *pVolumeMonitor = g_volume_monitor_get ();
	GDrive *pFoundDrive = NULL;
	
	gchar *str = strrchr (cName, '-');
	if (str)
		*str = '\0';
	
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
			g_free (cDriveName);
		}
		else
			g_object_unref (pDrive);
	}
	g_list_free (pDrivesList);
	if (str)
		*str = '-';
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
	if (pAssociatedVolumes == NULL)
		return NULL;
	
	int iNumVolume;
	gchar *str = strrchr (cName, '-');
	if (str)
	{
		iNumVolume = atoi (str+1);
	}
	else
		iNumVolume = 0;
	
	GVolume *pVolume = g_list_nth_data (pAssociatedVolumes, iNumVolume);
	if (pVolume != NULL)
	{
		cVolumeName = g_volume_get_name (pVolume);
	}
	cd_debug ("%dth volume -> cVolumeName : %s\n", iNumVolume, cVolumeName);
	
	cd_debug ("Pour info, la liste des volumes disponibles sur ce disque est :");
	GList *av;
	for (av = pAssociatedVolumes; av != NULL; av = av->next)
	{
		pVolume = av->data;
		cd_debug ("  - %s", g_volume_get_name  (pVolume));
	}
	
	g_list_foreach (pAssociatedVolumes, (GFunc)g_object_unref, NULL);
	g_list_free (pAssociatedVolumes);
	
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

static void cairo_dock_gio_vfs_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
{
	*cName = NULL;
	*cURI = NULL;
	*cIconName = NULL;
	g_return_if_fail (cBaseURI != NULL);
	GError *erreur = NULL;
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	// make it a valid URI.
	gchar *cValidUri;
	if (strncmp (cBaseURI, "x-nautilus-desktop://", 21) == 0)  // shortcut on the desktop (nautilus)
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
		cValidUri = g_filename_from_uri (cNautilusFile, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			return ;
		}
		gchar *cVolumeName = cValidUri + 1;  // on saute le '/'.
		cd_message ("cVolumeName : %s", cVolumeName);
		
		GMount *pMount = NULL;
		_cd_find_mount_from_volume_name (cVolumeName, &pMount, cURI, cIconName);
		g_return_if_fail (pMount != NULL);
		
		*cName = g_strdup (cVolumeName);
		*bIsDirectory = TRUE;
		*iVolumeID = 1;
		*fOrder = 0;
		//g_object_unref (pMount);
		
		g_free (cValidUri);
		//g_free (cNautilusFile);
		return;
	}
	else  // normal file
	{
		if (*cBaseURI == '/')
			cValidUri = g_filename_to_uri (cBaseURI, NULL, NULL);
		else
			cValidUri = g_strdup (cBaseURI);
		if (*cBaseURI == ':' || *cValidUri == ':')  // cas bizarre au demontage d'un signet ftp quand celui-ci n'est pas accessible plantage dans dbus).
		{
			cd_warning ("invalid URI (%s ; %s), skip it", cBaseURI, cValidUri);
			g_free (cValidUri);
			return;
		}
	}
	
	// get its attributes.
	GFile *pFile = g_file_new_for_uri (cValidUri);
	g_return_if_fail (pFile);
	const gchar *cQuery = G_FILE_ATTRIBUTE_STANDARD_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_SIZE","
		G_FILE_ATTRIBUTE_TIME_MODIFIED","
		G_FILE_ATTRIBUTE_TIME_ACCESS","
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_STANDARD_ICON","
		G_FILE_ATTRIBUTE_THUMBNAIL_PATH","
		#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 20)
		///G_FILE_ATTRIBUTE_PREVIEW_ICON","
		#endif
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE;
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NONE,  /// G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS
		NULL,
		&erreur);
	//g_object_unref (pFile);
	if (erreur != NULL)  // peut arriver si l'emplacement n'est pas monte.
	{
		cd_debug ("gnome_integration : %s", erreur->message);  // inutile d'en faire un warning.
		g_error_free (erreur);
		g_free (cValidUri);
		g_object_unref (pFile);
		return ;
	}
	
	const gchar *cFileName = g_file_info_get_name (pFileInfo);
	const gchar *cMimeType = g_file_info_get_content_type (pFileInfo);
	GFileType iFileType = g_file_info_get_file_type (pFileInfo);
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
	{
		GTimeVal t;
		g_file_info_get_modification_time (pFileInfo, &t);
		*fOrder = t.tv_sec;
	}
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_ACCESS)
		*fOrder =  g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
		*fOrder = g_file_info_get_size (pFileInfo);
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
		*fOrder = (cMimeType != NULL ? *((int *) cMimeType) : 0);
	else
		*fOrder = 0;
	
	*bIsDirectory = (iFileType == G_FILE_TYPE_DIRECTORY);
	cd_message (" => '%s' (mime:%s ; bIsDirectory:%d)", cFileName, cMimeType, *bIsDirectory);
	
	const gchar *cTargetURI = g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
	
	// if it's a mount point, find a readable name.
	if (iFileType == G_FILE_TYPE_MOUNTABLE)
	{
		*cName = NULL;
		*iVolumeID = 1;
		
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
	
	if (cTargetURI)
	{
		*cURI = g_strdup (cTargetURI);
		g_free (cValidUri);
		cValidUri = NULL;
	}
	else
		*cURI = cValidUri;
	
	// find an icon.
	*cIconName = NULL;
	*cIconName = g_strdup (g_file_info_get_attribute_byte_string (pFileInfo, G_FILE_ATTRIBUTE_THUMBNAIL_PATH));
	#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 20)
	/**if (*cIconName == NULL)
	{
		GIcon *pPreviewIcon = (GIcon *)g_file_info_get_attribute_object (pFileInfo, G_FILE_ATTRIBUTE_PREVIEW_ICON);
		if (pPreviewIcon != NULL)
		{
			*cIconName = _cd_get_icon_path (pPreviewIcon, NULL);
			//g_print ("got preview icon '%s'\n", *cIconName);
		}
	}*/
	#endif
	if (*cIconName == NULL && cMimeType != NULL && strncmp (cMimeType, "image", 5) == 0)
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
			*cIconName = _cd_get_icon_path (pSystemIcon, cTargetURI ? cTargetURI : *cURI);
		}
	}
	cd_message ("cIconName : %s", *cIconName);
	
	//*iVolumeID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE);
	//cd_message ("ID : %d\n", *iVolumeID);
	g_object_unref (pFileInfo);
}

static Icon *_cd_get_icon_for_volume (GVolume *pVolume, GMount *pMount)
{
	GIcon *pIcon;
	GFile *pRootDir;
	gchar *cName, *cCommand, *cFileName;
	if (pVolume != NULL)
		pMount = g_volume_get_mount (pVolume);
	else if (pMount == NULL)
		return NULL;
	
	if (pMount != NULL)  // ce volume est monte.
	{
		cName = g_mount_get_name (pMount);
		
		pRootDir = g_mount_get_root (pMount);
		cCommand = g_file_get_uri (pRootDir);
		
		pIcon = g_mount_get_icon (pMount);
		cFileName = _cd_get_icon_path (pIcon, NULL);
		
		g_object_unref (pRootDir);
		g_object_unref (pIcon);
		g_object_unref (pMount);
	}
	else  // ce volume est demonte, on le montre quand meme (l'automount peut etre off).
	{
		cName = g_volume_get_name (pVolume);
		
		pIcon = g_volume_get_icon (pVolume);
		cFileName = _cd_get_icon_path (pIcon, NULL);
		
		cCommand = g_strdup (cName);
		
		g_object_unref (pIcon);
	}
	
	Icon *pNewIcon = cairo_dock_create_dummy_launcher (cName,
		cFileName,
		cCommand,
		NULL,
		0);
	pNewIcon->iTrueType = CAIRO_DOCK_ICON_TYPE_FILE;
	pNewIcon->iVolumeID = 1;
	pNewIcon->cBaseURI = g_strdup (pNewIcon->cCommand);
	cd_message (" => %s", pNewIcon->cCommand);
	return pNewIcon;
}

static GList *cairo_dock_gio_vfs_list_volumes (void)
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

static GList *cairo_dock_gio_vfs_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsGroup, gboolean bListHiddenFiles, int iNbMaxFiles, gchar **cValidUri)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	cd_message ("%s (%s)", __func__, cBaseURI);
	
	gchar *cURI;
	gboolean bAddHome = FALSE;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
	{
		cURI = g_strdup ("computer://");
		bAddHome = TRUE;
		///*cValidUri = cURI;
		///return cairo_dock_gio_vfs_list_volumes ();
		//cairo_dock_gio_vfs_list_volumes ();
	}
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
		cURI = g_strdup ("network://");
	else
		cURI = (*cBaseURI == '/' ? g_strconcat ("file://", cBaseURI, NULL) : g_strdup (cBaseURI));
	*cValidUri = cURI;
	
	GFile *pFile = g_file_new_for_uri (cURI);
	GError *erreur = NULL;
	const gchar *cAttributes = G_FILE_ATTRIBUTE_STANDARD_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_SIZE","
		G_FILE_ATTRIBUTE_TIME_MODIFIED","
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN","
		G_FILE_ATTRIBUTE_STANDARD_ICON","
		G_FILE_ATTRIBUTE_THUMBNAIL_PATH","
		#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 20)
		///G_FILE_ATTRIBUTE_PREVIEW_ICON","
		#endif
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
		G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE;
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NONE,  /// G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_error_free (erreur);
		g_object_unref (pFile);
		return NULL;
	}
	
	int iOrder = 0;
	int iNbFiles = 0;
	GList *pIconList = NULL;
	Icon *icon;
	GFileInfo *pFileInfo;
	do
	{
		pFileInfo = g_file_enumerator_next_file (pFileEnum, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
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
			
			icon = cairo_dock_create_dummy_launcher (NULL, NULL, NULL, NULL, 0);
			icon->iTrueType = CAIRO_DOCK_ICON_TYPE_FILE;
			icon->iGroup = iNewIconsGroup;
			icon->cBaseURI = g_strconcat (*cValidUri, "/", cFileName, NULL);
			//g_print	 ("+ %s (mime:%s)n", icon->cBaseURI, cMimeType);
			
			if (iFileType == G_FILE_TYPE_MOUNTABLE)
			{
				const gchar *cTargetURI = g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
				cd_message ("  c'est un point de montage correspondant a %s", cTargetURI);
				
				GMount *pMount = NULL;
				if (cTargetURI != NULL)
				{
					icon->cCommand = g_strdup (cTargetURI);
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
								//g_free (cVolumeName);
								//continue;  /// apparemment il n'est plus necessaire d'afficher les .drives qui ont 1 (ou plusieurs ?) volumes, car ces derniers sont dans la liste, donc ca fait redondant.
								/**if (strcmp (cVolumeName, "discard") == 0)
									continue;
								g_free (cName);
								cName = cVolumeName;*/
							}
						}
					}
				}
				icon->iVolumeID = 1;
				cd_message ("le nom de ce volume est : %s", cName);
			}
			else
			{
				if (iFileType == G_FILE_TYPE_DIRECTORY)
					icon->iVolumeID = -1;
				cName = g_strdup (cFileName);
			}
			
			if (icon->cCommand == NULL)
				icon->cCommand = g_strdup (icon->cBaseURI);
			icon->cName = cName;
			icon->cFileName = NULL;
			icon->cFileName = g_strdup (g_file_info_get_attribute_byte_string (pFileInfo, G_FILE_ATTRIBUTE_THUMBNAIL_PATH));
			#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 20)
			/**if (icon->cFileName == NULL)
			{
				GIcon *pPreviewIcon = (GIcon *)g_file_info_get_attribute_object (pFileInfo, G_FILE_ATTRIBUTE_PREVIEW_ICON);
				if (pPreviewIcon != NULL)
				{
					icon->cFileName = _cd_get_icon_path (pPreviewIcon, NULL);
					g_print ("got preview icon '%s'\n", icon->cFileName);
				}
			}*/
			#endif
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
					icon->cFileName = g_strdup (cFilePath);
					cairo_dock_remove_html_spaces (icon->cFileName);
				}

				g_free (cHostname);
				g_free (cFilePath);
			}
			if (icon->cFileName == NULL)
			{
				icon->cFileName = _cd_get_icon_path (pFileIcon, icon->cCommand);
				//g_print ("icon->cFileName : %s\n", icon->cFileName);
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
			if (icon->fOrder == 0)  // un peu moyen mais mieux que rien non ?
				icon->fOrder = iOrder;
			/*pIconList = g_list_insert_sorted (pIconList,
				icon,
				(GCompareFunc) cairo_dock_compare_icons_order);*/
			pIconList = g_list_prepend (pIconList, icon);
			cd_debug (" + %s (%s)", icon->cName, icon->cFileName);
			iOrder ++;
			iNbFiles ++;
		}
		g_object_unref (pFileInfo);
	} while (iNbFiles < iNbMaxFiles);
	
	g_object_unref (pFileEnum);
	g_object_unref (pFile);
	
	if (bAddHome && pIconList != NULL)
	{
		Icon *pRootIcon = cairo_dock_get_icon_with_name (pIconList, "/");
		if (pRootIcon == NULL)
		{
			pRootIcon = cairo_dock_get_first_icon (pIconList);
			cd_debug ("domage ! (%s:%s)\n", pRootIcon->cCommand, pRootIcon->cName);
		}
		
		icon = cairo_dock_create_dummy_launcher (g_strdup ("home"),
			g_strdup (pRootIcon->cFileName),
			g_strdup ("/home"),
			NULL,
			iOrder++);
		icon->iTrueType = CAIRO_DOCK_ICON_TYPE_FILE;
		icon->iGroup = iNewIconsGroup;
		icon->cBaseURI = g_strdup_printf ("file://%s", "/home");
		icon->iVolumeID = 0;
		
		pIconList = g_list_insert_sorted (pIconList,
			icon,
			(GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	if (iSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
		pIconList = cairo_dock_sort_icons_by_name (pIconList);
	else
		pIconList = cairo_dock_sort_icons_by_order (pIconList);
	
	return pIconList;
}

static gsize cairo_dock_gio_vfs_measure_directory (const gchar *cBaseURI, gint iCountType, gboolean bRecursive, gint *pCancel)
{
	g_return_val_if_fail (cBaseURI != NULL, 0);
	//cd_debug ("%s (%s)", __func__, cBaseURI);
	
	gchar *cURI = (*cBaseURI == '/' ? g_strconcat ("file://", cBaseURI, NULL) : (gchar*)cBaseURI);  // on le libere a la fin si necessaire.
	
	GFile *pFile = g_file_new_for_uri (cURI);
	GError *erreur = NULL;
	const gchar *cAttributes = G_FILE_ATTRIBUTE_STANDARD_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_SIZE","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_STANDARD_TARGET_URI;
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_error_free (erreur);
		g_object_unref (pFile);
		return 0;
	}
	
	gsize iMeasure = 0;
	GFileInfo *pFileInfo;
	GString *sFilePath = g_string_new ("");
	do
	{
		pFileInfo = g_file_enumerator_next_file (pFileEnum, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
		if (pFileInfo == NULL)
			break ;
		
		const gchar *cFileName = g_file_info_get_name (pFileInfo);
		
		g_string_printf (sFilePath, "%s/%s", cURI, cFileName);
		GFile *file = g_file_new_for_uri (sFilePath->str);
		const gchar *cTargetURI = g_file_get_uri (file);
		//g_print ("+ %s [%s]\n", cFileName, cTargetURI);
		GFileType iFileType = g_file_info_get_file_type (pFileInfo);
		
		if (iFileType == G_FILE_TYPE_DIRECTORY && bRecursive)
		{
			g_string_printf (sFilePath, "%s/%s", cURI, cFileName);
			iMeasure += MAX (1, cairo_dock_gio_vfs_measure_directory (sFilePath->str, iCountType, bRecursive, pCancel));  // un repertoire vide comptera pour 1.
		}
		else
		{
			if (iCountType == 1)  // measure size.
			{
				iMeasure += g_file_info_get_size (pFileInfo);
			}
			else  // measure nb files.
			{
				iMeasure ++;
			}
		}
		g_object_unref (pFileInfo);
	} while (! g_atomic_int_get (pCancel));
	if (*pCancel)
		cd_debug ("mesure annulee");
	
	g_object_unref (pFileEnum);
	g_object_unref (pFile);
	g_string_free (sFilePath, TRUE);
	if (cURI != cBaseURI)
		g_free (cURI);
	
	return iMeasure;
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
		cd_debug ("%s (%s) : %s", __func__, cBaseURI, erreur->message);  // peut arriver avec un .mount, donc pas de warning.
		g_error_free (erreur);
		return NULL;
	}
	gchar *cTargetURI = g_strdup (g_file_info_get_attribute_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI));
	g_object_unref (pFileInfo);
	return cTargetURI;
}

static void cairo_dock_gio_vfs_launch_uri (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	GError *erreur = NULL;
	gchar *cValidUri = (*cURI == '/' ? g_strconcat ("file://", cURI, NULL) : g_strdup (cURI));
	cd_message ("%s (%s)", __func__, cValidUri);
	
	gchar *cTargetURI = _cd_find_target_uri (cValidUri);
	gboolean bSuccess = g_app_info_launch_default_for_uri (cTargetURI != NULL ? cTargetURI : cValidUri,
		NULL,
		&erreur);
	g_free (cValidUri);
	g_free (cTargetURI);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : couldn't launch '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
}

static GMount *_cd_find_mount_from_uri (const gchar *cURI, gchar **cTargetURI)
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

static gchar *cairo_dock_gio_vfs_is_mounted (const gchar *cURI, gboolean *bIsMounted)
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
static gboolean cairo_dock_gio_vfs_can_eject (const gchar *cURI)
{
	cd_message ("%s (%s)", __func__, cURI);
	gchar *cDriveName = _cd_find_drive_name_from_URI (cURI);
	if (cDriveName == NULL)
		return FALSE;
	
	gboolean bCanEject = _cd_find_can_eject_from_drive_name (cDriveName);
	//g_free (cDriveName);
	return bCanEject;
}
static gboolean cairo_dock_gio_vfs_eject_drive (const gchar *cURI)
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


static void _gio_vfs_mount_callback (gpointer pObject, GAsyncResult *res, gpointer *data)
//static void _gio_vfs_mount_callback (gboolean succeeded, char *error, char *detailed_error, gpointer *data)
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
		cd_warning ("gnome-integration : %s", erreur->message);
		g_error_free (erreur);
	}
	
	cd_message ("(un)mounted -> %d", bSuccess);
	if (pCallback != NULL)
		pCallback (GPOINTER_TO_INT (data[1]) == 1, bSuccess, data[2], data[3], data[4]);
	g_free (data[2]);
	g_free (data[3]);
	g_free (data);
}

static void cairo_dock_gio_vfs_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	gchar *cTargetURI = _cd_find_target_uri (cURI);
	GFile *pFile = g_file_new_for_uri (cURI);
	
	gpointer *data = g_new (gpointer, 5);  // libere dans la callback.
	data[0] = pCallback;
	data[1] = GINT_TO_POINTER (1);  // mount
	data[2] = (cTargetURI ? g_path_get_basename (cTargetURI) : g_strdup (cURI));
	data[3] = g_strdup (cURI);
	data[4] = user_data;
	g_file_mount_mountable  (pFile,
		G_MOUNT_MOUNT_NONE,
		NULL,
		NULL,
		(GAsyncReadyCallback) _gio_vfs_mount_callback,
		data);
	g_free (cTargetURI);
}

static void cairo_dock_gio_vfs_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, gpointer user_data)
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
	{
		cd_warning ("can't unmount this volume (%s)", cURI);
		return ;
	}
	
	gpointer *data = g_new (gpointer, 5);
	data[0] = pCallback;
	data[1] = GINT_TO_POINTER (bCanEject ? 2 : 0);
	data[2] = g_mount_get_name (pMount);
	data[3] = g_strdup (cURI);
	data[4] = user_data;
	if (bCanEject)
		g_mount_eject (pMount,
			G_MOUNT_UNMOUNT_NONE,
			NULL,
			(GAsyncReadyCallback) _gio_vfs_mount_callback,
			data);
	else
		g_mount_unmount (pMount,
			G_MOUNT_UNMOUNT_NONE ,
			NULL,
			(GAsyncReadyCallback) _gio_vfs_mount_callback,
			data);
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
		///case G_FILE_MONITOR_EVENT_CHANGED :  // ignorer celui-ci devrait permettre d'eviter la moitie des signaux inutiles que gvfs emet.
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


static void cairo_dock_gio_vfs_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer user_data)
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
		cd_warning ("gnome-integration : couldn't add monitor on '%s' (%d) [%s]", cURI, bDirectory, erreur->message);
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

static void cairo_dock_gio_vfs_remove_monitor (const gchar *cURI)
{
	if (cURI != NULL)
	{
		cd_message (">>> moniteur supprime sur %s", cURI);
		g_hash_table_remove (s_hMonitorHandleTable, cURI);
	}
}



static gboolean cairo_dock_gio_vfs_delete_file (const gchar *cURI, gboolean bNoTrash)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	
	GError *erreur = NULL;
	gboolean bSuccess;
	if (bNoTrash)
	{
		const gchar *cQuery = G_FILE_ATTRIBUTE_STANDARD_TYPE;
		GFileInfo *pFileInfo = g_file_query_info (pFile,
			cQuery,
			G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
			NULL,
			&erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			g_object_unref (pFile);
			return FALSE;
		}
		
		GFileType iFileType = g_file_info_get_file_type (pFileInfo);
		if (iFileType == G_FILE_TYPE_DIRECTORY)
		{
			_cairo_dock_gio_vfs_empty_dir (cURI);
		}
		
		bSuccess = g_file_delete (pFile, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome-integration : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	else
	{
		bSuccess = g_file_trash (pFile, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome-integration : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	g_object_unref (pFile);
	return bSuccess;
}

static gboolean cairo_dock_gio_vfs_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	g_return_val_if_fail (cOldURI != NULL, FALSE);
	GFile *pOldFile = (*cOldURI == '/' ? g_file_new_for_path (cOldURI) : g_file_new_for_uri (cOldURI));
	GError *erreur = NULL;
	GFile *pNewFile = g_file_set_display_name (pOldFile, cNewName, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome-integration : %s", erreur->message);
		g_error_free (erreur);
	}
	gboolean bSuccess = (pNewFile != NULL);
	if (pNewFile != NULL)
		g_object_unref (pNewFile);
	g_object_unref (pOldFile);
	return bSuccess;
}

static gboolean cairo_dock_gio_vfs_move_file (const gchar *cURI, const gchar *cDirectoryURI)
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
		cd_warning ("gnome-integration : %s", erreur->message);
		g_error_free (erreur);
	}
	g_object_unref (pFile);
	g_object_unref (pDestinationFile);
	return bSuccess;
}

static gboolean cairo_dock_gio_vfs_create_file (const gchar *cURI, gboolean bDirectory)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	
	GError *erreur = NULL;
	gboolean bSuccess = TRUE;
	#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 18)
	if (bDirectory)
		g_file_make_directory_with_parents (pFile, NULL, &erreur);
	else
	#endif
		g_file_create (pFile, G_FILE_CREATE_PRIVATE, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome-integration : %s", erreur->message);
		g_error_free (erreur);
		bSuccess = FALSE;
	}
	g_object_unref (pFile);
	
	return bSuccess;
}

static void cairo_dock_gio_vfs_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	g_return_if_fail (cURI != NULL);
	GFile *pFile = (*cURI == '/' ? g_file_new_for_path (cURI) : g_file_new_for_uri (cURI));
	GError *erreur = NULL;
	const gchar *cQuery = G_FILE_ATTRIBUTE_STANDARD_SIZE","
		G_FILE_ATTRIBUTE_TIME_MODIFIED","
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
		G_FILE_ATTRIBUTE_UNIX_UID","
		G_FILE_ATTRIBUTE_UNIX_GID","
		G_FILE_ATTRIBUTE_ACCESS_CAN_READ","
		G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE","
		G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE;
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NONE,  /// G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome-integration : couldn't get file properties for '%s' [%s]", cURI, erreur->message);
		g_error_free (erreur);
	}
	
	*iSize = g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);
	*iLastModificationTime = (time_t) g_file_info_get_attribute_uint64 (pFileInfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
	*cMimeType = g_file_info_get_attribute_as_string (pFileInfo, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	*iUID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_UID);
	*iGID = g_file_info_get_attribute_uint32 (pFileInfo, G_FILE_ATTRIBUTE_UNIX_GID);
	gboolean r = g_file_info_get_attribute_boolean (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
	gboolean w = g_file_info_get_attribute_boolean (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
	gboolean x = g_file_info_get_attribute_boolean (pFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
	*iPermissionsMask = r * 8 * 8 + w * 8 + x;
	
	g_object_unref (pFileInfo);
	g_object_unref (pFile);
}


static gchar *cairo_dock_gio_vfs_get_trash_path (const gchar *cNearURI, gchar **cFileInfoPath)
{
	if (cNearURI == NULL)
		return g_strdup ("trash://");
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

static gchar *cairo_dock_gio_vfs_get_desktop_path (void)
{
	GFile *pFile = g_file_new_for_uri ("desktop://");
	gchar *cPath = g_file_get_path (pFile);
	g_object_unref (pFile);
	return cPath;
}

static void _cairo_dock_gio_vfs_empty_dir (const gchar *cBaseURI)
{
	if (cBaseURI == NULL)
		return ;
	
	GFile *pFile = (*cBaseURI == '/' ? g_file_new_for_path (cBaseURI) : g_file_new_for_uri (cBaseURI));
	GError *erreur = NULL;
	const gchar *cAttributes = G_FILE_ATTRIBUTE_STANDARD_TYPE","
		G_FILE_ATTRIBUTE_STANDARD_NAME;
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_object_unref (pFile);
		g_error_free (erreur);
		return ;
	}
	
	GString *sFileUri = g_string_new ("");
	GFileInfo *pFileInfo;
	GFile *file;
	do
	{
		pFileInfo = g_file_enumerator_next_file (pFileEnum, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
		if (pFileInfo == NULL)
			break ;
		
		GFileType iFileType = g_file_info_get_file_type (pFileInfo);
		const gchar *cFileName = g_file_info_get_name (pFileInfo);
		
		g_string_printf (sFileUri, "%s/%s", cBaseURI, cFileName);
		if (iFileType == G_FILE_TYPE_DIRECTORY)
		{
			_cairo_dock_gio_vfs_empty_dir (sFileUri->str);
		}
		
		file = (*cBaseURI == '/' ? g_file_new_for_path (sFileUri->str) : g_file_new_for_uri (sFileUri->str));
		g_file_delete (file, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		g_object_unref (file);
		
		g_object_unref (pFileInfo);
	} while (1);
	
	g_string_free (sFileUri, TRUE);
	g_object_unref (pFileEnum);
	g_object_unref (pFile);
}

static inline int _convert_base16 (char c)
{
	int x;
	if (c >= '0' && c <= '9')
		x = c - '0';
	else
		x = 10 + (c - 'A');
	return x;
}
static void cairo_dock_gio_vfs_empty_trash (void)
{
	GFile *pFile = g_file_new_for_uri ("trash://");
	GError *erreur = NULL;
	const gchar *cAttributes = G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_STANDARD_TYPE;
	GFileEnumerator *pFileEnum = g_file_enumerate_children (pFile,
		cAttributes,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
		NULL,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_object_unref (pFile);
		g_error_free (erreur);
		return ;
	}
	
	GString *sFileUri = g_string_new ("");
	GFileInfo *pFileInfo;
	GFile *file;
	do
	{
		pFileInfo = g_file_enumerator_next_file (pFileEnum, NULL, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
		if (pFileInfo == NULL)
			break ;
		
		const gchar *cFileName = g_file_info_get_name (pFileInfo);
		//g_print (" - %s\n", cFileName);
		
		// il y'a 2 cas : un fichier dans la poubelle du home, et un fichier dans une poubelle d'un autre volume.
		if (cFileName && *cFileName == '\\')  // nom de la forme "\media\Fabounet2\.Trash-1000\files\t%C3%A8st%201" et URI "trash:///%5Cmedia%5CFabounet2%5C.Trash-1000%5Cfiles%5Ct%25C3%25A8st%25201", mais cette URI ne marche pas des qu'il y'a des caracteres non ASCII-7 dans le nom (bug dans gio/gvfs ?). Donc on feinte, en construisant le chemin du fichier (et de son double dans 'info').
		{
			g_string_printf (sFileUri, "file://%s", cFileName);
			g_strdelimit (sFileUri->str, "\\", '/');
			//g_print ("   - %s\n", sFileUri->str);
			
			GFileType iFileType = g_file_info_get_file_type (pFileInfo);
			if (iFileType == G_FILE_TYPE_DIRECTORY)  // can't delete a non-empty folder located on a different volume than home.
			{
				_cairo_dock_gio_vfs_empty_dir (sFileUri->str);
			}
			GFile *file = g_file_new_for_uri (sFileUri->str);
			g_file_delete (file, NULL, &erreur);
			g_object_unref (file);
			
			gchar *str = g_strrstr (sFileUri->str, "/files/");
			if (str)
			{
				*str = '\0';
				gchar *cInfo = g_strdup_printf ("%s/info/%s.trashinfo", sFileUri->str, str+7);
				//g_print ("   - %s\n", cInfo);
				file = g_file_new_for_uri (cInfo);
				g_free (cInfo);
				g_file_delete (file, NULL, NULL);
				g_object_unref (file);
			}
		}
		else  // poubelle principale : nom de la forme "tèst 1" et URI "trash:///t%C3%A8st%201"
		{
			if (strchr (cFileName, '%'))  // if there is a % inside the name, it disturb gio, so let's remove it.
			{
				gchar *cTmpPath = g_strdup_printf ("/%s", cFileName);
				gchar *cEscapedFileName = g_filename_to_uri (cTmpPath, NULL, NULL);
				g_free (cTmpPath);
				g_string_printf (sFileUri, "trash://%s", cEscapedFileName+7);  // replace file:// with trash://
				g_free (cEscapedFileName);
			}
			else  // else it can handle the URI as usual.
				g_string_printf (sFileUri, "trash:///%s", cFileName);
			GFile *file = g_file_new_for_uri (sFileUri->str);
			/*gchar *cValidURI = g_file_get_uri (file);
			//g_print ("   - %s\n", cValidURI);
			g_object_unref (file);
			
			file = g_file_new_for_uri (cValidURI);
			g_free (cValidURI);*/
			g_file_delete (file, NULL, &erreur);
			g_object_unref (file);
		}
		if (erreur != NULL)
		{
			cd_warning ("gnome_integration : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		
		g_object_unref (pFileInfo);
	} while (1);
	
	g_string_free (sFileUri, TRUE);
	g_object_unref (pFileEnum);
	g_object_unref (pFile);
}

static GList *cairo_dock_gio_vfs_list_apps_for_file (const gchar *cBaseURI)
{
	gchar *cValidUri;
	if (*cBaseURI == '/')
		cValidUri = g_filename_to_uri (cBaseURI, NULL, NULL);
	else
		cValidUri = g_strdup (cBaseURI);
	GFile *pFile = g_file_new_for_uri (cValidUri);
	
	GError *erreur = NULL;
	const gchar *cQuery = G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE;
	GFileInfo *pFileInfo = g_file_query_info (pFile,
		cQuery,
		G_FILE_QUERY_INFO_NONE,
		NULL,
		&erreur);
	
	if (erreur != NULL)  // peut arriver si l'emplacement n'est pas monte, mais on signale tout de meme la raison avec un warning.
	{
		cd_warning ("gnome_integration : %s", erreur->message);
		g_error_free (erreur);
		g_free (cValidUri);
		g_object_unref (pFile);
		return NULL;
	}
	
	const gchar *cMimeType = g_file_info_get_content_type (pFileInfo);
	
	GList *pAppsList = g_app_info_get_all_for_type (cMimeType);
	GList *a;
	GList *pList = NULL;
	gchar **pData;
	GAppInfo *pAppInfo;
	const char *cName, *cDisplayedName, *cExec;
	GIcon *pIcon;
	for (a = pAppsList; a != NULL; a = a->next)
	{
		pAppInfo = a->data;
		pIcon = g_app_info_get_icon (pAppInfo);
		
		pData = g_new0 (gchar*, 4);
		#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 24)
			pData[0] = g_strdup (g_app_info_get_display_name (pAppInfo));
		#else
			pData[0] = g_strdup (g_app_info_get_name (pAppInfo));
		#endif
		pData[1] = g_strdup (g_app_info_get_executable (pAppInfo));
		if (pIcon)
		#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 20)
			pData[2] = g_icon_to_string (pIcon);
		#else
			pData[2] = _cd_get_icon_path (pIcon, NULL);
		#endif
		pList = g_list_prepend (pList, pData);
	}
	
	g_free (cValidUri);
	g_object_unref (pFile);
	g_list_free (pAppsList);
	g_object_unref (pFileInfo);
	return pList;
}

gboolean cairo_dock_gio_vfs_fill_backend(CairoDockDesktopEnvBackend *pVFSBackend)
{
	if(pVFSBackend)
	{
		pVFSBackend->get_file_info = cairo_dock_gio_vfs_get_file_info;
		pVFSBackend->get_file_properties = cairo_dock_gio_vfs_get_file_properties;
		pVFSBackend->list_directory = cairo_dock_gio_vfs_list_directory;
		pVFSBackend->measure_directory = cairo_dock_gio_vfs_measure_directory;
		pVFSBackend->launch_uri = cairo_dock_gio_vfs_launch_uri;
		pVFSBackend->is_mounted = cairo_dock_gio_vfs_is_mounted;
		pVFSBackend->can_eject = cairo_dock_gio_vfs_can_eject;
		pVFSBackend->eject = cairo_dock_gio_vfs_eject_drive;
		pVFSBackend->mount = cairo_dock_gio_vfs_mount;
		pVFSBackend->unmount = cairo_dock_gio_vfs_unmount;
		pVFSBackend->add_monitor = cairo_dock_gio_vfs_add_monitor;
		pVFSBackend->remove_monitor = cairo_dock_gio_vfs_remove_monitor;
		pVFSBackend->delete_file = cairo_dock_gio_vfs_delete_file;
		pVFSBackend->rename = cairo_dock_gio_vfs_rename_file;
		pVFSBackend->move = cairo_dock_gio_vfs_move_file;
		pVFSBackend->create = cairo_dock_gio_vfs_create_file;
		pVFSBackend->get_trash_path = cairo_dock_gio_vfs_get_trash_path;
		pVFSBackend->empty_trash = cairo_dock_gio_vfs_empty_trash;
		pVFSBackend->get_desktop_path = cairo_dock_gio_vfs_get_desktop_path;
		pVFSBackend->list_apps_for_file = cairo_dock_gio_vfs_list_apps_for_file;
	}

	return TRUE;
}

#else

gboolean cairo_dock_gio_vfs_init (void)
{
	return FALSE;
}

gboolean cairo_dock_gio_vfs_fill_backend(CairoDockDesktopEnvBackend *pVFSBackend)
{
	return FALSE;
}

#endif

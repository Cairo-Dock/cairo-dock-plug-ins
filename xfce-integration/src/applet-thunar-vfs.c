/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to chris.chapuis@gmail.com)
Inspiration was taken from the "gnome-integration" plug-in...

******************************************************************************/
#include <string.h>

#include <thunar-vfs/thunar-vfs.h>

#include "applet-thunar-vfs.h"

static GHashTable *s_fm_MonitorHandleTable = NULL;


static void _vfs_backend_free_monitor_data (gpointer *data)
{
	if (data != NULL)
	{
    	ThunarVfsMonitor *pMonitor = thunar_vfs_monitor_get_default();
		ThunarVfsMonitorHandle *pHandle = data[2];
		thunar_vfs_monitor_remove(pMonitor, pHandle);  // le ThunarVFSMonitorHandle est-il libere lors du thunar_vfs_monitor_remove () ?
		g_free (data);
	}
}

gboolean init_vfs_backend (void)
{
	cd_message ("Initialisation du backend xfce-environnement\n");

	if (s_fm_MonitorHandleTable != NULL)
		g_hash_table_destroy (s_fm_MonitorHandleTable);

	s_fm_MonitorHandleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		(GDestroyNotify) _vfs_backend_free_monitor_data);

	thunar_vfs_init();

	return TRUE;
}

void stop_vfs_backend (void)
{
	cd_message ("Arret du backend xfce-environnement\n");

	if (s_fm_MonitorHandleTable != NULL)
	{
		g_hash_table_destroy (s_fm_MonitorHandleTable);
		s_fm_MonitorHandleTable = NULL;
	}

	thunar_vfs_shutdown();
}



static gboolean file_manager_get_file_info_from_desktop_link (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID)
{
	cd_message ("%s (%s)\n", __func__, cBaseURI);
	GError *erreur = NULL;

	gchar *cFileData = NULL;
	int iFileSize = 0;

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cBaseURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cBaseURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	gchar *cFilePath = thunar_vfs_path_dup_string(pThunarPath);
	thunar_vfs_path_unref(pThunarPath);
	if (cFilePath == NULL)
	{
		cd_message ("Attention : Couldn't retrieve path of %s\n", cBaseURI);
		return FALSE;
	}

	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile,
		cFilePath,
		G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
		&erreur);
	g_free (cFilePath);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
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
	GError *erreur = NULL;
	g_return_if_fail (cBaseURI != NULL);
	cd_message ("%s (%s)\n", __func__, cBaseURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cBaseURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cBaseURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
    ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarPath, &erreur);
	thunar_vfs_path_unref(pThunarPath);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}

	if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
		*fOrder = (double)pThunarVfsInfo->mtime;
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
		*fOrder = (double)pThunarVfsInfo->size;
	else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
		*fOrder = (double)pThunarVfsInfo->type;
	else
		*fOrder = 0;

	*cName = g_strdup (pThunarVfsInfo->display_name);
	*cURI = g_strdup (cBaseURI);
	*bIsDirectory = ((pThunarVfsInfo->type & THUNAR_VFS_FILE_TYPE_DIRECTORY) != 0);
	*cIconName = pThunarVfsInfo->custom_icon?g_strdup(pThunarVfsInfo->custom_icon):NULL;

    ThunarVfsMimeInfo*pThunarMimeInfo = pThunarVfsInfo->mime_info;
    if( pThunarMimeInfo )
    {
	    const gchar *cMimeType = thunar_vfs_mime_info_get_name (pThunarMimeInfo);
	    cd_message ("  cMimeType : %s\n", cMimeType);
	    if ( *cIconName == NULL && cMimeType && strcmp (cMimeType, "application/x-desktop") == 0)
	    {
        	thunar_vfs_info_unref(pThunarVfsInfo);
		    thunar_vfs_mime_info_unref( pThunarMimeInfo );
		    file_manager_get_file_info_from_desktop_link (cBaseURI, cName, cURI, cIconName, bIsDirectory, iVolumeID);
		    *fOrder = 0;
		    return ;
	    }

    	/*On verra un peu plus tard pour les vignettes des images, hein*/
	    if(*cIconName == NULL && (strncmp (cMimeType, "image", 5) == 0))
	    {
		    gchar *cHostname = NULL;
		    gchar *cFilePath = g_filename_from_uri (cBaseURI, &cHostname, &erreur);
		    if (erreur != NULL)
		    {
			    g_error_free (erreur);
		    }
		    else if (cHostname == NULL || strcmp (cHostname, "localhost") == 0)  // on ne recupere la vignette que sur les fichiers locaux.
		    {
			    *cIconName = thunar_vfs_path_dup_string(pThunarPath);
			    cairo_dock_remove_html_spaces (*cIconName);
		    }
		    g_free (cHostname);
	    }

	    if (*cIconName == NULL)
	    {
    	    *cIconName = g_strdup (thunar_vfs_mime_info_lookup_icon_name(pThunarMimeInfo, gtk_icon_theme_get_default()));
	    }
    }

	*iVolumeID = (int)pThunarVfsInfo->device;

	thunar_vfs_info_unref(pThunarVfsInfo);
}


struct ThunarFolder_t {
	GList *file_list;
	ThunarVfsJob *job;
	gboolean isJobFinished;
};

typedef struct ThunarFolder_t ThunarFolder;

static gboolean thunar_folder_infos_ready (ThunarVfsJob *job,
										   GList        *infos,
										   ThunarFolder *folder)
{
  /* merge the list with the existing list of new files */
  folder->file_list = g_list_concat (folder->file_list, infos);

  /* TRUE to indicate that we took over ownership of the infos list */
  return TRUE;
}

static void thunar_folder_finished (ThunarVfsJob *job, ThunarFolder *folder)
{
	/* we did it, the folder is loaded */
	g_signal_handlers_disconnect_matched (folder->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
	g_object_unref (G_OBJECT (folder->job));
	folder->job = NULL;
	folder->isJobFinished = TRUE;
}

/*
 * Attention: si l'URI demandee est CAIRO_DOCK_FM_VFS_ROOT, alors il faut retourner la liste des volumes !
 *    En effet dans ThunarVFS "root://" correspond simplement à "/"
 */
GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, int iNewIconsType, gchar **cFullURI)
{
	GError *erreur = NULL;
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	cd_message ("%s (%s)\n", __func__, cBaseURI);

	GList *pIconList = NULL;

	const gchar *cURI = NULL;
	if (strcmp (cBaseURI, CAIRO_DOCK_FM_VFS_ROOT) == 0)
	{
		cURI = "removable-media:///";

		if( cFullURI )
		    *cFullURI = g_strdup(cURI);

		/* listons joyeusement les volumes */
		ThunarVfsVolumeManager *pThunarVolumeManager = thunar_vfs_volume_manager_get_default();
		GList *pListVolumes = thunar_vfs_volume_manager_get_volumes(pThunarVolumeManager);
		int lVolumeFakeID = 1;

		for( ; pListVolumes != NULL; pListVolumes = pListVolumes->next, lVolumeFakeID++ )
		{
			ThunarVfsVolume *pThunarVfsVolume = (ThunarVfsVolume *)pListVolumes->data;

            /* Skip the volumes that are not there or that are not removable */
            if (!thunar_vfs_volume_is_present (pThunarVfsVolume) || !thunar_vfs_volume_is_removable (pThunarVfsVolume))
                continue;

      		ThunarVfsPath *pThunarVfsPath = thunar_vfs_volume_get_mount_point(pThunarVfsVolume);
			Icon *icon = g_new0 (Icon, 1);

			/* il nous faut: URI, type, nom, icone */

			icon->cBaseURI = thunar_vfs_path_dup_uri(pThunarVfsPath);

			icon->acCommand = thunar_vfs_path_dup_uri(pThunarVfsPath);
			icon->iVolumeID = lVolumeFakeID;

			cd_message (" -> icon->cBaseURI : %s\n", icon->cBaseURI);

			icon->iType = iNewIconsType;

			icon->acName = g_strdup(thunar_vfs_volume_get_name( pThunarVfsVolume ));
			cd_debug (" -> icon->acName : %s\n", icon->acName);

			icon->acFileName = thunar_vfs_volume_lookup_icon_name(pThunarVfsVolume, gtk_icon_theme_get_default());
			cd_debug (" -> icon->acFileName : %s\n", icon->acFileName);

			erreur = NULL;
		    ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarVfsPath, &erreur);
			if (erreur != NULL)
			{
				icon->fOrder = 0;
				cd_message ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
			}
			else
			{
				if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
					icon->fOrder = pThunarVfsInfo->size;
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
					icon->fOrder = pThunarVfsInfo->mtime;
				else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
					icon->fOrder = pThunarVfsInfo->type;

				thunar_vfs_info_unref(pThunarVfsInfo);
			}

			thunar_vfs_path_unref(pThunarVfsPath);

			pIconList = g_list_prepend (pIconList, icon);
		}

		return pIconList;
	}
	else if (strcmp (cBaseURI, CAIRO_DOCK_FM_NETWORK) == 0)
	{
    	cd_message (" -> Good try, but no, there's no network management in Thunar VFS !\n");

	    if( cFullURI )
        	*cFullURI = g_strdup(CAIRO_DOCK_FM_NETWORK);

		return pIconList;
	}
	else
		cURI = cBaseURI;

    if( cFullURI )
    	*cFullURI = g_strdup(cURI);
	g_return_val_if_fail (cURI != NULL, NULL);

	cd_message (" -> cURI : %s\n", cURI);

	ThunarFolder *folder = g_new( ThunarFolder, 1 );
	folder->isJobFinished = FALSE;
	folder->file_list = NULL;

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
	folder->job = thunar_vfs_listdir(pThunarPath, NULL);
	thunar_vfs_path_unref(pThunarPath);

  	g_signal_connect (folder->job, "error", G_CALLBACK (thunar_folder_finished), folder);
  	g_signal_connect (folder->job, "finished", G_CALLBACK (thunar_folder_finished), folder);
  	g_signal_connect (folder->job, "infos-ready", G_CALLBACK (thunar_folder_infos_ready), folder);

	while ( !folder->isJobFinished )
    {
      if (gtk_events_pending())
      {
          gtk_main_iteration(); // Handle unprocessed GTK events
      }
      else
      {
          g_thread_yield();     // Yield processing time
      }
    }

	/* Ô joie, on a une liste de ThunarVfsInfos ! */
	GList *lp = NULL;
	for (lp = folder->file_list; lp != NULL; lp = lp->next)
    {
		/* get the info... */
		ThunarVfsInfo *pThunarVfsInfo = (ThunarVfsInfo *)(lp->data);

		Icon *icon;

		if(pThunarVfsInfo != NULL &&
		   strcmp (thunar_vfs_path_get_name(pThunarVfsInfo->path), ".") != 0 &&
		   strcmp (thunar_vfs_path_get_name(pThunarVfsInfo->path), "..") != 0)
		{
			icon = g_new0 (Icon, 1);
			icon->cBaseURI = thunar_vfs_path_dup_uri(pThunarVfsInfo->path);
			cd_message (" item in directory : %s\n", icon->cBaseURI);
			icon->iType = iNewIconsType;
			if ( strcmp (thunar_vfs_mime_info_get_name(pThunarVfsInfo->mime_info), "application/x-desktop") == 0)
			{
				gboolean bIsDirectory = FALSE;
				file_manager_get_file_info_from_desktop_link (icon->cBaseURI, &icon->acName, &icon->acCommand, &icon->acFileName, &bIsDirectory, &icon->iVolumeID);
				cd_message ("  bIsDirectory : %d; iVolumeID : %d\n", bIsDirectory, icon->iVolumeID);
			}
			else
			{
				icon->acCommand = g_strdup(icon->cBaseURI);
				icon->acName = g_strdup (thunar_vfs_path_get_name(pThunarVfsInfo->path));
				icon->acFileName = NULL;
				if (strncmp (thunar_vfs_mime_info_get_name(pThunarVfsInfo->mime_info), "image", 5) == 0)  // && strncmp (cFileURI, "file://", 7) == 0
				{
					gchar *cHostname = NULL;
					GError *erreur = NULL;
					gchar *cFilePath = g_filename_from_uri (icon->cBaseURI, &cHostname, &erreur);
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
					icon->acFileName = g_strdup(thunar_vfs_mime_info_lookup_icon_name(pThunarVfsInfo->mime_info, gtk_icon_theme_get_default()));
				}
			}

			if (iSortType == CAIRO_DOCK_FM_SORT_BY_SIZE)
				icon->fOrder = pThunarVfsInfo->size;
			else if (iSortType == CAIRO_DOCK_FM_SORT_BY_DATE)
				icon->fOrder = pThunarVfsInfo->mtime;
			else if (iSortType == CAIRO_DOCK_FM_SORT_BY_TYPE)
				icon->fOrder = pThunarVfsInfo->type;

			pIconList = g_list_prepend (pIconList, icon);
		}

		/* ...release the info at the list position... */
		thunar_vfs_info_unref (lp->data);
	}

	if (iSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
		pIconList = cairo_dock_sort_icons_by_name (pIconList);
	else
		pIconList = cairo_dock_sort_icons_by_order (pIconList);

	return pIconList;
}

static ThunarVfsVolume *thunar_find_volume_from_path (ThunarVfsPath *pThunarPath)
{
	GError *erreur = NULL;

    ThunarVfsVolume *pThunarVolume = NULL;

    /* premiere methode: on scanne les volumes. c'est peut-etre un volume non monte... */
    ThunarVfsVolumeManager *pThunarVolumeManager = thunar_vfs_volume_manager_get_default();
    GList *pListVolumes = thunar_vfs_volume_manager_get_volumes(pThunarVolumeManager);
    for( ; pListVolumes != NULL; pListVolumes = pListVolumes->next )
    {
        pThunarVolume = (ThunarVfsVolume *)pListVolumes->data;

        /* Skip the volumes that are not there or that are not removable */
        if (!thunar_vfs_volume_is_present (pThunarVolume) || !thunar_vfs_volume_is_removable (pThunarVolume))
        {
            pThunarVolume = NULL;
            continue;
        }

        ThunarVfsPath *pMountPointVfsPath = thunar_vfs_volume_get_mount_point(pThunarVolume);
        if( thunar_vfs_path_is_ancestor( pThunarPath, pMountPointVfsPath ) )
        {
            g_object_ref(pThunarVolume);
            break;
        }
        pThunarVolume = NULL;
    }

    if( pThunarVolume == NULL )
    {
        /* deuxieme methode: avec le vfs_info. */
        ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarPath, &erreur);
        if (erreur != NULL)
        {
            cd_message ("Attention : %s\n", erreur->message);
            g_error_free (erreur);
        }
        else
        {
            ThunarVfsVolumeManager *pThunarVolumeManager = thunar_vfs_volume_manager_get_default();
            ThunarVfsVolume *pThunarVolume = thunar_vfs_volume_manager_get_volume_by_info(pThunarVolumeManager, pThunarVfsInfo);
            g_object_unref(pThunarVolumeManager);
            thunar_vfs_info_unref(pThunarVfsInfo);
        }
    }

    return pThunarVolume;
}

/* Fait */
void vfs_backend_launch_uri (const gchar *cURI)
{
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}

    ThunarVfsPath *pThunarRealPath = NULL;

    /* hop, trouvons le volume correspondant */
    ThunarVfsVolume *pThunarVolume = thunar_find_volume_from_path(pThunarPath);
	if (pThunarVolume != NULL)
	{
		thunar_vfs_path_unref(pThunarPath);
        pThunarPath = thunar_vfs_volume_get_mount_point(pThunarVolume);
        g_object_unref(pThunarVolume);
	}

    ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarPath, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		thunar_vfs_path_unref(pThunarPath);
		return;
	}

	/* if this is a directory, open Thunar in that directory */
	if( pThunarVfsInfo->flags & THUNAR_VFS_FILE_FLAGS_EXECUTABLE )
	{
		thunar_vfs_info_execute(pThunarVfsInfo,NULL,NULL, NULL,&erreur);
		if (erreur != NULL)
		{
			cd_message ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
		}
	}
	else
	{
		ThunarVfsMimeDatabase *pMimeDB = thunar_vfs_mime_database_get_default();
		if( pMimeDB )
		{
			ThunarVfsMimeApplication *pMimeApplication = thunar_vfs_mime_database_get_default_application(pMimeDB, pThunarVfsInfo->mime_info);
			if( pMimeApplication )
			{
				GList *path_list = g_list_prepend (NULL, pThunarPath);
				cd_message ("Launching %s ...\n", thunar_vfs_mime_handler_get_command(pMimeApplication) );
				thunar_vfs_mime_handler_exec(pMimeApplication,gdk_screen_get_default (),path_list,&erreur);
				g_list_free(path_list);
				g_object_unref( pMimeApplication );
				if (erreur != NULL)
				{
					cd_message ("Attention : %s\n", erreur->message);
					g_error_free (erreur);
				}
			}
			g_object_unref( pMimeDB );
		}
	}

	thunar_vfs_path_unref(pThunarPath);
	thunar_vfs_info_unref(pThunarVfsInfo);
}

/* Fait */
gchar *vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	GError *erreur = NULL;
	cd_message ("%s (%s)\n", __func__, cURI);

    ThunarVfsVolume *pThunarVolume = NULL;

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
        cd_message ("ERROR : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}

    /* hop, trouvons le volume correspondant */
    pThunarVolume = thunar_find_volume_from_path(pThunarPath);
    thunar_vfs_path_unref(pThunarPath);

	if (pThunarVolume == NULL)
	{
		cd_message ("Attention : no volume associated to %s, guessing that it is mounted\n", cURI);
		*bIsMounted = TRUE;
		return NULL;
	}

	*bIsMounted = thunar_vfs_volume_is_mounted(pThunarVolume);
	ThunarVfsPath *pMountPointVfsPath = thunar_vfs_volume_get_mount_point(pThunarVolume);
	gchar *cMountPointID = NULL;/*pMountPointVfsPath?thunar_vfs_path_dup_uri(pMountPointVfsPath):NULL;*/
    g_object_unref(pThunarVolume);

	cd_message ("  bIsMounted <- %d\n", *bIsMounted);

	return cMountPointID;
}


static void _vfs_backend_mount_callback(ThunarVfsVolume *volume, gpointer *data)
{
	cd_message ("%s (%x)\n", __func__, data);

	CairoDockFMMountCallback pCallback = data[0];

	pCallback (GPOINTER_TO_INT (data[1]), TRUE, data[2], data[3], data[4]);
}

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
    /* hop, trouvons le volume correspondant */
    ThunarVfsVolume *pThunarVolume = thunar_find_volume_from_path(pThunarPath);
    thunar_vfs_path_unref(pThunarPath);

	if (pThunarVolume == NULL)
	{
		cd_message ("Attention : no volume associated to %s\n", cURI);
		return;
	}

	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (TRUE);
	data2[2] = thunar_vfs_volume_get_name(pThunarVolume);
	data2[3] = icon;
	data2[4] = pDock;
	g_signal_connect(pThunarVolume, "mounted", G_CALLBACK (_vfs_backend_mount_callback), data2);

	if( !thunar_vfs_volume_mount(pThunarVolume, NULL, &erreur) )
	{
		cd_message ("Attention, %s couldn't be mounted : %s\n",cURI, erreur->message);
		g_error_free (erreur);
	}
	g_signal_handlers_disconnect_matched (pThunarVolume, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data2);
	g_free (data2);
    g_object_unref(pThunarVolume);
}

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : couldn't read %s\n", cURI);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
    /* hop, trouvons le volume correspondant */
    ThunarVfsVolume *pThunarVolume = thunar_find_volume_from_path(pThunarPath);
    thunar_vfs_path_unref(pThunarPath);

	if (pThunarVolume == NULL)
	{
		cd_message ("Attention : no volume associated to %s\n", cURI);
		return NULL;
	}

	gpointer *data2 = g_new (gpointer, 5);
	data2[0] = pCallback;
	data2[1] = GINT_TO_POINTER (FALSE);
	data2[2] = thunar_vfs_volume_get_name(pThunarVolume);
	data2[3] = icon;
	data2[4] = pDock;
	g_signal_connect(pThunarVolume, "unmounted", G_CALLBACK (_vfs_backend_mount_callback), data2);

	if( !thunar_vfs_volume_unmount(pThunarVolume, NULL, &erreur) )
	{
		cd_message ("Attention, %s couldn't be unmounted : %s\n",cURI, erreur->message);
		g_error_free (erreur);
	}
	g_signal_handlers_disconnect_matched (pThunarVolume, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data2);
	g_free (data2);
    g_object_unref(pThunarVolume);
}

static void _vfs_backend_thunar_monitor_callback(ThunarVfsMonitor *monitor,
                                                 ThunarVfsMonitorHandle *handle,
                                                 ThunarVfsMonitorEvent event,
                                                 ThunarVfsPath *handle_path,
                                                 ThunarVfsPath *event_path,
                                                 gpointer *data)
{
	CairoDockFMMonitorCallback pCallback = data[0];
	gpointer user_data = data[1];
	cd_message ("%s (%d , data : %x)\n", __func__, event, user_data);

	CairoDockFMEventType iEventType;
	switch (event)
	{
		case THUNAR_VFS_MONITOR_EVENT_CHANGED :
			iEventType = CAIRO_DOCK_FILE_MODIFIED;
		break;

		case THUNAR_VFS_MONITOR_EVENT_DELETED :
			iEventType = CAIRO_DOCK_FILE_DELETED;
		break;

		case THUNAR_VFS_MONITOR_EVENT_CREATED :
			iEventType = CAIRO_DOCK_FILE_CREATED;
		break;

		default :
		return ;
	}
	gchar *info_uri = thunar_vfs_path_dup_uri(event_path);
	pCallback (iEventType, info_uri, user_data);
	g_free(info_uri);
}

void vfs_backend_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer user_data)
{
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}

	ThunarVfsMonitor *pThunarMonitor = thunar_vfs_monitor_get_default();
	ThunarVfsMonitorHandle *pHandle = NULL;
	gpointer *data = g_new0 (gpointer, 3);
	data[0] = pCallback;
	data[1] = user_data;

	if( bDirectory )
	{
		pHandle = thunar_vfs_monitor_add_directory(pThunarMonitor, pThunarPath,
												   (ThunarVfsMonitorCallback) _vfs_backend_thunar_monitor_callback,
												   data);
	}
	else
	{
		pHandle = thunar_vfs_monitor_add_file(pThunarMonitor, pThunarPath,
											  (ThunarVfsMonitorCallback) _vfs_backend_thunar_monitor_callback,
											  data);
	}
	g_object_unref(pThunarMonitor);
	thunar_vfs_path_unref(pThunarPath);

	if (pHandle == NULL)
	{
		cd_message ("Attention : couldn't add monitor function to %s\n  I will not be able to receive events about this file\n", cURI);
		g_free (data);
	}
	else
	{
		cd_message (">>> moniteur ajoute sur %s (%x)\n", cURI, user_data);
		data[2] = pHandle;
		g_hash_table_insert (s_fm_MonitorHandleTable, g_strdup (cURI), data);
	}
}

void vfs_backend_remove_monitor (const gchar *cURI)
{
	cd_message ("%s (%s)\n", __func__, cURI);
	if (cURI != NULL)
	{
		gpointer *data = g_hash_table_lookup(s_fm_MonitorHandleTable,cURI);
		if( data )
		{
			ThunarVfsMonitorHandle *pHandle = data[2];
			if( pHandle )
			{
				ThunarVfsMonitor *pThunarMonitor = thunar_vfs_monitor_get_default();
				thunar_vfs_monitor_remove(pThunarMonitor, pHandle);
				g_object_unref(pThunarMonitor);
			}
		}

		cd_message (">>> moniteur supprime sur %s\n", cURI);
		g_hash_table_remove (s_fm_MonitorHandleTable, cURI);
	}
}


gboolean vfs_backend_delete_file (const gchar *cURI)
{
	GError *erreur = NULL;
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	ThunarVfsJob *pJob = thunar_vfs_unlink_file(pThunarPath, &erreur);
	g_object_unref(pJob);

	thunar_vfs_path_unref(pThunarPath);

	return TRUE;
}

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	GError *erreur = NULL;
	g_return_if_fail (cOldURI != NULL);
	cd_message ("%s (%s)\n", __func__, cOldURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cOldURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
    ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarPath, &erreur);
	thunar_vfs_path_unref(pThunarPath);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	thunar_vfs_info_rename(pThunarVfsInfo, cNewName, &erreur );
	thunar_vfs_info_unref(pThunarVfsInfo);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	return TRUE;
}

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	/*
	 @see thunar_vfs_move_file
	*/
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s, %s)\n", __func__, cURI, cDirectoryURI);

	ThunarVfsPath *pThunarPathFrom = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	gchar *cDestURI = g_strdup_printf("%s/%s", cDirectoryURI,  thunar_vfs_path_get_name(pThunarPathFrom));
	ThunarVfsPath *pThunarPathTo = thunar_vfs_path_new(cDestURI, &erreur);
	g_free(cDestURI);
	if (erreur != NULL)
	{
		thunar_vfs_path_unref(pThunarPathFrom);
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	ThunarVfsJob *pJob = thunar_vfs_move_file(pThunarPathFrom, pThunarPathTo, &erreur);

	thunar_vfs_path_unref(pThunarPathFrom);
	thunar_vfs_path_unref(pThunarPathTo);

	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}

	g_object_unref(pJob);
	return TRUE;
}

void vfs_backend_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	GError *erreur = NULL;
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)\n", __func__, cURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new(cURI, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}
    ThunarVfsInfo *pThunarVfsInfo = thunar_vfs_info_new_for_path(pThunarPath, &erreur);
	thunar_vfs_path_unref(pThunarPath);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return;
	}

	*iSize = pThunarVfsInfo->size;
	*iLastModificationTime = pThunarVfsInfo->mtime;
	*cMimeType = g_strdup (thunar_vfs_mime_info_get_name(pThunarVfsInfo->mime_info));
	*iUID = pThunarVfsInfo->uid;
	*iGID = pThunarVfsInfo->gid;
	*iPermissionsMask = pThunarVfsInfo->mode;

	thunar_vfs_info_unref(pThunarVfsInfo);
}


gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gboolean bCreateIfNecessary)
{
	GError *erreur = NULL;
	cd_message ("%s (%s)\n", __func__, cNearURI);

	ThunarVfsPath *pThunarPath = thunar_vfs_path_new("trash:/", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	thunar_vfs_path_unref(pThunarPath);

    /*
     * If we haven't returned NULL yet, it means that there exist a valid trash.
     * We know that Thunar follows the XDG recommendations.
     * So the trash is in ~/.local/share/Trash
     */

    gchar *trashPath = NULL;
    char *home = getenv("HOME");
    if( home )
    {
	    trashPath = g_strdup_printf("%s/%s", home,  ".local/share/Trash/files");
	}

	return trashPath;
}

gchar *vfs_backend_get_desktop_path (void)
{
	GError *erreur = NULL;
	cd_message ("%s\n", __func__);

	ThunarVfsPath *pThunarDesktopPath = thunar_vfs_path_new("desktop:/", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Error is : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	thunar_vfs_path_unref(pThunarDesktopPath);

    /*
     * If we haven't returned NULL yet, it means that there exist a valid desktop.
     * We know that Thunar follows the XDG recommendations.
     * So the trash is in ~/Desktop
     */

    gchar *desktopPath = NULL;
    char *home = getenv("HOME");
    if( home )
    {
	    desktopPath = g_strdup_printf("%s/%s", home,  "Desktop");
    }

	return desktopPath;
}



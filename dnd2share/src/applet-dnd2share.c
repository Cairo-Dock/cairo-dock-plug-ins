#include <stdlib.h>
#include <math.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"


void cd_dnd2share_free_uploaded_item (CDUploadedItem *pItem)
{
	if (pItem == NULL)
		return ;
	g_strfreev (pItem->cDistantUrls);
	g_free (pItem->cItemName);
	g_free (pItem->cLocalPath);
	g_free (pItem);
}


void cd_dnd2share_build_history (void)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile == NULL)  // pas encore d'historique.
	{
		g_free (cConfFilePath);
		return ;
	}
	
	CDUploadedItem *pItem;
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	int iSiteID;
	gchar *cItemName;
	GString *sUrlKey = g_string_new ("");
	GError *erreur = NULL;
	int i,j;
	for (i = 0; pGroupList[i] != NULL; i ++)
	{
		cItemName = pGroupList[i];
		iSiteID = g_key_file_get_integer (pKeyFile, cItemName, "site", &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
		if (iSiteID >= CD_NB_SITES)
		{
			cd_warning ("dnd2share : this backend doesn't exist !");
			continue;
		}
		
		pItem = g_new0 (CDUploadedItem, 1);
		pItem->cItemName = cItemName;
		pItem->iSiteID = iSiteID;
		pItem->cDistantUrls = g_new0 (gchar*, myData.backends[pItem->iSiteID].iNbUrls+1);
		for (j = 0; j < myData.backends[pItem->iSiteID].iNbUrls; j ++)
		{
			g_string_printf (sUrlKey, "url%d", j);
			pItem->cDistantUrls[j] = g_key_file_get_string (pKeyFile, cItemName, sUrlKey->str, NULL);
		}
		pItem->iDate = g_key_file_get_integer (pKeyFile, cItemName, "date", NULL);  /// un 'int' est-ce que ca suffit ?...
		pItem->cLocalPath = g_key_file_get_string (pKeyFile, cItemName, "local path", NULL);
		pItem->cFileName = g_path_get_basename (pItem->cLocalPath);
		
		myData.pUpoadedItems = g_list_prepend (myData.pUpoadedItems, pItem);
	}
	g_string_free (sUrlKey, TRUE);
	g_free (pGroupList);  // le contenu a ete pris par la liste.
	g_key_file_free (pKeyFile);
}

void cd_dnd2share_clear_history (void)
{
	g_list_foreach (myData.pUpoadedItems, (GFunc) cd_dnd2share_free_uploaded_item, NULL);
	g_list_free (myData.pUpoadedItems);
	myData.pUpoadedItems = NULL;
}



static void _cd_dnd2share_threaded_upload (gchar *cFilePath)
{
	gboolean bResultOK = myData.pCurrentBackend->upload (cFilePath, myData.iCurrentFileType);
}
static gboolean _cd_dnd2share_update_from_result (gchar *cFilePath)
{
	if (myData.cResultUrls == NULL)  // une erreur s'est produite.
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("Couldn't upload the file, check that your internet connexion is active."),
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	else
	{
		// On rajoute l'item a l'historique.
		if (myConfig.iNbItems != 0)
		{
			// On ouvre le fichier de l'historique.
			gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
			GKeyFile *pKeyFile;
			if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // pas encore d'historique.
				pKeyFile = g_key_file_new ();
			else
				pKeyFile = cairo_dock_open_key_file (cConfFilePath);
			if (pKeyFile == NULL)  // probleme de droit ?
			{
				cd_warning ("Couldn't add this item to history.");
			}
			else
			{
				// On regarde si on n'a pas atteint la limite de taille de l'historique.
				gsize length = 0;
				gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
				if (length == myConfig.iNbItems)  // il faut supprimer le 1er item.
				{
					g_key_file_remove_group (pKeyFile, pGroupList[0], NULL);
					if (myData.pUpoadedItems != NULL)  // il est en dernier dans la liste.
					{
						GList *it = g_list_last (myData.pUpoadedItems);
						if (it->prev != NULL)
							it->prev->next = NULL;
						it->prev = NULL;
						cd_dnd2share_free_uploaded_item (it->data);
						g_list_free1 (it);
					}
				}
				g_strfreev (pGroupList);
				
				// on rajoute le nouvel item en fin de fichier.
				time_t iDate = time (NULL);
				gchar *cItemName = g_strdup_printf ("item_%ld", iDate);
				
				g_key_file_set_integer (pKeyFile, cItemName, "site", myConfig.iPreferedSite);
				g_key_file_set_integer (pKeyFile, cItemName, "date", iDate);  // idem que precedemment sur l'integer.
				GString *sUrlKey = g_string_new ("");
				int j;
				for (j = 0; j < myData.pCurrentBackend->iNbUrls; j ++)
				{
					g_string_printf (sUrlKey, "url%d", j);
					g_key_file_set_string (pKeyFile, cItemName, sUrlKey->str, myData.cResultUrls[j]);
				}
				g_key_file_set_string (pKeyFile, cItemName, "local path", cFilePath);
				
				// et en debut de liste aussi.
				CDUploadedItem *pItem = g_new0 (CDUploadedItem, 1);
				pItem->cItemName = cItemName;
				pItem->iSiteID = myConfig.iPreferedSite;
				pItem->cDistantUrls = g_new0 (gchar*, myData.backends[pItem->iSiteID].iNbUrls + 1);
				for (j = 0; j < myData.pCurrentBackend->iNbUrls; j ++)
				{
					pItem->cDistantUrls[j] = g_strdup (myData.cResultUrls[j]);
				}
				pItem->iDate = iDate;
				pItem->cLocalPath = g_strdup (cFilePath);
				pItem->cFileName = g_path_get_basename (cFilePath);
				myData.pUpoadedItems = g_list_prepend (myData.pUpoadedItems, pItem);
				
				// On ecrit tout.
				cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
				g_key_file_free (pKeyFile);
				g_string_free (sUrlKey, TRUE);
				
				// On garde une copie du fichier.
				if (myConfig.bkeepCopy)
				{
					gchar *cCommand = g_strdup_printf ("cp '%s' '%s/%s'", cFilePath, myData.cWorkingDirPath, cItemName);
					int r = system (cCommand);
					g_free (cCommand);
				}
			}
			g_free (cConfFilePath);
		}
		
		// On copie l'URL dans le clipboard.
		gchar *cURL = myData.cResultUrls[myData.pCurrentBackend->iPreferedUrlType];
		if (cURL == NULL)
		{
			int i;
			for (i = 0; i < myData.pCurrentBackend->iNbUrls && cURL == NULL; i ++)
			{
				cURL = myData.cResultUrls[i];
			}
		}
		cd_dnd2share_copy_url_to_clipboard (cURL);
		
		// On garde en memoire la derniere URL au cas ou on n'aurait pas/plus d'historique.
		g_free (myData.cLastURL);
		myData.cLastURL = g_strdup (cURL);
		myData.iCurrentItemNum = 0;
		
		// On signale par un dialogue la fin de l'upload.
		if (myConfig.bEnableDialogs || myDesklet)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("File has been uploaded.\nJust press CTRL+v to paste its URL anywhere."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}
	
	// On arrete son animation.
	cairo_dock_stop_icon_animation (myIcon);
	
	if (myConfig.bDisplayLastImage)
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cFilePath);
		CD_APPLET_REDRAW_MY_ICON;
	}
	
	// On nettoie la memoire partagee.
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	myData.pMeasureTimer = NULL;
	g_free (myData.cCurrentFilePath);
	myData.cCurrentFilePath = NULL;
	g_strfreev (myData.cResultUrls);
	myData.cResultUrls = NULL;
	return FALSE;
}
void cd_dnd2share_launch_upload (const gchar *cFilePath, CDFileType iFileType)
{
	if (strncmp (cFilePath, "file://", 7) == 0)
		cFilePath += 7;
	if (myData.pMeasureTimer != NULL)
	{
		cd_warning ("Please wait the current upload is finished.");
		return ;
	}
	
	// on lance la mesure.
	myData.cCurrentFilePath = g_strdup (cFilePath);  // sera efface a la fin de l'upload.
	myData.iCurrentFileType = myData.iCurrentFileType;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (0,  // 1 shot measure.
		NULL,
		(CairoDockReadTimerFunc) _cd_dnd2share_threaded_upload,
		(CairoDockUpdateTimerFunc) _cd_dnd2share_update_from_result,
		myData.cCurrentFilePath);
	
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	// On lance une animation.
	cairo_dock_request_icon_animation (myIcon, myContainer, myConfig.cIconAnimation, 1e6);  // on l'interrompra nous-memes a la fin de l'upload.
	cairo_dock_mark_icon_as_clicked (myIcon);  // pour ne pas se faire interrompre par un survol.
	cairo_dock_launch_animation (myContainer);
}



void cd_dnd2share_clear_working_directory (void)
{
	g_return_if_fail (myData.cWorkingDirPath != NULL && *myData.cWorkingDirPath == '/');
	gchar *cCommand = g_strdup_printf ("rm -rf '%s'/*", myData.cWorkingDirPath);
	int r = system (cCommand);
	g_free (cCommand);
	
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	g_file_set_contents (cConfFilePath, "#dnd2share's history\n\n", -1, NULL);
	g_free (cConfFilePath);
	
	if (myConfig.bDisplayLastImage)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	}
}


void cd_dnd2share_clear_copies_in_working_directory (void)
{
	g_return_if_fail (myData.cWorkingDirPath != NULL && *myData.cWorkingDirPath == '/');
	gchar *cCommand = g_strdup_printf ("find '%s' ! -name *.conf -exec rm -f {} \\;", myData.cWorkingDirPath);
	int r = system (cCommand);
	g_free (cCommand);
}

void cd_dnd2share_set_working_directory_size (int iNbItems)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile == NULL)  // pas encore d'historique.
	{
		g_free (cConfFilePath);
		return ;
	}
	
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	if (length > iNbItems)
	{
		gchar *cItemName;
		GString *sPreviewPath = g_string_new ("");
		int i;
		for (i = 0; pGroupList[i] != NULL && i < length - iNbItems; i ++)  // on supprime les n premiers groupes en trop, ainsi que leurs eventuelles prevues.
		{
			cItemName = pGroupList[i];
			g_string_printf (sPreviewPath, "%s/%s", myData.cWorkingDirPath, cItemName);
			g_remove (sPreviewPath->str);
			g_key_file_remove_group (pKeyFile, cItemName, NULL);
		}
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
		g_string_free (sPreviewPath, TRUE);
	}
	
	g_strfreev (pGroupList);
	g_key_file_free (pKeyFile);
	g_free (cConfFilePath);
}

void cd_dnd2share_clean_working_directory (void)
{
	if (myConfig.iNbItems == 0)  // on ne veut plus d'historique => vidons le repertoire.
	{
		cd_debug ("DND2SHARE : Pas d'historique -> On efface le contenu de '%s'", myData.cWorkingDirPath);
		cd_dnd2share_clear_working_directory ();
	}
	else
	{
		cd_dnd2share_set_working_directory_size (myConfig.iNbItems);  // on efface les items en trop.
		if (! myConfig.bkeepCopy)  // on veut bien un historique mais sans les sauvegardes locales des images => nettoyons le repertoire.
		{
			cd_debug ("DND2SHARE : Pas de copies locales -> On efface les images de '%s'", myData.cWorkingDirPath);
			cd_dnd2share_clear_copies_in_working_directory ();
		}
	}
}


void cd_dnd2share_copy_url_to_clipboard (const gchar *cURL)
{
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text (pClipBoard, cURL, -1);
}


gchar *cd_dnd2share_get_prefered_url_from_item (CDUploadedItem *pItem)
{
	CDSiteBackend *pBackend = &myData.backends[pItem->iSiteID];
	gchar *cURL = pItem->cDistantUrls[pBackend->iPreferedUrlType];
	if (cURL == NULL)
	{
		int i;
		for (i = 0; i < pBackend->iNbUrls && cURL == NULL; i ++)
		{
			cURL = pItem->cDistantUrls[i];
		}
	}
	return cURL;
}

void cd_dnd2share_set_current_url_from_item (CDUploadedItem *pItem)
{
	gchar *cURL = cd_dnd2share_get_prefered_url_from_item (pItem);
	g_free (myData.cLastURL);
	myData.cLastURL = g_strdup (cURL);
	
	int i = 0;
	GList *it;
	for (it = myData.pUpoadedItems; it != NULL; it = it->next)
	{
		if (it->data == pItem)
			break ;
		i ++;
	}
	myData.iCurrentItemNum = i;
}

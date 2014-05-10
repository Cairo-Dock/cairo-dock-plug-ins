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

#include <string.h>
#include <stdlib.h>
#define __USE_POSIX
#include <time.h>
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


static gchar * _get_short_text_for_menu (const gchar *cInitText)
{
	if (cInitText == NULL) // the backend should not allow that!
		return g_strdup (D_("No text"));

	// remove extras withespaces first
	gchar *cLongText = g_strstrip (g_strdup (cInitText));
	// then print only the first line => no, like Clipper
	/*gchar *str = strchr (cLongText, '\n');
	if (str)
		*str = '\0';*/
	gchar *cShortText = cairo_dock_cut_string (cLongText, 40);

	// With 'Text' label, it's different than filename
	gchar *cResult = g_strdup_printf ("%s %s", D_("Text:"), cShortText);

	g_free (cLongText);
	g_free (cShortText);

	return cResult;
}

void cd_dnd2share_build_history (void)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_free (cConfFilePath);
	if (pKeyFile == NULL)  // no history yet.
		return ;

	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	if (pGroupList == NULL)
		return ;

	CDUploadedItem *pItem;
	int iSiteID, iFileType;
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
			g_free (cItemName);
			continue;
		}
		if (iSiteID >= CD_NB_SITES_MAX)
		{
			cd_warning ("dnd2share : this backend doesn't exist !");
			g_free (cItemName);
			continue;
		}
		iFileType = g_key_file_get_integer (pKeyFile, cItemName, "type", &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			g_free (cItemName);
			continue;
		}
		if (iFileType >= CD_NB_FILE_TYPES)
		{
			cd_warning ("dnd2share : this type of file doesn't exist !");
			g_free (cItemName);
			continue;
		}

		pItem = g_new0 (CDUploadedItem, 1);
		pItem->cItemName = cItemName;
		pItem->iSiteID = iSiteID;
		pItem->iFileType = iFileType;
		pItem->cDistantUrls = g_new0 (gchar*, myData.backends[iFileType][iSiteID].iNbUrls+1);
		for (j = 0; j < myData.backends[iFileType][iSiteID].iNbUrls; j ++)
		{
			g_string_printf (sUrlKey, "url%d", j);
			pItem->cDistantUrls[j] = g_key_file_get_string (pKeyFile, cItemName, sUrlKey->str, NULL);  // NULL if this URL has not been saved before.
		}
		pItem->iDate = g_key_file_get_integer (pKeyFile, cItemName, "date", NULL);

		pItem->cLocalPath = g_key_file_get_string (pKeyFile, cItemName, "local path", NULL);
		if (pItem->iFileType == CD_TYPE_TEXT)
			pItem->cFileName = _get_short_text_for_menu (pItem->cLocalPath);
		else
			pItem->cFileName = g_path_get_basename (pItem->cLocalPath);

		myData.pUpoadedItems = g_list_prepend (myData.pUpoadedItems, pItem);
	}
	g_string_free (sUrlKey, TRUE);
	g_free (pGroupList);  // the content has been added in the list.
	g_key_file_free (pKeyFile);
}

void cd_dnd2share_clear_history (void)
{
	g_list_foreach (myData.pUpoadedItems, (GFunc) cd_dnd2share_free_uploaded_item, NULL);
	g_list_free (myData.pUpoadedItems);
	myData.pUpoadedItems = NULL;
}



static void _cd_dnd2share_threaded_upload (CDSharedMemory *pSharedMemory)
{
	gchar *cFilePath = pSharedMemory->cCurrentFilePath;

	pSharedMemory->cResultUrls = g_new0 (gchar *, pSharedMemory->iNbUrls+1);  // NULL-terminated
	pSharedMemory->upload (cFilePath, pSharedMemory->cLocalDir, pSharedMemory->bAnonymous, pSharedMemory->iLimitRate, pSharedMemory->cResultUrls, &pSharedMemory->pError);

	if (pSharedMemory->cResultUrls[0] && pSharedMemory->iTinyURLService != 0)  // tiny-url.
	{
		gchar *Command = NULL;
		switch (pSharedMemory->iTinyURLService)
		{
			case 1:
			default:
				Command = g_strdup_printf ("http://tinyurl.com/api-create.php?url=%s", pSharedMemory->cResultUrls[0]);
			break;
			case 2:
				Command = g_strdup_printf ("http://shorterlink.org/createlink.php?url=%s", pSharedMemory->cResultUrls[0]);
			break;
			/*http://soso.bz/
			http://notlong.com/links/
			http://www.minu.me/
			http://cuturl.biz/
			http://tiny.cc/
			http://o-x.fr/create.php
			http://petitlien.fr/create.php
			http://bit.ly
			http://is.gd/create.php*/
		}
		pSharedMemory->cResultUrls[pSharedMemory->iNbUrls-1] = cairo_dock_get_url_data (Command, NULL);
		g_free (Command);
	}
}

static void _cd_dnd2share_show_error_dialog (const gchar *cError)
{
	gldi_dialogs_remove_on_icon (myIcon);

	gldi_dialog_show_temporary_with_icon (cError,
		myIcon,
		myContainer,
		myConfig.dTimeDialogs * 2,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
}

static gboolean _cd_dnd2share_update_from_result (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	gchar *cFilePath = pSharedMemory->cCurrentFilePath;
	if (pSharedMemory->pError != NULL)
		_cd_dnd2share_show_error_dialog (pSharedMemory->pError->message);
	else if (pSharedMemory->cResultUrls == NULL || pSharedMemory->cResultUrls[0] == NULL) // just to be sure
		_cd_dnd2share_show_error_dialog (DND2SHARE_GENERIC_ERROR_MSG);
	else
	{
		CDSiteBackend *pCurrentBackend = myData.pCurrentBackend[pSharedMemory->iCurrentFileType];
		// we add it in the history.
		if (myConfig.iNbItems != 0)
		{
			// open the file which contains the history
			gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
			GKeyFile *pKeyFile;
			if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // no history yet.
				pKeyFile = g_key_file_new ();
			else
				pKeyFile = cairo_dock_open_key_file (cConfFilePath);
			if (pKeyFile == NULL)  // probleme de droit ?
			{
				cd_warning ("Couldn't add this item to history.");
			}
			else
			{
				// we check the size limit
				gsize length = 0;
				gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
				if (length == myConfig.iNbItems)  // if yes, we remove the first entry
				{
					g_key_file_remove_group (pKeyFile, pGroupList[0], NULL);
					if (myData.pUpoadedItems != NULL)  // which is the last one in the list
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

				// we add the new item at the end of the file
				time_t iDate = time (NULL);
				gchar *cItemName = g_strdup_printf ("item_%ld", iDate);

				g_key_file_set_integer (pKeyFile, cItemName, "site", myConfig.iPreferedSite[pSharedMemory->iCurrentFileType]);
				g_key_file_set_integer (pKeyFile, cItemName, "date", iDate);
				g_key_file_set_integer (pKeyFile, cItemName, "type", pSharedMemory->iCurrentFileType);
				GString *sUrlKey = g_string_new ("");
				int j;
				for (j = 0; j < pCurrentBackend->iNbUrls; j ++)
				{
					g_string_printf (sUrlKey, "url%d", j);
					g_key_file_set_string (pKeyFile, cItemName, sUrlKey->str, pSharedMemory->cResultUrls[j]);
				}
				g_key_file_set_string (pKeyFile, cItemName, "local path", cFilePath);

				// and at the beginning of the list
				CDUploadedItem *pItem = g_new0 (CDUploadedItem, 1);
				pItem->cItemName = cItemName;
				pItem->iSiteID = myConfig.iPreferedSite[pSharedMemory->iCurrentFileType];
				pItem->iFileType = pSharedMemory->iCurrentFileType;
				pItem->cDistantUrls = g_new0 (gchar*, pCurrentBackend->iNbUrls + 1);
				for (j = 0; j < pCurrentBackend->iNbUrls; j ++)
				{
					pItem->cDistantUrls[j] = g_strdup (pSharedMemory->cResultUrls[j]);
				}
				pItem->iDate = iDate;
				pItem->cLocalPath = g_strdup (cFilePath);
				if (pItem->iFileType == CD_TYPE_TEXT)
					pItem->cFileName = _get_short_text_for_menu (cFilePath);
				else
					pItem->cFileName = g_path_get_basename (cFilePath);
				myData.pUpoadedItems = g_list_prepend (myData.pUpoadedItems, pItem);

				// We flush the file.
				cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
				g_key_file_free (pKeyFile);
				g_string_free (sUrlKey, TRUE);

				// we keep a copy if it's an image
				if (myConfig.bkeepCopy && pSharedMemory->iCurrentFileType == CD_TYPE_IMAGE)
				{
					gchar *cCommand = g_strdup_printf ("cp '%s' '%s/%s'", cFilePath, myData.cWorkingDirPath, cItemName);
					int r = system (cCommand);
					if (r < 0)
						cd_warning ("Not able to launch this command: %s", cCommand);
					g_free (cCommand);
				}
			}
			g_free (cConfFilePath);
		}

		// We copy the url to the clipboard
		gchar *cURL = NULL;
		if (myConfig.bUseTinyAsDefault)
			cURL = pSharedMemory->cResultUrls[pCurrentBackend->iNbUrls-1];
		if (cURL == NULL)
			cURL = pSharedMemory->cResultUrls[pCurrentBackend->iPreferedUrlType];
		if (cURL == NULL)
		{
			int i;
			for (i = 0; i < pCurrentBackend->iNbUrls && cURL == NULL; i ++)
			{
				cURL = pSharedMemory->cResultUrls[i];
			}
		}
		cd_dnd2share_copy_url_to_clipboard (cURL);

		// we keep the last URL if we don't want the history.
		g_free (myData.cLastURL);
		myData.cLastURL = g_strdup (cURL);
		myData.iCurrentItemNum = 0;

		// we can now display a dialogue.
		if (myConfig.bEnableDialogs || myDesklet)
		{
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("File has been uploaded.\nJust press CTRL+v to paste its URL anywhere."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}

		// and set the image on the icon.
		if (myConfig.bDisplayLastImage)
		{
			if (pSharedMemory->iCurrentFileType == CD_TYPE_IMAGE)
				CD_APPLET_SET_IMAGE_ON_MY_ICON (cFilePath);
			else
				CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}

	// stop the animation.
	CD_APPLET_STOP_DEMANDING_ATTENTION;

	// delete the file if it was a temporary one.
	if (pSharedMemory->bTempFile)
	{
		g_remove (pSharedMemory->cCurrentFilePath);
	}

	if (myData.cTmpFilePath != NULL)
	{
		g_remove (myData.cTmpFilePath);
		g_free (myData.cTmpFilePath);
		myData.cTmpFilePath = NULL;
	}

	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	CD_APPLET_LEAVE (FALSE);
}
static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cLocalDir);
	g_free (pSharedMemory->cCurrentFilePath);
	g_strfreev (pSharedMemory->cResultUrls);
	if (pSharedMemory->pError != NULL)
		g_error_free (pSharedMemory->pError);
	g_free (pSharedMemory);
}
#define CD_BUFFER_LENGTH 50
void cd_dnd2share_launch_upload (const gchar *cFilePath, CDFileType iFileType)
{
	if (myData.pTask != NULL)
	{
		cd_warning ("Please wait the current upload is finished before starting a new one.");
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("Please wait for the current upload to finish before starting a new one."),
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		return ;
	}

	if (myData.pCurrentBackend[iFileType]->upload == NULL)
	{
		cd_warning ("sorry, it's still not possible to upload this type of file");
		return ;
	}

	// launch the task.
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	gboolean bIsPath = FALSE; // we can receive text or a text file
	if (strncmp (cFilePath, "file://", 7) == 0)
	{
		cd_debug ("FilePath: %s", cFilePath);
		cFilePath += 7;
		bIsPath = TRUE;
	}
	else if (iFileType == CD_TYPE_TEXT && *cFilePath == '/' && g_file_test (cFilePath, G_FILE_TEST_EXISTS))
		bIsPath = TRUE;

	gchar *cTmpFile = NULL;
	if (myConfig.bUseOnlyFileType)
	{
		// for a piece of text, write it in a temporary file and upload this one.
		if (iFileType == CD_TYPE_TEXT && ! bIsPath)
		{
			// make a filename based on the upload date.
			cTmpFile = g_new0 (gchar, CD_BUFFER_LENGTH+1);
			time_t epoch = time (NULL);
			struct tm currentTime;
			localtime_r (&epoch, &currentTime);
			strftime (cTmpFile, CD_BUFFER_LENGTH, "/tmp/cd-%F__%H-%M-%S.txt", &currentTime);

			// write the text inside.
			g_file_set_contents (cTmpFile,
				cFilePath,
				-1,
				NULL);

			// upload this file
			cFilePath = cTmpFile;
			pSharedMemory->bTempFile = TRUE;
		}
		// force the 'file' type to be used.
		pSharedMemory->iCurrentFileType = CD_TYPE_FILE;
	}
	else
	{
		pSharedMemory->iCurrentFileType = iFileType;
	}

	// If we drop a text file, we have an URI but we want to post the content to a website
	if (pSharedMemory->iCurrentFileType == CD_TYPE_TEXT && bIsPath)
	{
		cd_debug ("Type is text and it's a file: %s", cFilePath);
		gchar *cContents = NULL;
		gsize iLength;
		g_file_get_contents (cFilePath, &cContents, &iLength, NULL);
		if (cContents == NULL)  // file was not readable, abort.
		{
			cd_warning ("file not readable !");
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("This file is not readable."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			return ;
		}
		pSharedMemory->cCurrentFilePath = cContents;
	}
	else
		pSharedMemory->cCurrentFilePath = g_strdup (cFilePath);

	g_free (cTmpFile);

	pSharedMemory->iTinyURLService = myConfig.iTinyURLService;
	pSharedMemory->cLocalDir = g_strdup (myConfig.cLocalDir);
	pSharedMemory->bAnonymous = myConfig.bAnonymous;
	pSharedMemory->iLimitRate = myConfig.iLimitRate;

	CDSiteBackend *pCurrentBackend = myData.pCurrentBackend[pSharedMemory->iCurrentFileType];
	g_return_if_fail (pCurrentBackend != NULL);
	pSharedMemory->upload = pCurrentBackend->upload;
	pSharedMemory->iNbUrls = pCurrentBackend->iNbUrls;

	myData.pTask = cairo_dock_new_task_full (0,  // 1 shot task.
		(CairoDockGetDataAsyncFunc) _cd_dnd2share_threaded_upload,
		(CairoDockUpdateSyncFunc) _cd_dnd2share_update_from_result,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);

	cairo_dock_launch_task (myData.pTask);

	CD_APPLET_DEMANDS_ATTENTION (myConfig.cIconAnimation, 1e6);  // we'll stop it later, at the end of the upload.
}



void cd_dnd2share_clear_working_directory (void)
{
	g_return_if_fail (myData.cWorkingDirPath != NULL && *myData.cWorkingDirPath == '/');
	gchar *cCommand = g_strdup_printf ("rm -rf '%s'/*", myData.cWorkingDirPath);
	int r = system (cCommand);
	if (r < 0)
		cd_warning ("Not able to launch this command: %s", cCommand);
	g_free (cCommand);

	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	g_file_set_contents (cConfFilePath, "#dnd2share's history\n\n", -1, NULL);
	g_free (cConfFilePath);

	if (myConfig.bDisplayLastImage)
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
}


void cd_dnd2share_clear_copies_in_working_directory (void)
{
	g_return_if_fail (myData.cWorkingDirPath != NULL && *myData.cWorkingDirPath == '/');
	gchar *cCommand = g_strdup_printf ("find '%s' -mindepth 1 ! -name *.conf -exec rm -f '{}' \\;", myData.cWorkingDirPath);
	int r = system (cCommand);
	if (r < 0)
		cd_warning ("Not able to launch this command: %s", cCommand);
	g_free (cCommand);
}

void cd_dnd2share_set_working_directory_size (guint iNbItems)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile == NULL)  // no history yet
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
		guint i;
		for (i = 0; pGroupList[i] != NULL && i < length - iNbItems; i ++)  // we remove all extras groups and the preview
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
	if (myConfig.iNbItems == 0)  // no more history => clean the working directory.
	{
		cd_debug ("DND2SHARE : Pas d'historique -> On efface le contenu de '%s'", myData.cWorkingDirPath);
		cd_dnd2share_clear_working_directory ();
	}
	else
	{
		cd_dnd2share_set_working_directory_size (myConfig.iNbItems);  // we remove extras items.
		if (! myConfig.bkeepCopy)  // we don't want a copy of the images
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
void cd_dnd2share_copy_url_to_primary (const gchar *cURL)
{
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
	gtk_clipboard_set_text (pClipBoard, cURL, -1);
}

gchar *cd_dnd2share_get_prefered_url_from_item (CDUploadedItem *pItem)
{
	CDSiteBackend *pBackend = &myData.backends[pItem->iFileType][pItem->iSiteID];
	//g_print ("%s (type:%d; site:%d)\n", __func__, pItem->iFileType, pItem->iSiteID);
	gchar *cURL = NULL;
	if (myConfig.bUseTinyAsDefault)
		cURL = pItem->cDistantUrls[pBackend->iNbUrls-1];
	if (cURL == NULL)
		cURL = pItem->cDistantUrls[pBackend->iPreferedUrlType];
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


void cd_dnd2share_remove_one_item (CDUploadedItem *pItem)
{
	g_return_if_fail (pItem != NULL);

	// we remove the corresponding group in the history file
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, "history.conf");
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // no history yet.
		return;

	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile == NULL)  // right problem?
	{
		cd_warning ("Couldn't remove this item from history.");
		return ;
	}

	g_key_file_remove_group (pKeyFile, pItem->cItemName, NULL);
	cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	g_key_file_free (pKeyFile);
	g_free (cConfFilePath);

	// we remove the local copy.
	gchar *cPreviewPath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
	g_remove (cPreviewPath);
	g_free (cPreviewPath);

	// If it's the current item, switch to the next one.
	if (myData.pUpoadedItems && myData.pUpoadedItems->data == pItem)
	{
		g_free (myData.cLastURL);
		myData.cLastURL = NULL;
		myData.iCurrentItemNum = 0;
		if (myData.pUpoadedItems->next != NULL)
		{
			CDUploadedItem *pNextItem = myData.pUpoadedItems->next->data;
			gchar *cURL = cd_dnd2share_get_prefered_url_from_item (pNextItem);
			myData.cLastURL = g_strdup (cURL);
		}
	}

	// We remove the item from the list
	myData.pUpoadedItems = g_list_remove (myData.pUpoadedItems, pItem);
	cd_dnd2share_free_uploaded_item (pItem);
}


void cd_dnd2share_register_new_backend (CDFileType iFileType, const gchar *cSiteName, int iNbUrls, const gchar **cUrlLabels, int iPreferedUrlType, CDUploadFunc pUploadFunc)
{
	int iNumSite = myData.iNbSitesForType[iFileType];
	CDSiteBackend *pNewBackend = &myData.backends[iFileType][iNumSite];
	myData.iNbSitesForType[iFileType] ++;

	pNewBackend->cSiteName = cSiteName;
	pNewBackend->iNbUrls = iNbUrls + 1;  // +1 for tiny-url.
	pNewBackend->cUrlLabels = g_new0 (gchar *, pNewBackend->iNbUrls+1);  // +1 for end NULL.
	memcpy (pNewBackend->cUrlLabels, cUrlLabels, iNbUrls * sizeof (gchar*));  // we take N first labels given by the backend.
	pNewBackend->cUrlLabels[iNbUrls] = D_("Tiny URL");  // + tiny-url.
	pNewBackend->iPreferedUrlType = iPreferedUrlType;
	pNewBackend->upload = pUploadFunc;
}

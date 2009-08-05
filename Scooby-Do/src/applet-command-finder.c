/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-command-finder.h"

static gboolean _cd_do_fill_file_action_entry (CDEntry *pEntry);
static gboolean _cd_do_fill_file_entry (CDEntry *pEntry);
static CDEntry *_cd_do_list_file_sub_entries (CDEntry *pEntry, int *iNbEntries);
static void _cd_do_launch_file (CDEntry *pEntry);
static void _cd_do_show_file_location (CDEntry *pEntry);
static void _cd_do_zip_file (CDEntry *pEntry);
static void _cd_do_zip_folder (CDEntry *pEntry);
static void _cd_do_mail_file (CDEntry *pEntry);
static void _cd_do_move_file (CDEntry *pEntry);
static void _cd_do_copy_url (CDEntry *pEntry);


gboolean cd_do_check_locate_is_available (void)
{
	gchar *cResult = cairo_dock_launch_command_sync ("which locate");
	
	gboolean bAvailable = (cResult != NULL && *cResult != '\0');
	g_free (cResult);
	cd_debug ("locate available : %d", bAvailable);
	
	if (bAvailable)
	{
		gchar *cDirPath = g_strdup_printf ("%s/ScoobyDo", g_cCairoDockDataDir);
		if (! g_file_test (cDirPath, G_FILE_TEST_IS_DIR))
		{
			if (g_mkdir (cDirPath, 7*8*8+7*8+5) != 0)
			{
				cd_warning ("couldn't create directory %s", cDirPath);
				g_free (cDirPath);
				return FALSE;
			}
		}
		
		gchar *cDataBase = g_strdup_printf ("%s/ScoobyDo.db", cDirPath);
		gchar *cLastUpdateFile = g_strdup_printf ("%s/.last-update", cDirPath);
		gboolean bNeedsUpdate = FALSE;
		
		if (! g_file_test (cDataBase, G_FILE_TEST_EXISTS))
		{
			bNeedsUpdate = TRUE;
		}
		else
		{
			if (! g_file_test (cLastUpdateFile, G_FILE_TEST_EXISTS))
			{
				bNeedsUpdate = TRUE;
			}
			else
			{
				gsize length = 0;
				gchar *cContent = NULL;
				g_file_get_contents (cLastUpdateFile,
					&cContent,
					&length,
					NULL);
				if (cContent == NULL || *cContent == '\0')
				{
					bNeedsUpdate = TRUE;
				}
				else
				{
					time_t iLastUpdateTime = atoll (cContent);
					time_t iCurrentTime = (time_t) time (NULL);
					if (iCurrentTime - iLastUpdateTime > 86400)
					{
						bNeedsUpdate = TRUE;
					}
				}
				g_free (cContent);
			}
		}
		
		if (bNeedsUpdate)
		{
			gchar *cCommand = g_strdup_printf ("updatedb -U / --prunepaths=\"/tmp /usr /lib /var /bin /boot /sbin /etc /sys /proc /dev /root\" --prunefs=\"NFS nfs nfs4 rpc_pipefs afs binfmt_misc proc smbfs autofs iso9660 ncpfs coda devpts ftpfs devfs mfs shfs sysfs cifs lustre_lite tmpfs usbfs udf\" -o '%s' -l0", cDataBase);  // -U $HOME
			g_print ("updating or creating data-base with : %s\n", cCommand);
			cairo_dock_launch_command (cCommand);
			g_free (cCommand);
			
			gchar *cDate = g_strdup_printf ("%ld", time (NULL));
			g_file_set_contents (cLastUpdateFile,
				cDate,
				-1,
				NULL);
			g_free (cDate);
		}
		
		g_free (cDataBase);
		g_free (cLastUpdateFile);
		g_free (cDirPath);
	}
	
	return bAvailable;
}


#define NB_ACTIONS_ON_FOLDER 3
static CDEntry *_list_folder (const gchar *cPath, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cPath);
	// on ouvre le repertoire.
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		*iNbEntries = 0;
		return NULL;
	}
	
	// on liste le repertoire entierement, mais sans les icones.
	int iNbFiles = 0;
	while (g_dir_read_name (dir) != NULL)
		iNbFiles ++;
	g_print (" %d fichiers\n", iNbFiles);
	
	g_dir_rewind (dir);
	CDEntry *pNewEntries = g_new0 (CDEntry, iNbFiles+NB_ACTIONS_ON_FOLDER);
	int i = 0;
	CDEntry *pEntry;
	pEntry = &pNewEntries[i++];
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Zip folder"));
	pEntry->cIconName = g_strdup ("zip");
	pEntry->fill = _cd_do_fill_file_action_entry;
	pEntry->execute = _cd_do_zip_folder;
	
	pEntry = &pNewEntries[i++];
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Move to"));
	pEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pEntry->fill = _cd_do_fill_file_action_entry;
	pEntry->execute = _cd_do_move_file;
	
	pEntry = &pNewEntries[i++];
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Copy URL"));
	pEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pEntry->fill = _cd_do_fill_file_action_entry;
	pEntry->execute = _cd_do_copy_url;
	
	const gchar *cFileName;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		pEntry = &pNewEntries[i++];
		
		pEntry->cName = g_strdup (cFileName);
		pEntry->cPath = g_strdup_printf ("%s/%s", cPath, cFileName);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
	}
	while (1);
	g_dir_close (dir);
	
	*iNbEntries = iNbFiles;
	return pNewEntries;
}

#define NB_ACTIONS_ON_FILE 5
static CDEntry *_list_actions_on_file (const gchar *cPath, int *iNbActions)
{
	CDEntry *pNewEntries = g_new0 (CDEntry, NB_ACTIONS_ON_FILE);
	int i = 0;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Open location"));
	pNewEntries[i].cIconName = g_strdup (GTK_STOCK_DIRECTORY);
	pNewEntries[i].fill = _cd_do_fill_file_action_entry;
	pNewEntries[i++].execute = _cd_do_show_file_location;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Zip file"));
	pNewEntries[i].cIconName = g_strdup ("zip");
	pNewEntries[i].fill = _cd_do_fill_file_action_entry;
	pNewEntries[i++].execute = _cd_do_zip_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Mail file"));
	pNewEntries[i].cIconName = g_strdup ("thunderbird");  /// utiliser le client mail par defaut...
	pNewEntries[i].fill = _cd_do_fill_file_action_entry;
	pNewEntries[i++].execute = _cd_do_mail_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Move to"));
	pNewEntries[i].cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pNewEntries[i].fill = _cd_do_fill_file_action_entry;
	pNewEntries[i++].execute = _cd_do_move_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Copy URL"));
	pNewEntries[i].cIconName = g_strdup (GTK_STOCK_COPY);
	pNewEntries[i].fill = _cd_do_fill_file_action_entry;
	pNewEntries[i++].execute = _cd_do_copy_url;
	
	*iNbActions = NB_ACTIONS_ON_FILE;
	return pNewEntries;
}

static gboolean _cd_do_fill_file_action_entry (CDEntry *pEntry)
{
	if (pEntry->cIconName && pEntry->pIconSurface == NULL)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		pEntry->pIconSurface = cairo_dock_create_surface_from_icon (pEntry->cIconName,
			pSourceContext,
			myDialogs.dialogTextDescription.iSize + 2,
			myDialogs.dialogTextDescription.iSize + 2);
		cairo_destroy (pSourceContext);
		return TRUE;
	}
	return FALSE;
}

static gboolean _cd_do_fill_file_entry (CDEntry *pEntry)
{
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fOrder;
	cairo_dock_fm_get_file_info (pEntry->cPath, &cName, &cURI, &cIconName, &pEntry->bIsFolder, &iVolumeID, &fOrder, 0);
	g_free (cName);
	g_free (cURI);
	if (cIconName != NULL && pEntry->pIconSurface == NULL)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		pEntry->pIconSurface = cairo_dock_create_surface_from_icon (cIconName,
			pSourceContext,
			myDialogs.dialogTextDescription.iSize,
			myDialogs.dialogTextDescription.iSize);
		g_free (cIconName);
		cairo_destroy (pSourceContext);
		return TRUE;
	}
	return FALSE;
}

static CDEntry *_cd_do_list_file_sub_entries (CDEntry *pEntry, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	if (pEntry->cPath == NULL)  // on est deja en bout de chaine.
		return NULL;
	if (g_file_test (pEntry->cPath, G_FILE_TEST_IS_DIR))  // on liste les fichiers du repertoire + 2 actions.
	{
		return _list_folder (pEntry->cPath, iNbEntries);
	}
	else  // on liste les actions sur le fichier.
	{
		return _list_actions_on_file (pEntry->cPath, iNbEntries);
	}
}

static void _cd_do_launch_file (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_fm_launch_uri (pEntry->cPath);
}

static void _cd_do_show_file_location (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	gchar *cPathUp = g_path_get_dirname (pEntry->cPath);
	g_return_if_fail (cPathUp != NULL);
	cairo_dock_fm_launch_uri (cPathUp);
	g_free (cPathUp);
}

static void _cd_do_zip_file (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	gchar *cCommand = g_strdup_printf ("zip '%s.zip' '%s'", pEntry->cPath, pEntry->cPath);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

static void _cd_do_zip_folder (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	gchar *cCommand = g_strdup_printf ("tar cfz '%s.tar.gz' '%s'", pEntry->cPath, pEntry->cPath);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

static void _cd_do_mail_file (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	gchar *cCommand = g_strdup_printf ("thunderbird pouet pouet");  /// est-ce possible avec thunderbird ? prendre aussi en compte les autres clients mail, et utiliser celui par defaut...
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

static void _cd_do_move_file (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		D_("Pick up a directory"),
		GTK_WINDOW (g_pMainDock->pWidget),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), FALSE);
	
	gtk_widget_show (pFileChooserDialog);
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		gchar *cDirPath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pFileChooserDialog));
		
		gchar *cFileName = g_path_get_basename (pEntry->cPath);
		gchar *cNewFilePath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		g_return_if_fail (! g_file_test (cNewFilePath, G_FILE_TEST_EXISTS));
		g_free (cFileName);
		g_free (cNewFilePath);
		
		gchar *cCommand = g_strdup_printf ("mv '%s' '%s'", pEntry->cPath, cDirPath);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
	}
	gtk_widget_destroy (pFileChooserDialog);
}

static void _cd_do_copy_url (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text (pClipBoard, pEntry->cPath, -1);
}



static gchar **_cd_do_locate_files (const char *text, gboolean bWithLimit)
{
	GString *sCommand = g_string_new ("locate");
	g_string_append_printf (sCommand, " -d '%s/ScoobyDo/ScoobyDo.db'", g_cCairoDockDataDir);
	if (bWithLimit)
		g_string_append_printf (sCommand, " --limit=%d", myConfig.iNbResultMax);
	if (! (myData.iLocateFilter & DO_MATCH_CASE))
		g_string_append (sCommand, " -i");
	if (*text != '/')
		g_string_append (sCommand, " -b");
	
	if (myData.iLocateFilter == DO_FILTER_NONE || myData.iLocateFilter == DO_MATCH_CASE)
	{
		g_string_append_printf (sCommand, " \"%s\"", text);
	}
	else
	{
		if (myData.iLocateFilter & DO_TYPE_MUSIC)
		{
			g_string_append_printf (sCommand, " \"*%s*.mp3\" \"*%s*.ogg\" \"*%s*.wav\"", text, text, text);
		}
		if (myData.iLocateFilter & DO_TYPE_IMAGE)
		{
			g_string_append_printf (sCommand, " \"*%s*.jpg\" \"*%s*.jpeg\" \"*%s*.png\"", text, text, text);
		}
		if (myData.iLocateFilter & DO_TYPE_VIDEO)
		{
			g_string_append_printf (sCommand, " \"*%s*.avi\" \"*%s*.mkv\" \"*%s*.og[gv]\" \"*%s*.wmv\"", text, text, text, text);
		}
		if (myData.iLocateFilter & DO_TYPE_TEXT)
		{
			g_string_append_printf (sCommand, " \"*%s*.txt\" \"*%s*.odt\" \"*%s*.doc\"", text, text, text);
		}
		if (myData.iLocateFilter & DO_TYPE_HTML)
		{
			g_string_append_printf (sCommand, " \"*%s*.html\" \"*%s*.htm\"", text, text);
		}
		if (myData.iLocateFilter & DO_TYPE_SOURCE)
		{
			g_string_append_printf (sCommand, " \"*%s*.[ch]\" \"*%s*.cpp\"", text, text);
		}
	}
	
	g_print (">>> %s\n", sCommand->str);
	gchar *cResult = cairo_dock_launch_command_sync (sCommand->str);
	if (cResult == NULL || *cResult == '\0')
	{
		g_free (cResult);
		return NULL;
	}
	
	gchar **files = g_strsplit (cResult, "\n", 0);
	g_free (cResult);
	return files;
}

static void _cd_do_search_files (gpointer data)
{
	myData.pMatchingFiles = _cd_do_locate_files (myData.cCurrentLocateText, TRUE);  // TRUE <=> avec limite.
}
static gboolean _cd_do_update_from_files (gpointer data)
{
	if (! cd_do_session_is_waiting_for_input ())  // on a quitte la session en cours de route.
	{
		return FALSE;
	}
	if (myData.iLocateFilter != myData.iCurrentFilter ||
		cairo_dock_strings_differ (myData.cCurrentLocateText, myData.sCurrentText->str))  // la situation a change entre le lancement de la tache et la mise a jour, on va relancer la recherche immediatement.
	{
		if (myData.pMatchingFiles == NULL &&
			myData.sCurrentText->len > 0 &&
			strncmp (myData.cCurrentLocateText, myData.sCurrentText->str, strlen (myData.cCurrentLocateText)) == 0 &&
			myData.iLocateFilter == myData.iCurrentFilter)  // la recherche est identique, seul le texte comporte plus de caracteres; comme on n'a rien trouve, inutile de chercher a nouveau, on quitte.
		{
			g_print ("useless\n");
			myData.bFoundNothing = TRUE;
			return FALSE;
		}
		
		if (myData.pMatchingFiles != NULL)  // on bache tout, sans regret.
		{
			g_strfreev (myData.pMatchingFiles);
			myData.pMatchingFiles = NULL;
			/// effacer aussi le contenu actuel du listing ?...
			if (myData.pListing && myData.pListing->pEntries)
			{
				
			}
		}
		
		if (myData.pMatchingIcons != NULL || myData.sCurrentText->len == 0)  // avec le texte courant on a des applis, on quitte.
		{
			cd_do_hide_listing ();
			return FALSE;
		}
		
		myData.iLocateFilter = myData.iCurrentFilter;
		g_free (myData.cCurrentLocateText);
		myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		cairo_dock_relaunch_task_immediately (myData.pLocateTask, 0);
		return FALSE;
	}
	
	// on parse les resultats.
	myData.bFoundNothing = (myData.pMatchingFiles == NULL);
	int i, iNbEntries;
	if (myData.pMatchingFiles)
		for (iNbEntries = 0; myData.pMatchingFiles[iNbEntries] != NULL; iNbEntries ++);
	CDEntry *pEntries = g_new0 (CDEntry, iNbEntries);
	CDEntry *pEntry;
	for (i = 0; i < iNbEntries; i ++)
	{
		pEntry = &pEntries[i];
		pEntry->cPath = myData.pMatchingFiles[i];
		pEntry->cName = g_path_get_basename (pEntry->cPath);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
	}
	
	// on montre les resultats.
	cd_do_show_listing (pEntries, iNbEntries);
	g_free (myData.pMatchingFiles);  // ses elements sont dans la liste des entrees.
	myData.pMatchingFiles = NULL;  // on n'a plus besoin de la liste.
	
	return FALSE;
}
void cd_do_find_matching_files (void)
{
	if (myData.sCurrentText->len == 0)
		return ;
	
	if (myData.pLocateTask == NULL)
	{
		myData.pLocateTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) _cd_do_search_files,
			(CairoDockUpdateSyncFunc) _cd_do_update_from_files,
			NULL);
	}
	
	if (! cairo_dock_task_is_running (myData.pLocateTask))  // sinon, on la laisse se finir, et lorsqu'elle aura fini, on la relancera avec le nouveau texte/filtre.
	{
		/// vider le listing en attendant la fin de la recherche ?...
		
		g_print ("filtre : %d -> %d (%d)\n", myData.iLocateFilter, myData.iCurrentFilter, myData.iLocateFilter & myData.iCurrentFilter);
		if (/*(myData.iLocateFilter & myData.iCurrentFilter) == myData.iLocateFilter*/myData.iCurrentFilter == myData.iLocateFilter &&  /// a remplacer une fois ecrit le filtre.
			myData.cCurrentLocateText &&
			strncmp (myData.cCurrentLocateText, myData.sCurrentText->str, strlen (myData.cCurrentLocateText)) == 0 &&
			(! myData.pListing || myData.pListing->iNbEntries < myConfig.iNbResultMax))  // c'est une sous-recherche de la precedente.
		{
			g_print ("filtrage de la recherche\n");
			if (myData.pListing != NULL && myData.pListing->pEntries != NULL)
			{
				CDEntry *pEntry;
				int i,j=0;
				for (i = 0; i < myData.pListing->iNbEntries; i ++)
				{
					pEntry = &myData.pListing->pEntries[i];
					if (g_strstr_len (pEntry->cName, -1, myData.sCurrentText->str) != NULL)
					{
						if (i != j)
						{
							cd_do_free_entry (&myData.pListing->pEntries[j]);
							memcpy (&myData.pListing->pEntries[j], pEntry, sizeof (CDEntry));
							memset (pEntry, 0, sizeof (CDEntry));
						}
						j ++;
					}
					
				}
				for (i = j; i < myData.pListing->iNbEntries; i ++)
				{
					pEntry = &myData.pListing->pEntries[i];
					cd_do_free_entry (pEntry);
					memset (pEntry, 0, sizeof (CDEntry));
				}
				
				myData.pListing->iNbEntries = j;
			}
			else
			{
				myData.pListing->iNbEntries = 0;
			}
			if (myData.pListing->iNbEntries > 0)
			{
				myData.bFoundNothing = FALSE;
			}
			else
			{
				myData.bFoundNothing = TRUE;
				g_free (myData.pListing->pEntries);
				myData.pListing->pEntries = NULL;
			}
			
			myData.pListing->iCurrentEntry = 0;
			myData.pListing->iScrollAnimationCount = 0;
			myData.pListing->fAimedOffset = 0;
			myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
			myData.pListing->sens = 1;
			myData.pListing->iTitleOffset = 0;
			myData.pListing->iTitleWidth = 0;
			
			g_free (myData.cCurrentLocateText);
			myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
			cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
		}
		else
		{
			g_print ("on relance la recherche\n");
			myData.iLocateFilter = myData.iCurrentFilter;
			g_free (myData.cCurrentLocateText);
			myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
			
			myData.bFoundNothing = FALSE;
			cd_do_set_status (D_("Searching ..."));
			cairo_dock_launch_task (myData.pLocateTask);
		}
	}
}


void cd_do_activate_filter_option (int iNumOption)
{
	int iMaskOption = 1 << iNumOption;
	if (myData.iCurrentFilter & iMaskOption)  // on enleve l'option => ca fait (beaucoup) plus de resultats.
	{
		myData.iCurrentFilter &= (~iMaskOption);
	}
	else  // on active l'option => ca filtre les resultats courants.
	{
		myData.iCurrentFilter |= iMaskOption;
		if (myData.pListing && myData.pListing->pEntries == NULL)  // on rajoute une contrainte sur une recherche qui ne fournit aucun resultat => on ignore.
		{
			g_print ("useless\n");
			return ;
		}
	}
	g_print ("myData.iCurrentFilter  <- %d\n", myData.iCurrentFilter);
	
	// on cherche les nouveaux fichiers correpondants.
	cd_do_find_matching_files ();  // relance le locate seulement si necessaire.
}



void cd_do_show_current_sub_listing (void)
{
	// on conserve l'entree courante.
	/*if (myData.pCurrentSubEntry == NULL)
		myData.pCurrentSubEntry = g_new0 (CDEntry, 1);
	else
		cd_do_free_entry (myData.pCurrentSubEntry);
	
	CDEntry *pCurrentEntry = &myData.pListing->pEntries[myData.pListing->iCurrentEntry];
	cd_do_backup_entry (myData.pCurrentSubEntry, pCurrentEntry);*/
	
	// on construit la liste des sous-entrees.
	CDEntry *pCurrentEntry = &myData.pListing->pEntries[myData.pListing->iCurrentEntry];
	CDEntry *pNewEntries = NULL;
	int iNbNewEntries = 0;
	if (pCurrentEntry->list)
		pNewEntries = pCurrentEntry->list (pCurrentEntry, &iNbNewEntries);
	if (pNewEntries == NULL)
		return ;
	
	// on enleve les entrees courantes du listing.
	CDEntry *pEntry;
	if (myData.pLocateBackup == NULL)  // on est dans les resultats du locate => on les sauvegarde.
	{
		myData.pLocateBackup = myData.pListing->pEntries;
		myData.iNbEntriesBackup = myData.pListing->iNbEntries;
	}
	else  // on est deja dans un sous-listing => on benne tout.
	{
		guint i;
		for (i = 0; i < myData.pListing->iNbEntries; i ++)
		{
			pEntry = &myData.pListing->pEntries[i];
			cd_do_free_entry (pEntry);
		}
		g_free (myData.pListing->pEntries);
	}
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->iCurrentEntry = 0;
	
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	
	// on montre les nouveaux resultats.
	cd_do_show_listing (pNewEntries, iNbNewEntries);
}

void cd_do_show_previous_listing (void)
{
	if (myData.pLocateBackup == NULL)  // si on n'est pas dans un sous-listing.
		return ;
	
	
}

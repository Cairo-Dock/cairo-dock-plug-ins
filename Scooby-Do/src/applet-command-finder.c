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

static gboolean _cd_do_fill_default_entry (CDEntry *pEntry);
static gboolean _cd_do_fill_file_entry (CDEntry *pEntry);
static CDEntry *_cd_do_list_file_sub_entries (CDEntry *pEntry, int *iNbEntries);
static void _cd_do_launch_file (CDEntry *pEntry);
static void _cd_do_show_file_location (CDEntry *pEntry);
static void _cd_do_zip_file (CDEntry *pEntry);
static void _cd_do_zip_folder (CDEntry *pEntry);
static void _cd_do_mail_file (CDEntry *pEntry);
static void _cd_do_move_file (CDEntry *pEntry);
static void _cd_do_copy_url (CDEntry *pEntry);

static gboolean _cd_do_fill_web_entry (CDEntry *pEntry);
static CDEntry *_cd_do_list_web_sub_entries (CDEntry *pEntry, int *iNbEntries);
static void _cd_do_web_search (CDEntry *pEntry);

static void _cd_do_execute_command (CDEntry *pEntry);


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


  //////////////////
 // FILE BACKEND //
//////////////////

#define NB_ACTIONS_ON_FOLDER 3
static CDEntry *_list_folder (const gchar *cPath, gboolean bNoHiddenFile, int *iNbEntries)
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
	pEntry->fill = _cd_do_fill_default_entry;
	pEntry->execute = _cd_do_zip_folder;
	
	pEntry = &pNewEntries[i++];
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Move to"));
	pEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pEntry->fill = _cd_do_fill_default_entry;
	pEntry->execute = _cd_do_move_file;
	
	pEntry = &pNewEntries[i++];
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Copy URL"));
	pEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pEntry->fill = _cd_do_fill_default_entry;
	pEntry->execute = _cd_do_copy_url;
	
	const gchar *cFileName;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		if (bNoHiddenFile && *cFileName == '.')
			continue;
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
	pNewEntries[i].fill = _cd_do_fill_default_entry;
	pNewEntries[i++].execute = _cd_do_show_file_location;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Zip file"));
	pNewEntries[i].cIconName = g_strdup ("zip");
	pNewEntries[i].fill = _cd_do_fill_default_entry;
	pNewEntries[i++].execute = _cd_do_zip_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Mail file"));
	pNewEntries[i].cIconName = g_strdup ("thunderbird");  /// utiliser le client mail par defaut...
	pNewEntries[i].fill = _cd_do_fill_default_entry;
	pNewEntries[i++].execute = _cd_do_mail_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Move to"));
	pNewEntries[i].cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pNewEntries[i].fill = _cd_do_fill_default_entry;
	pNewEntries[i++].execute = _cd_do_move_file;
	
	pNewEntries[i].cPath = g_strdup (cPath);
	pNewEntries[i].cName = g_strdup (D_("Copy URL"));
	pNewEntries[i].cIconName = g_strdup (GTK_STOCK_COPY);
	pNewEntries[i].fill = _cd_do_fill_default_entry;
	pNewEntries[i++].execute = _cd_do_copy_url;
	
	*iNbActions = NB_ACTIONS_ON_FILE;
	return pNewEntries;
}

static gboolean _cd_do_fill_default_entry (CDEntry *pEntry)
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
	cairo_dock_fm_get_file_info (pEntry->cPath, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
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
	if (g_file_test (pEntry->cPath, G_FILE_TEST_IS_DIR))  // on liste les fichiers du repertoire et les actions sur le repertoire.
	{
		return _list_folder (pEntry->cPath, TRUE, iNbEntries);  // TRUE <=> no hidden files.
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
	gchar *cURI = g_filename_to_uri (pEntry->cPath, NULL, NULL);
	gchar *cCommand = g_strdup_printf ("thunderbird -compose \"attachment=%s\"", cURI);  /// prendre aussi en compte les autres clients mail, et utiliser celui par defaut...
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
	g_free (cURI);
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


  /////////////////
 // WEB BACKEND //
/////////////////

static gboolean _cd_do_fill_web_entry (CDEntry *pEntry)
{
	if (pEntry->cIconName && pEntry->pIconSurface == NULL)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		gchar *cImagePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/", pEntry->cIconName, NULL);
		pEntry->pIconSurface = cairo_dock_create_surface_from_icon (cImagePath,
			pSourceContext,
			myDialogs.dialogTextDescription.iSize + 2,
			myDialogs.dialogTextDescription.iSize + 2);
		cairo_destroy (pSourceContext);
		g_free (cImagePath);
		return TRUE;
	}
	return FALSE;
}
#define NB_WEB_ENGINES 3
static CDEntry *_cd_do_list_web_sub_entries (CDEntry *pEntry, int *iNbEntries)
{
	CDEntry *pNewEntries = g_new0 (CDEntry, NB_ACTIONS_ON_FILE);
	int i = 0;
	
	pNewEntries[i].cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
	pNewEntries[i].cName = g_strdup (D_("Google"));
	pNewEntries[i].cIconName = g_strdup ("google.png");
	pNewEntries[i].fill = _cd_do_fill_web_entry;
	pNewEntries[i++].execute = _cd_do_web_search;
	
	pNewEntries[i].cPath = g_strdup ("http://en.wikipedia.org/w/index.php?title=Special:Search&go=Go&search=%s");
	pNewEntries[i].cName = g_strdup (D_("Wikipedia"));
	pNewEntries[i].cIconName = g_strdup ("wikipedia.png");
	pNewEntries[i].fill = _cd_do_fill_web_entry;
	pNewEntries[i++].execute = _cd_do_web_search;
	
	pNewEntries[i].cPath = g_strdup ("http://search.yahoo.com/search?p=%s&ie=utf-8");
	pNewEntries[i].cName = g_strdup (D_("Yahoo"));
	pNewEntries[i].cIconName = g_strdup ("yahoo.png");
	pNewEntries[i].fill = _cd_do_fill_web_entry;
	pNewEntries[i++].execute = _cd_do_web_search;
	
	*iNbEntries = NB_WEB_ENGINES;
	return pNewEntries;
}
static void _cd_do_web_search (CDEntry *pEntry)
{
	gchar *cEscapedText = g_uri_escape_string (myData.cSearchText,
		"",
		TRUE);
	g_print ("cEscapedText : %s\n", cEscapedText);
	gchar *cURI = g_strdup_printf (pEntry->cPath, cEscapedText);
	cairo_dock_fm_launch_uri (cURI);
	g_free (cURI);
	g_free (cEscapedText);
}

  /////////////////////
 // COMMAND BACKEND //
/////////////////////

static void _cd_do_execute_command (CDEntry *pEntry)
{
	gchar *cCommand = g_strdup_printf ("%s/calc.sh '%s'", MY_APPLET_SHARE_DATA_DIR, myData.sCurrentText->str);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult != NULL && strcmp (cResult, "0") != 0)
	{
		g_print (" resultat du calcul : '%s'\n", cResult);
		GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cResult, -1);
		Icon *pIcon = cairo_dock_get_dialogless_icon ();
		cairo_dock_show_temporary_dialog_with_icon (D_("The value %s has been copied into the clipboard."),
			pIcon,
			CAIRO_CONTAINER (g_pMainDock),
			3000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			cResult);
	}
	else  // le calcul n'a rien donne, on execute sans chercher.
	{
		g_print (" pas un calcul => on execute '%s'\n", myData.sCurrentText->str);
		cairo_dock_launch_command (myData.sCurrentText->str);
	}
	g_free (cResult);
}


  ////////////////////////
 // FILE SEARCH ENGINE //
////////////////////////

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
			g_string_append_printf (sCommand, " \"*%s*.avi\" \"*%s*.mkv\" \"*%s*.ogv\" \"*%s*.wmv\" \"*%s*.mov\"", text, text, text, text, text);
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


  //////////////////////
 // LISTING & FILTER //
//////////////////////

static void _cd_do_search_files (gpointer data)
{
	myData.pMatchingFiles = _cd_do_locate_files (myData.cCurrentLocateText, TRUE);  // TRUE <=> avec limite.
}
static gboolean _cd_do_update_from_files (gpointer data)
{
	g_print ("%s ()\n", __func__);
	if (! cd_do_session_is_waiting_for_input () || myData.pListingHistory != NULL)  // on a quitte la session ou on a choisi une entree en cours de route.
	{
		g_print (" on a quitte la session ou on a choisi une entree en cours de route\n");
		g_strfreev (myData.pMatchingFiles);
		myData.pMatchingFiles = NULL;
		return FALSE;
	}
	
	if (myData.iLocateFilter != myData.iCurrentFilter ||
		cairo_dock_strings_differ (myData.cCurrentLocateText, myData.sCurrentText->str))  // la situation a change entre le lancement de la tache et la mise a jour, on va relancer la recherche immediatement.
	{
		if ((myData.iLocateFilter & myData.iCurrentFilter) == myData.iLocateFilter &&
			myData.cCurrentLocateText &&
			strncmp (myData.cCurrentLocateText, myData.sCurrentText->str, strlen (myData.cCurrentLocateText)) == 0)  // c'est une sous-recherche.
		{
			g_print (" c'est une sous-recherche\n");
			if (myData.pMatchingFiles == NULL)  // on n'a rien trouve, on vide le listing et on ne la relance pas.
			{
				myData.bFoundNothing = TRUE;
				cd_do_show_listing (NULL, 0);
				return FALSE;
			}
			else  // la recherche a ete fructueuse, on regarde si on peut la filtrer ou s'il faut la relancer.
			{
				int iNbEntries = 0;
				if (myData.pMatchingFiles)
					for (iNbEntries = 0; myData.pMatchingFiles[iNbEntries] != NULL; iNbEntries ++);
				if (iNbEntries < myConfig.iNbResultMax)  // on a des resultats mais pas trop, on les charge et on lance le filtre.
				{
					myData.bFoundNothing = FALSE;
					
					CDEntry *pEntries = g_new0 (CDEntry, iNbEntries + 2);
					CDEntry *pEntry;
					int i = 0;
					
					pEntry = &pEntries[i++];
					pEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
					pEntry->cName = g_strdup (D_("Search on the web"));
					pEntry->cIconName = g_strdup ("google.png");
					pEntry->execute = _cd_do_web_search;
					pEntry->fill = _cd_do_fill_web_entry;
					pEntry->list = _cd_do_list_web_sub_entries;
					
					pEntry = &pEntries[i++];
					pEntry->cName = g_strdup (D_("Execute"));
					pEntry->cIconName = g_strdup (GTK_STOCK_EXECUTE);
					pEntry->execute = _cd_do_execute_command;
					pEntry->fill = _cd_do_fill_default_entry;
					pEntry->list = NULL;
					
					for (i = 0; i < iNbEntries; i ++)
					{
						pEntry = &pEntries[i+2];
						pEntry->cPath = myData.pMatchingFiles[i];
						pEntry->cName = g_path_get_basename (pEntry->cPath);
						pEntry->fill = _cd_do_fill_file_entry;
						pEntry->execute = _cd_do_launch_file;
						pEntry->list = _cd_do_list_file_sub_entries;
					}
					
					// on montre les resultats.
					cd_do_show_listing (pEntries, iNbEntries + 2);
					g_free (myData.pMatchingFiles);  // ses elements sont dans la liste des entrees.
					myData.pMatchingFiles = NULL;
					myData.cSearchText = g_strdup (myData.cCurrentLocateText);
					
					cd_do_filter_current_listing ();
					//cd_do_find_matching_files ();
					return FALSE;
				}
				else  // on a trop de resultats, on bache tout et on relance.
				{
					if (myData.pMatchingFiles != NULL)  // on bache tout, sans regret.
					{
						g_strfreev (myData.pMatchingFiles);
						myData.pMatchingFiles = NULL;
					}
				}
			}
		}
		else  // c'est une nouvelle recherche, on bache tout et on relance.
		{
			g_print (" c'est une nouvelle recherche, on bache tout et on relance\n");
			if (myData.pMatchingFiles != NULL)  // on bache tout, sans regret.
			{
				g_strfreev (myData.pMatchingFiles);
				myData.pMatchingFiles = NULL;
			}
			
			if (myData.pMatchingIcons != NULL || myData.sCurrentText->len == 0)  // avec le texte courant on a des applis, on quitte.
			{
				cd_do_hide_listing ();
				g_free (myData.cCurrentLocateText);
				myData.cCurrentLocateText = NULL;
				myData.iLocateFilter = 0;
				return FALSE;
			}
		}
		
		// on relance.
		g_print (" on relance\n");
		myData.bFoundNothing = FALSE;
		cd_do_set_status (D_("Searching ..."));
		myData.iLocateFilter = myData.iCurrentFilter;
		g_free (myData.cCurrentLocateText);
		myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		cairo_dock_relaunch_task_immediately (myData.pLocateTask, 0);
		return FALSE;
	}
	
	// on parse les resultats.
	g_print (" on parse les resultats\n");
	myData.bFoundNothing = (myData.pMatchingFiles == NULL);
	int i, iNbEntries = 0;
	if (myData.pMatchingFiles)
		for (iNbEntries = 0; myData.pMatchingFiles[iNbEntries] != NULL; iNbEntries ++);
				
	
	CDEntry *pEntries = g_new0 (CDEntry, iNbEntries + 2);
	CDEntry *pEntry;
	
	i = 0;
	pEntry = &pEntries[i++];
	pEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
	pEntry->cName = g_strdup (D_("Search on the web"));
	pEntry->cIconName = g_strdup ("google.png");
	pEntry->execute = _cd_do_web_search;
	pEntry->fill = _cd_do_fill_web_entry;
	pEntry->list = _cd_do_list_web_sub_entries;
	
	pEntry = &pEntries[i++];
	pEntry->cName = g_strdup (D_("Execute"));
	pEntry->cIconName = g_strdup (GTK_STOCK_EXECUTE);
	pEntry->execute = _cd_do_execute_command;
	pEntry->fill = _cd_do_fill_default_entry;
	pEntry->list = NULL;
	
	for (i = 0; i < iNbEntries; i ++)
	{
		pEntry = &pEntries[i+2];
		pEntry->cPath = myData.pMatchingFiles[i];
		pEntry->cName = g_path_get_basename (pEntry->cPath);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
	}
	
	// on montre les resultats.
	myData.cSearchText = g_strdup (myData.cCurrentLocateText);
	cd_do_show_listing (pEntries, iNbEntries + 2);
	g_free (myData.pMatchingFiles);  // ses elements sont dans la liste des entrees.
	myData.pMatchingFiles = NULL;  // on n'a plus besoin de la liste.
	
	return FALSE;
}
void cd_do_find_matching_files (void)
{
	g_print ("%s ()\n", __func__);
	if (myData.sCurrentText->len == 0)  // pas de texte, donc rien a chercher.
		return ;
	
	if (myData.pLocateTask == NULL)  // la tache est creee la 1ere fois.
	{
		myData.pLocateTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) _cd_do_search_files,
			(CairoDockUpdateSyncFunc) _cd_do_update_from_files,
			NULL);
	}
	
	if (cairo_dock_task_is_running (myData.pLocateTask))  // on la laisse se finir, et lorsqu'elle aura fini, on la relancera avec le nouveau texte/filtre.
	{
		g_print (" on laisse la tache courante se finir\n");
		return ;
	}
	
	//g_print ("filtre : %d -> %d (%d)\n", myData.iLocateFilter, myData.iCurrentFilter, myData.iLocateFilter & myData.iCurrentFilter);
	if (myData.pListingHistory != NULL ||
		((myData.iLocateFilter & myData.iCurrentFilter) == myData.iLocateFilter &&
		myData.cCurrentLocateText &&
		strncmp (myData.cCurrentLocateText, myData.sCurrentText->str, strlen (myData.cCurrentLocateText)) == 0 &&
		(! myData.pListing || myData.pListing->iNbEntries < myConfig.iNbResultMax)))  // c'est une sous-recherche de la precedente qui etait fructueuse, ou un filtre sur un sous-listing.
	{
		g_print ("filtrage de la recherche\n");
		cd_do_filter_current_listing ();
		
		if (myData.pListing->iNbVisibleEntries > 0)
		{
			myData.bFoundNothing = FALSE;
			if (myData.pListing->iNbEntries >= myConfig.iNbResultMax)
				cd_do_set_status_printf ("> %d results", myConfig.iNbResultMax);
			else
				cd_do_set_status_printf ("%d %s", myData.pListing->iNbEntries, myData.pListing->iNbEntries > 1 ? D_("results") : D_("result"));
		}
		else
		{
			myData.bFoundNothing = TRUE;
			cd_do_set_status (D_("No result"));
		}
		
		myData.pListing->iCurrentEntry = MIN (myConfig.iNbLinesInListing, myData.pListing->iNbVisibleEntries) / 2;
		myData.pListing->iScrollAnimationCount = 0;
		myData.pListing->fAimedOffset = 0;
		myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
		myData.pListing->sens = 1;
		myData.pListing->iTitleOffset = 0;
		myData.pListing->iTitleWidth = 0;
		
		cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
	}
	else  // soit c'est une recherche differente, soit la recherche precedente avait fourni trop de resultats => on (re)lance la recherche.
	{
		g_print ("on (re)lance la recherche\n");
		myData.iLocateFilter = myData.iCurrentFilter;
		g_free (myData.cCurrentLocateText);
		myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		
		myData.bFoundNothing = FALSE;
		cd_do_set_status (D_("Searching ..."));
		cairo_dock_launch_task (myData.pLocateTask);
	}
}


void cd_do_activate_filter_option (int iNumOption)
{
	g_print ("%s (%d)\n", __func__, iNumOption);
	int iMaskOption = (1 << iNumOption);
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
	if (cairo_dock_task_is_running (myData.pLocateTask))
		return ;
	
	// on construit la liste des sous-entrees de l'entree courante.
	CDEntry *pEntry = &myData.pListing->pEntries[myData.pListing->iCurrentEntry];
	CDEntry *pNewEntries = NULL;
	int iNbNewEntries = 0;
	if (pEntry->list)
		pNewEntries = pEntry->list (pEntry, &iNbNewEntries);
	if (pNewEntries == NULL)
		return ;
	
	// on enleve le listing courant et on le conserve dans l'historique.
	CDListingBackup *pBackup = g_new0 (CDListingBackup, 1);
	pBackup->pEntries = myData.pListing->pEntries;
	pBackup->iNbEntries = myData.pListing->iNbEntries;
	pBackup->iCurrentEntry = myData.pListing->iCurrentEntry;
	
	myData.pListingHistory = g_list_prepend (myData.pListingHistory, pBackup);
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->iNbVisibleEntries = 0;
	myData.pListing->iCurrentEntry = 0;
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	
	// on montre les nouveaux resultats.
	cd_do_show_listing (pNewEntries, iNbNewEntries);
}

void cd_do_show_previous_listing (void)
{
	if (myData.pListingHistory == NULL)  // si on n'est pas dans un sous-listing.
		return ;
	if (cairo_dock_task_is_running (myData.pLocateTask))
		return ;
	
	// on recupere le precedent sous-listing.
	CDListingBackup *pBackup = myData.pListingHistory->data;
	myData.pListingHistory = g_list_delete_link (myData.pListingHistory, myData.pListingHistory);
	
	// on enleve le sous-listing courant.
	gint i;
	CDEntry *pEntry;
	for (i = 0; i < myData.pListing->iNbEntries; i ++)
	{
		pEntry = &myData.pListing->pEntries[i];
		cd_do_free_entry (pEntry);
	}
	g_free (myData.pListing->pEntries);
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->iCurrentEntry = 0;
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	
	// on charge le nouveau sous-listing.
	cd_do_show_listing (pBackup->pEntries, pBackup->iNbEntries);  // les entrees du backup appartiennent desormais au listing.
	g_free (pBackup);
}


void cd_do_filter_current_listing (void)
{
	if (myData.pListing == NULL || myData.pListing->pEntries == NULL)
		return ;
	
	gchar *cPattern = ((myData.iCurrentFilter & DO_MATCH_CASE) ? g_strdup (myData.sCurrentText->str) : g_ascii_strdown (myData.sCurrentText->str, -1));
	gchar *cHayStack;
	CDEntry *pEntry;
	int i,j=0;
	gchar *ext;
	for (i = 0; i < myData.pListing->iNbEntries; i ++)
	{
		pEntry = &myData.pListing->pEntries[i];
		if (! pEntry->cName)
		{
			cd_warning ("l'entree n°%d/%d est vide !", i, myData.pListing->iNbEntries);
			continue ;
		}
		ext = strrchr (pEntry->cName, '.');
		if (ext)
			ext ++;
		if (myData.iCurrentFilter & DO_MATCH_CASE)
		{
			cHayStack = pEntry->cName;
		}
		else
		{
			if (pEntry->cCaseDownName == NULL)
				pEntry->cCaseDownName = g_ascii_strdown (pEntry->cName, -1);
			cHayStack = pEntry->cCaseDownName;
		}
		if (g_strstr_len (cHayStack, -1, cPattern) != NULL &&
			(!(myData.iCurrentFilter & DO_TYPE_MUSIC)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "mp3") == 0
				|| g_ascii_strcasecmp (ext, "ogg") == 0
				|| g_ascii_strcasecmp (ext, "wav") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_IMAGE)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "jpg") == 0
				|| g_ascii_strcasecmp (ext, "jpeg") == 0
				|| g_ascii_strcasecmp (ext, "png") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_VIDEO)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "avi") == 0
				|| g_ascii_strcasecmp (ext, "mkv") == 0
				|| g_ascii_strcasecmp (ext, "ogv") == 0
				|| g_ascii_strcasecmp (ext, "wmv") == 0
				|| g_ascii_strcasecmp (ext, "mov") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_TEXT)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "txt") == 0
				|| g_ascii_strcasecmp (ext, "odt") == 0
				|| g_ascii_strcasecmp (ext, "doc") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_HTML)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "html") == 0
				|| g_ascii_strcasecmp (ext, "htm") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_SOURCE)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "c") == 0
				|| g_ascii_strcasecmp (ext, "h") == 0
				|| g_ascii_strcasecmp (ext, "cpp") == 0))))
		{
			//g_print (" %s a passe le filtre\n", pEntry->cName);
			pEntry->bHidden = FALSE;
			j ++;
		}
		else
		{
			pEntry->bHidden = TRUE;
		}
	}
	
	myData.pListing->iNbVisibleEntries = j;
	cd_do_fill_listing_entries (myData.pListing);
	
	if (myData.pListing->pEntries[myData.pListing->iCurrentEntry].bHidden)
		myData.pListing->iCurrentEntry = MIN (myConfig.iNbLinesInListing, myData.pListing->iNbVisibleEntries) / 2;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	myData.pListing->sens = 1;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->iTitleWidth = 0;
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-search.h"
#include "applet-backend-files.h"

// sub-listing
static GList *_cd_do_list_file_sub_entries (CDEntry *pEntry, int *iNbEntries);
// fill entry
static gboolean _cd_do_fill_file_entry (CDEntry *pEntry);
// actions
static void _cd_do_launch_file (CDEntry *pEntry);
static void _cd_do_show_file_location (CDEntry *pEntry);
static void _cd_do_zip_file (CDEntry *pEntry);
static void _cd_do_zip_folder (CDEntry *pEntry);
static void _cd_do_mail_file (CDEntry *pEntry);
static void _cd_do_move_file (CDEntry *pEntry);
static void _cd_do_copy_url (CDEntry *pEntry);

  //////////
 // INIT //
//////////

static gboolean init (gpointer *pData)
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
			cairo_dock_launch_command (MY_APPLET_SHARE_DATA_DIR"/updatedb.sh");
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


  /////////////////
 // SUB-LISTING //
/////////////////

#define NB_ACTIONS_ON_FOLDER 3
static GList *_list_folder (const gchar *cPath, gboolean bNoHiddenFile, int *iNbEntries)
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
	
	// on ajoute les entrees d'actions sur le repertoire.
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Zip folder"));
	pEntry->cIconName = g_strdup ("zip");
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_zip_folder;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Move to"));
	pEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_move_file;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Copy URL"));
	pEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_copy_url;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	// on ajoute une entree par fichier du repertoire.
	int iNbFiles = 0;
	const gchar *cFileName;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		if (bNoHiddenFile && *cFileName == '.')
			continue;
		
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cName = g_strdup (cFileName);
		pEntry->cPath = g_strdup_printf ("%s/%s", cPath, cFileName);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
		pEntries = g_list_prepend (pEntries, pEntry);
		iNbFiles ++;
	}
	while (1);
	g_dir_close (dir);
	
	*iNbEntries = iNbFiles + NB_ACTIONS_ON_FOLDER;
	return pEntries;
}

#define NB_ACTIONS_ON_FILE 5
static GList *_list_actions_on_file (const gchar *cPath, int *iNbEntries)
{
	g_print ("%s ()\n", __func__);
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Open location"));
	pEntry->cIconName = g_strdup (GTK_STOCK_DIRECTORY);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_show_file_location;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Zip file"));
	pEntry->cIconName = g_strdup ("zip");
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_zip_file;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Mail file"));
	pEntry->cIconName = g_strdup ("thunderbird");  /// utiliser le client mail par defaut...
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_mail_file;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Move to"));
	pEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_move_file;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Copy URL"));
	pEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_copy_url;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	*iNbEntries = NB_ACTIONS_ON_FILE;
	return pEntries;
}

static GList *_cd_do_list_file_sub_entries (CDEntry *pEntry, int *iNbEntries)
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


  ////////////////
 // FILL ENTRY //
////////////////

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


  /////////////
 // ACTIONS //
/////////////

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


  ////////////
 // SEARCH //
////////////

static gchar *_locate_files (const char *text, int iFilter, gboolean bWithLimit)
{
	GString *sCommand = g_string_new ("locate");
	g_string_append_printf (sCommand, " -d '%s/ScoobyDo/ScoobyDo.db'", g_cCairoDockDataDir);
	if (bWithLimit)
		g_string_append_printf (sCommand, " --limit=%d", myConfig.iNbResultMax);
	if (! (iFilter & DO_MATCH_CASE))
		g_string_append (sCommand, " -i");
	if (*text != '/')
		g_string_append (sCommand, " -b");
	
	if (iFilter == DO_FILTER_NONE || iFilter == DO_MATCH_CASE)
	{
		g_string_append_printf (sCommand, " \"%s\"", text);
	}
	else
	{
		if (iFilter & DO_TYPE_MUSIC)
		{
			g_string_append_printf (sCommand, " \"*%s*.mp3\" \"*%s*.ogg\" \"*%s*.wav\"", text, text, text);
		}
		if (iFilter & DO_TYPE_IMAGE)
		{
			g_string_append_printf (sCommand, " \"*%s*.jpg\" \"*%s*.jpeg\" \"*%s*.png\"", text, text, text);
		}
		if (iFilter & DO_TYPE_VIDEO)
		{
			g_string_append_printf (sCommand, " \"*%s*.avi\" \"*%s*.mkv\" \"*%s*.ogv\" \"*%s*.wmv\" \"*%s*.mov\"", text, text, text, text, text);
		}
		if (iFilter & DO_TYPE_TEXT)
		{
			g_string_append_printf (sCommand, " \"*%s*.txt\" \"*%s*.odt\" \"*%s*.doc\"", text, text, text);
		}
		if (iFilter & DO_TYPE_HTML)
		{
			g_string_append_printf (sCommand, " \"*%s*.html\" \"*%s*.htm\"", text, text);
		}
		if (iFilter & DO_TYPE_SOURCE)
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
	
	return cResult;
}

static GList * _build_entries (gchar *cResult, int *iNbEntries)
{
	gchar **pMatchingFiles = g_strsplit (cResult, "\n", 0);
	
	GList *pEntries = NULL;
	CDEntry *pEntry;
	int i;
	for (i = 0; pMatchingFiles[i] != NULL; i ++)
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = pMatchingFiles[i];
		pEntry->cName = g_path_get_basename (pEntry->cPath);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
		pEntries = g_list_prepend (pEntries, pEntry);
	}
	g_free (pMatchingFiles);
	
	*iNbEntries = i;
	return pEntries;
}

static GList* search (const gchar *cText, int iFilter, gpointer pData, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	gchar *cResult = _locate_files (cText, iFilter, TRUE);  // TRUE <=> avec limite.
	
	if (cResult == NULL)
	{
		*iNbEntries = 0;
		return NULL;
	}
	GList *pEntries = _build_entries (cResult, iNbEntries);
	g_free (cResult);
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_files_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Files";
	pBackend->bIsThreaded = TRUE;
	pBackend->init =(CDBackendInitFunc) init;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}

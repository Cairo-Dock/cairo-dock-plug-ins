/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#define __USE_BSD 1
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "applet-struct.h"
#include "applet-command-finder.h"
#include "applet-session.h"

static void _browse_dir (const gchar *cDirPath);

static GList *
fill_files_from (const char *dirname,
		const char *dirprefix,
		char        prefix,
		GList      *existing_items)
{
	GList         *list;
	DIR           *dir;
	struct dirent *dent;
	
	list = NULL;
	dir = opendir (dirname);
	
	if (!dir)
		return list;
	
	while ((dent = readdir (dir))) {
		char       *file;
		char       *item;
		const char *suffix;
		
		if (!dent->d_name ||
		    dent->d_name [0] != prefix)
			continue;

		file = g_build_filename (dirname, dent->d_name, NULL);
		
		suffix = NULL;
		if (
		// don't use g_file_test at first so we don't stat()
		    dent->d_type == DT_DIR ||
		    (dent->d_type == DT_LNK &&
		     g_file_test (file, G_FILE_TEST_IS_DIR))
			//g_file_test (file, G_FILE_TEST_IS_DIR)
		   )
			suffix = "/";
		
		g_free (file);
		
		item = g_build_filename (dirprefix, dent->d_name, suffix, NULL);
		
		list = g_list_prepend (list, item);
	}

	closedir (dir);
	
	return list;
}	

static GList *
fill_possible_executables (void)
{
	GList         *list;
	const char    *path;
	char         **pathv;
	int            i;
	
	list = NULL;
	path = g_getenv ("PATH");

	if (!path || !path [0])
		return list;

	pathv = g_strsplit (path, ":", 0);
	
	for (i = 0; pathv [i]; i++) {
		const char *file;
		char       *filename;
		GDir       *dir;

		dir = g_dir_open (pathv [i], 0, NULL);

		if (!dir)
			continue;

		while ((file = g_dir_read_name (dir))) {
			filename = g_build_filename (pathv [i], file, NULL);
			list = g_list_prepend (list, filename);
		}

		g_dir_close (dir);
	}
	
	g_strfreev (pathv);
	
	return list;
}

static GList *
fill_executables (GList *possible_executables,
		GList *existing_items,
		char   prefix)
{
	GList *list;
	GList *l;
	
	list = NULL;
	
	for (l = possible_executables; l; l = l->next) {
		const char *filename;
		char       *basename;
			
		filename = l->data;
		basename = g_path_get_basename (filename);
			
		if (basename [0] == prefix &&
		    g_file_test (filename, G_FILE_TEST_IS_REGULAR) &&
		    g_file_test (filename, G_FILE_TEST_IS_EXECUTABLE)) {
			    
			if (g_list_find_custom (existing_items, basename,
						(GCompareFunc) strcmp)) {
				g_free (basename);
				return NULL;
			}

			list = g_list_prepend (list, basename);
		 } else {
			g_free (basename);
		 }
	}
	
	return list;
}

void cd_do_update_completion (const char *text)
{
	GList *list;
	GList *executables;
	char   prefix;
	char  *buf;
	char  *dirname;
	char  *dirprefix;
	char  *key;

	g_assert (text != NULL && *text != '\0' && !g_ascii_isspace (*text));
	
	list = NULL;
	executables = NULL;

	if (!myData.completion) {
		myData.completion = g_completion_new (NULL);
		myData.possible_executables = fill_possible_executables ();
		myData.dir_hash = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free, NULL);
	}
	
	buf = g_path_get_basename (text);
	prefix = buf[0];
	g_free (buf);
	if (prefix == '/' || prefix == '.')
		return;

	if (text [0] == '/') {
		/* complete against absolute path */
		dirname = g_path_get_dirname (text);
		dirprefix = g_strdup (dirname);
	} else {
		/* complete against relative path and executable name */
		if (!strchr (text, '/')) {
			executables = fill_executables (myData.possible_executables,
							myData.completion_items,
							text [0]);
			dirprefix = g_strdup ("");
		} else {
			dirprefix = g_path_get_dirname (text);
		}

		dirname = g_build_filename (g_get_home_dir (), dirprefix, NULL);
	}

	key = g_strdup_printf ("%s%c%c", dirprefix, G_DIR_SEPARATOR, prefix);

	if (!g_hash_table_lookup (myData.dir_hash, key)) {
		g_hash_table_insert (myData.dir_hash, key, myApplet);

		list = fill_files_from (dirname, dirprefix, prefix,
					myData.completion_items);
	} else {
		g_free (key);
	}

	list = g_list_concat (list, executables);

	g_free (dirname);
	g_free (dirprefix);

	if (list == NULL)
		return;
		
	g_completion_add_items (myData.completion, list);
		
	myData.completion_items = g_list_concat (myData.completion_items, list);
}



gboolean cd_do_check_locate_is_available (void)
{
	gchar *standard_output=NULL, *standard_error=NULL;
	gint exit_status=0;
	GError *erreur = NULL;
	gboolean r = g_spawn_command_line_sync ("which locate",
		&standard_output,
		&standard_error,
		&exit_status,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	if (standard_error != NULL && *standard_error != '\0')
	{
		cd_warning (standard_error);
	}
	
	gboolean bAvailable = (standard_output != NULL && *standard_output != '\0');
	g_free (standard_output);
	g_free (standard_error);
	cd_debug ("locate available : %d", bAvailable);
	return bAvailable;
}

static gchar **_cd_do_locate_files (const char *text)
{
	gchar *standard_output=NULL, *standard_error=NULL;
	gint exit_status=0;
	GError *erreur = NULL;
	GString *sCommand = g_string_new ("");
	g_string_printf (sCommand, "locate %s --limit=%d %s ",
		(myData.bMatchCase ? "" : "-i"),
		myConfig.iNbResultMax+1,
		(*text != '/' ? "-b" : ""));
	
	if (myData.iCurrentFilter == DO_TYPE_NONE)
	{
		g_string_append_printf (sCommand, "\"%s\"", text);
	}
	else
	{
		if (myData.iCurrentFilter & DO_TYPE_MUSIC)
		{
			g_string_append_printf (sCommand, " \"*%s*.mp3\" \"*%s*.ogg\" \"*%s*.wav\"", text, text, text);
		}
		if (myData.iCurrentFilter & DO_TYPE_IMAGE)
		{
			g_string_append_printf (sCommand, " \"*%s*.jpg\" \"*%s*.jpeg\" \"*%s*.png\"", text, text, text);
		}
		if (myData.iCurrentFilter & DO_TYPE_VIDEO)
		{
			g_string_append_printf (sCommand, " \"*%s*.avi\" \"*%s*.mkv\" \"*%s*.ogg\" \"*%s*.wmv\"", text, text, text, text);
		}
		if (myData.iCurrentFilter & DO_TYPE_TEXT)
		{
			g_string_append_printf (sCommand, " \"*%s*.txt\" \"*%s*.odt\" \"*%s*.doc\"", text, text, text);
		}
		if (myData.iCurrentFilter & DO_TYPE_HTML)
		{
			g_string_append_printf (sCommand, " \"*%s*.html\" \"*%s*.htm\"", text, text);
		}
		if (myData.iCurrentFilter & DO_TYPE_SOURCE)
		{
			g_string_append_printf (sCommand, " \"*%s*.c\" \"*%s*.cpp\" \"*%s*.h\"", text, text, text);
		}
	}
	g_print (">>> %s\n", sCommand->str);
	gboolean r = g_spawn_command_line_sync (sCommand->str,
		&standard_output,
		&standard_error,
		&exit_status,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	if (standard_error != NULL && *standard_error != '\0')
	{
		cd_warning (standard_error);
	}
	
	gchar **files = g_strsplit (standard_output, "\n", 0);
	if (files)
	{
		int i;
		for (i = 0; files[i] != NULL; i ++)  // on vire la derniere ligne blanche.
		{
			if (*files[i] == '\0')
			{
				g_free (files[i]);
				files[i] = NULL;
				break ;
			}
		}
	}
	
	
	g_free (standard_output);
	g_free (standard_error);
	g_string_free (sCommand, TRUE);
	return files;
}


static void _on_activate_item (GtkWidget *pMenuItem, gchar *cPath)
{
	g_print ("%s (%s)\n", __func__, cPath);
	if (cPath == NULL)
		return ;
	cairo_dock_fm_launch_uri (cPath);
	cd_do_close_session ();
}
static void _on_delete_menu (GtkMenuShell *menu, CairoDock *pDock)
{
	gtk_window_present (GTK_WINDOW (pDock->pWidget));
}

static void inline _cd_do_make_info (const gchar *cInfo)
{
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	myData.pInfoSurface = cairo_dock_create_surface_from_text (cInfo, pCairoContext, &myConfig.infoDescription, &myData.iInfoWidth, &myData.iInfoHeight);
	if (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock))
		myData.iInfoTexture = cairo_dock_create_texture_from_surface (myData.pInfoSurface);
	cairo_destroy (pCairoContext);
}

void cd_do_find_matching_files (void)
{
	if (myData.sCurrentText->len == 0)
		return ;
	
	// on recupere les N premiers fichiers.
	if (myData.pMatchingFiles != NULL)
	{
		g_strfreev (myData.pMatchingFiles);
		myData.iNbMatchingFiles = 0;
	}
	if (myData.pInfoSurface != NULL)
	{
		cairo_surface_destroy (myData.pInfoSurface);
		myData.pInfoSurface = NULL;
	}
	if (myData.iInfoTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iInfoTexture);
		myData.iInfoTexture = 0;
	}
	
	myData.pMatchingFiles = _cd_do_locate_files (myData.sCurrentText->str);
	
	// si aucun resultat, on l'indique et on quitte.
	if (myData.pMatchingFiles == NULL || myData.pMatchingFiles[0] == NULL)
	{
		_cd_do_make_info (" x ");
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
		return ;
	}
	
	// 1 seul resultat => on est arrive au bout de la recherche.
	if (myData.pMatchingFiles[1] == NULL)
	{
		g_print (">>> found 1 result !\n");
		
		// on l'indique.
		myData.iNbMatchingFiles = 1;
		_cd_do_make_info (" 1 ");
		
		// on complete automatiquement.
		g_string_assign (myData.sCurrentText, myData.pMatchingFiles[0]);
		cd_do_load_pending_caracters ();
		cd_do_launch_appearance_animation ();
		return ;
	}
	g_print ("%s;%s;...\n", myData.pMatchingFiles[0], myData.pMatchingFiles[1]);
	
	// trop de resultats, on l'indique et on abandonne la recherche.
	int i=0;
	for (i = 0; myData.pMatchingFiles[i] != NULL; i ++);
	g_print ("i = %d\n", i);
	myData.iNbMatchingFiles = i;
	if (i == myConfig.iNbResultMax + 1)
	{
		g_strfreev (myData.pMatchingFiles);
		myData.pMatchingFiles = NULL;
		
		gchar *cInfo = g_strdup_printf ("> %d", myConfig.iNbResultMax);
		_cd_do_make_info (cInfo);
		g_free (cInfo);
		
		return ;
	}
	
	// on a suffisamment affine la recherche pour pouvoir afficher nos resultat dans un menu.
	myData.pFileMenu = gtk_menu_new ();
	
	gchar *cInfo = g_strdup_printf (" %d ", myData.iNbMatchingFiles);
	_cd_do_make_info (cInfo);
	g_free (cInfo);
	
	gchar *cPath;
	gchar *cFileName;
	GtkWidget *pMenuItem;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fOrder;
	for (i = 0; myData.pMatchingFiles[i] != NULL; i ++)
	{
		cPath = myData.pMatchingFiles[i];
		g_print (" + %s\n", cPath);

		cairo_dock_fm_get_file_info (cPath, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
		g_free (cName);
		cName = NULL;
		g_free (cURI);
		cURI = NULL;
		
		cFileName = strrchr (cPath, '/');
		if (! cFileName)
			continue;
		cFileName ++;
		if (cIconName != NULL)
		{
			pMenuItem = gtk_image_menu_item_new_with_label (cFileName);
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconName, 32, 32, NULL);
			GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
			g_object_unref (pixbuf);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
			gtk_widget_set_tooltip_text (pMenuItem, cPath);
			g_free (cIconName);
			cIconName = NULL;
		}
		else
		{
			pMenuItem = gtk_menu_item_new_with_label (cFileName);
		}
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (myData.pFileMenu), pMenuItem);
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), cPath);
	}
	
	gtk_widget_show_all (myData.pFileMenu);
	
	g_signal_connect (G_OBJECT (myData.pFileMenu),
		"deactivate",
		G_CALLBACK (_on_delete_menu),
		g_pMainDock);
	
	gtk_menu_popup (GTK_MENU (myData.pFileMenu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());
}


void cd_do_reset_applications_list (void)
{
	g_list_foreach (myData.pApplications, (GFunc) cairo_dock_free_icon, NULL);
	g_list_free (myData.pApplications);
	myData.pApplications = NULL;
	
	GList *m;
	for (m = myData.pMonitorList; m != NULL; m = m->next)
	{
		cairo_dock_fm_remove_monitor_full (m->data, TRUE, NULL);
		g_free (m->data);
	}
	g_list_free (myData.pMonitorList);
	myData.pMonitorList = NULL;
}

static int _compare_appli (Icon *pIcon1, Icon *pIcon2)
{
	if (pIcon1->acCommand == NULL)
		return -1;
	if (pIcon2->acCommand == NULL)
		return 1;
	return strcmp (pIcon1->acCommand, pIcon2->acCommand);
}
static void _cd_do_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	g_print ("la liste des applis a change dans %s!\n", cURI);
	// on reconstruit la liste des applis.
	cd_do_reset_applications_list ();
	_browse_dir ("/usr/share/applications");
	myData.pApplications = g_list_sort (myData.pApplications, (GCompareFunc) _compare_appli);
}
static void _browse_dir (const gchar *cDirPath)
{
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	gchar *cPath, *str, *cCommand, *cIconName;
	Icon *pIcon;
	GKeyFile* pKeyFile;
	const gchar *cFileName;
	GList *pLocalItemList = NULL;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		cPath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		if (g_file_test (cPath, G_FILE_TEST_IS_DIR))
		{
			_browse_dir (cPath);
			if (cairo_dock_fm_add_monitor_full (cPath, TRUE, NULL, (CairoDockFMMonitorCallback) _cd_do_on_file_event, cPath))
			{
				myData.pMonitorList = g_list_prepend (myData.pMonitorList, cPath);
			}
			else
				g_free (cPath);
		}
		else
		{
			pKeyFile = cairo_dock_open_key_file (cPath);
			if (pKeyFile == NULL)
				continue;
			cCommand = g_key_file_get_string (pKeyFile, "Desktop Entry", "Exec", NULL);
			if (cCommand == NULL)
			{
				g_key_file_free (pKeyFile);
				continue;
			}
			cIconName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Icon", NULL);
			if (cIconName == NULL)
			{
				g_key_file_free (pKeyFile);
				continue;
			}
			pIcon = g_new0 (Icon, 1);
			pIcon->acDesktopFileName = cPath;
			pIcon->acFileName = cIconName;
			pIcon->acCommand = cCommand;
			str = strchr (pIcon->acCommand, '%');
			if (str != NULL)
				*str = '\0';
			g_print (" + %s\n", pIcon->acCommand);
			pIcon->cWorkingDirectory = g_key_file_get_string (pKeyFile, "Desktop Entry", "Path", NULL);
			myData.pApplications = g_list_prepend (myData.pApplications, pIcon);
			g_key_file_free (pKeyFile);
		}
	}
	while (1);
	g_dir_close (dir);
}

static int _same_command (Icon *pIcon1, Icon *pIcon2)
{
	return cairo_dock_strings_differ (pIcon1->acCommand, pIcon2->acCommand);
}
void cd_do_find_matching_applications (void)
{
	if (myData.pApplications == NULL)
	{
		/// parse /usr/share/applications ...
		_browse_dir ("/usr/share/applications");
		myData.pApplications = g_list_sort (myData.pApplications, (GCompareFunc) _compare_appli);  // on parcourt tout d'un coup (plutot que par exemple seulement les .desktop correspondant a la 1ere lettre car il y'a les sous-rep a parcourir, donc il faut de toute maniere se farcir la totale; de plus la commande peut differer du nom de .desktop.
	}
	
	if (myData.sCurrentText->len == 0)
		return ;
	
	gboolean bFound = FALSE;
	Icon *pIcon;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	GList *a;
	for (a = myData.pApplications; a != NULL; a = a->next)
	{
		pIcon = a->data;
		if (pIcon->acCommand == NULL || g_strncasecmp (pIcon->acCommand, myData.sCurrentText->str, myData.sCurrentText->len) != 0)
		{
			if (bFound)
				break;
			else
				continue;
		}
		if (g_list_find_custom (myData.pMatchingIcons, pIcon, (GCompareFunc)_same_command) == NULL)
		{
			g_print (" on ajoute %s\n", pIcon->acCommand);
			myData.pMatchingIcons = g_list_append (myData.pMatchingIcons, pIcon);
			if (pIcon->pIconBuffer == NULL && pIcon->acFileName != NULL)
			{
				gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->acFileName);
				pIcon->pIconBuffer = cairo_dock_create_surface_for_icon (cIconPath, pCairoContext, 48., 48);
				g_free (cIconPath);
				pIcon->fWidth = 48.;
				pIcon->fHeight = 48.;
				pIcon->fScale = 1.;
				if (bLoadTexture)
					pIcon->iIconTexture = cairo_dock_create_texture_from_surface (pIcon->pIconBuffer);
			}
		}
	}
	cairo_destroy (pCairoContext);
}



static void _on_activate_filter_item (GtkToggleButton *pButton, gpointer data)
{
	gint iFilterItem = GPOINTER_TO_INT (data);
	if (gtk_toggle_button_get_active (pButton))
		myData.iCurrentFilter |= iFilterItem;
	else
		myData.iCurrentFilter &= (~iFilterItem);
	g_print ("myData.iCurrentFilter  <- %d\n", myData.iCurrentFilter);
	
	// on relance le locate.
	cd_do_find_matching_files ();
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
}
static void _on_activate_match_case (GtkToggleButton *pButton, gpointer data)
{
	myData.bMatchCase = gtk_toggle_button_get_active (pButton);
	cd_do_find_matching_files ();
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
}
static GtkWidget *_cd_do_build_filter_widget (void)
{
	GtkWidget *pFilterWidget = gtk_vbox_new (FALSE, 0);
	GtkWidget *pCheckButton;
	
	pCheckButton = gtk_check_button_new_with_label (D_("Match case"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_match_case), NULL);
	
	pCheckButton = gtk_check_button_new_with_label (D_("Music"));
	//gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pCheckButton), TRUE);
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_MUSIC));
	
	pCheckButton = gtk_check_button_new_with_label (D_("Image"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_IMAGE));
	
	pCheckButton = gtk_check_button_new_with_label (D_("Video"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_VIDEO));
	
	pCheckButton = gtk_check_button_new_with_label (D_("Text"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_TEXT));
	
	pCheckButton = gtk_check_button_new_with_label (D_("Html"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_HTML));
	
	pCheckButton = gtk_check_button_new_with_label (D_("Sources"));
	gtk_box_pack_start (GTK_BOX (pFilterWidget), pCheckButton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (pCheckButton), "clicked", G_CALLBACK (_on_activate_filter_item), GINT_TO_POINTER (DO_TYPE_SOURCE));
	
	return pFilterWidget;
}

void cd_do_show_filter_dialog (void)
{
	if (myData.pFilterDialog != NULL)
		return ;
	
	GtkWidget *pFilterWidget = _cd_do_build_filter_widget ();
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	attr.cText = D_ ("Narrow your search");
	attr.cImageFilePath = MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE;
	attr.pInteractiveWidget = pFilterWidget;
	Icon *pIcon = cairo_dock_get_dialogless_icon ();
	myData.pFilterDialog = cairo_dock_build_dialog (&attr, pIcon, pIcon ? CAIRO_CONTAINER (g_pMainDock) : NULL);
	
	cairo_dock_dialog_reference (myData.pFilterDialog);
}

void cd_do_hide_filter_dialog (void)
{
	if (myData.pFilterDialog == NULL)
		return ;
	
	if (! cairo_dock_dialog_unreference (myData.pFilterDialog))
		cairo_dock_dialog_unreference (myData.pFilterDialog);
	myData.pFilterDialog = NULL;
}

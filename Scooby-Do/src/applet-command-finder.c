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

static gchar **_cd_do_locate_files (const char *text, gboolean bWithLimit)
{
	gchar *standard_output=NULL, *standard_error=NULL;
	gint exit_status=0;
	GError *erreur = NULL;
	GString *sCommand = g_string_new ("locate");
	if (bWithLimit)
		g_string_append_printf (sCommand, " --limit=%d", myConfig.iNbResultMax+1);
	if (myData.bLocateMatchCase)
		g_string_append (sCommand, " -i");
	if (*text != '/')
		g_string_append (sCommand, " -b");
	
	if (myData.iLocateFilter == DO_TYPE_NONE)
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

static void _cd_do_search_files (gpointer data)
{
	myData.pMatchingFiles = _cd_do_locate_files (myData.cCurrentLocateText, TRUE);  // avec limite.
}
static gboolean _cd_do_update_from_files (gpointer data)
{
	if (myData.iLocateFilter != myData.iCurrentFilter ||
		myData.bLocateMatchCase != myData.bMatchCase ||
		cairo_dock_strings_differ (myData.cCurrentLocateText, myData.sCurrentText->str))  // la situation a change entre le lancement de la tache et la mise a jour.
	{
		if (myData.pMatchingFiles != NULL)  // on bache tout, sans regret.
		{
			g_strfreev (myData.pMatchingFiles);
			myData.pMatchingFiles = NULL;
			myData.iNbMatchingFiles = 0;
		}
		
		if (myData.pMatchingIcons != NULL)  // on a des applis, on quitte.
			return FALSE;
		
		myData.iLocateFilter = myData.iCurrentFilter;
		myData.bLocateMatchCase = myData.bMatchCase;
		g_free (myData.cCurrentLocateText);
		myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		cairo_dock_launch_task (myData.pLocateTask);
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
	
	// si aucun resultat, on l'indique et on quitte.
	if (myData.pMatchingFiles == NULL || myData.pMatchingFiles[0] == NULL)
	{
		_cd_do_make_info (D_("no result"));
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
		return FALSE;
	}
	
	// 1 seul resultat => on est arrive au bout de la recherche.
	if (myData.pMatchingFiles[1] == NULL)
	{
		g_print (">>> found 1 result !\n");
		
		// on l'indique.
		/*myData.iNbMatchingFiles = 1;
		_cd_do_make_info (D_("found!"));
		
		// on complete automatiquement.
		g_string_assign (myData.sCurrentText, myData.pMatchingFiles[0]);
		cd_do_load_pending_caracters ();
		cd_do_launch_appearance_animation ();
		return FALSE;*/
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
		
		gchar *cInfo = g_strdup_printf ("> %d %s", myConfig.iNbResultMax, D_("results"));
		_cd_do_make_info (cInfo);
		g_free (cInfo);
		
		return FALSE;
	}
	
	// on a suffisamment affine la recherche pour pouvoir afficher nos resultat dans un menu.
	myData.pFileMenu = gtk_menu_new ();
	
	gchar *cInfo = g_strdup_printf (" %d %s", myData.iNbMatchingFiles, D_("results"));
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
			g_free (cIconName);
			cIconName = NULL;
		}
		else
		{
			pMenuItem = gtk_menu_item_new_with_label (cFileName);
		}
		gtk_widget_set_tooltip_text (pMenuItem, cPath);
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (myData.pFileMenu), pMenuItem);
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), cPath);
	}
	
	/// completer avec les actions :
	/// mail, open folder, copy adress, copy, move, ...
	
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
	cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	return FALSE;
}
void cd_do_find_matching_files (void)
{
	if (myData.sCurrentText->len == 0)
		return ;
	
	if (myData.pLocateTask== NULL)
	{
		myData.pLocateTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) _cd_do_search_files,
			(CairoDockUpdateSyncFunc) _cd_do_update_from_files,
			NULL);
	}
	if (! cairo_dock_task_is_running (myData.pLocateTask))  // sinon, on la laisse se finir, et lorsqu'elle aura finit, on la relancera avec le nouveau texte.
	{
		if (myData.pMatchingFiles != NULL)
		{
			g_strfreev (myData.pMatchingFiles);
			myData.pMatchingFiles = NULL;
			myData.iNbMatchingFiles = 0;
		}
		myData.iLocateFilter = myData.iCurrentFilter;
		myData.bLocateMatchCase = myData.bMatchCase;
		g_free (myData.cCurrentLocateText);
		myData.cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		
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
		_cd_do_make_info (D_("Searching..."));
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
		
		cairo_dock_launch_task (myData.pLocateTask);
	}
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

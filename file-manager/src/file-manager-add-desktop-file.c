/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-struct.h"
#include "file-manager-add-desktop-file.h"

extern FileManagerGetFileInfoFunc file_manager_get_file_info;

extern FileManagerSortType my_fm_iSortType;


gchar * file_manager_add_desktop_file_from_uri (gchar *cURI, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur)
{
	g_print ("%s (%s)\n", __func__, cURI);
	GError *tmp_erreur = NULL;
	gchar *cNewDesktopFileName = NULL;
	
	//\___________________ On ouvre le patron.
	gchar *cDesktopFileTemplate = cairo_dock_get_launcher_template_conf_file ();
	
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cDesktopFileTemplate, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &tmp_erreur);
	g_free (cDesktopFileTemplate);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	//\___________________ On renseigne ce qu'on peut.
	g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", fOrder);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cDockName);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Base URI", cURI);
	
	
	//\___________________ On renseigne les champs propres au type mime.
	gchar *cIconName = NULL, *cName = NULL, *cRealURI = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fUnusedOrder;
	file_manager_get_file_info (cURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fUnusedOrder, my_fm_iSortType);
	g_print (" -> cIconName : %s; bIsDirectory : %d; iVolumeID : %d\n", cIconName, bIsDirectory, iVolumeID);
	
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Name", cName);
	g_free (cName);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", cRealURI);
	g_free (cRealURI);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Icon", (cIconName != NULL ? cIconName : ""));
	g_free (cIconName);
	
	
	if (bIsDirectory)
	{
		/*GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Do you want to monitor the content of the directory ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);*/
		int answer = cairo_dock_ask_general_question_and_wait ("Do you want to monitor the content of the directory ?");
		if (answer != GTK_RESPONSE_YES)
			bIsDirectory = FALSE;
	}
	g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is container", bIsDirectory);
	g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is mounting point", (iVolumeID > 0));
	
	
	//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
	cNewDesktopFileName = cairo_dock_generate_desktop_filename ("file-launcher.desktop", g_cCurrentLaunchersPath);
	
	//\___________________ On ecrit tout.
	gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cNewDesktopFileName);
	cairo_dock_write_keys_to_file (pKeyFile, cNewDesktopFilePath);
	g_free (cNewDesktopFilePath);
	g_key_file_free (pKeyFile);
	
	return cNewDesktopFileName;
}


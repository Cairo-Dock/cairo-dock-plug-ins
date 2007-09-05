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
extern FileManagerListDirectoryFunc file_manager_list_directory;
extern FileManagerLaunchUriFunc file_manager_launch_uri;
extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;
extern FileManagerMountFunc file_manager_mount;
extern FileManagerUnmountFunc file_manager_unmount;

gchar * file_manager_add_desktop_file_from_uri (gchar *cURI, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur)
{
	g_print ("%s (%s)\n", __func__, cURI);
	GError *tmp_erreur = NULL;
	gchar *cNewDesktopFileName = NULL;
	
	//\___________________ On ouvre le patron.
	gchar *cDesktopFileTemplate = cairo_dock_get_template_path ("launcher");
	
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
	g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is URI", TRUE);
	
	
	//\___________________ On renseigne les champs propres au type mime.
	gchar *cIconName = NULL, *cName = NULL, *cRealURI = NULL;
	gboolean bIsDirectory, bIsMountPoint;
	file_manager_get_file_info (cURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &bIsMountPoint);
	g_print (" -> cIconName : %s; bIsDirectory : %d; bIsMountPoint : %d\n", cIconName, bIsDirectory, bIsMountPoint);
	
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Name", cName);
	g_free (cName);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", cRealURI);
	g_free (cRealURI);
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Icon", (cIconName != NULL ? cIconName : ""));
	g_free (cIconName);
	
	
	if (bIsDirectory)
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Do you want to monitor this directory ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer != GTK_RESPONSE_YES)
			bIsDirectory = FALSE;
	}
	g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is container", bIsDirectory);
	g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is mounting point", bIsMountPoint);
	
	
	//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
	cNewDesktopFileName = cairo_dock_generate_desktop_filename (g_cCurrentThemePath);
	
	//\___________________ On ecrit tout.
	gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, cNewDesktopFileName);
	cairo_dock_write_keys_to_file (pKeyFile, cNewDesktopFilePath);
	g_free (cNewDesktopFilePath);
	g_key_file_free (pKeyFile);
	
	return cNewDesktopFileName;
}


void file_manager_launch_icon (Icon *icon)
{
	file_manager_launch_uri (icon->acCommand);
}


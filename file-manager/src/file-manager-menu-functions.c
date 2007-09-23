/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib/gstdio.h>

#include "file-manager-struct.h"
#include "file-manager-add-desktop-file.h"
#include "file-manager-load-directory.h"
#include "file-manager-menu-functions.h"

extern FileManagerLaunchUriFunc file_manager_launch_uri;
extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;
extern FileManagerMountFunc file_manager_mount;
extern FileManagerUnmountFunc file_manager_unmount;
extern FileManagerDeleteFileFunc file_manager_delete_file;
extern FileManagerRenameFileFunc file_manager_rename_file;
extern FileManagerMoveFileFunc file_manager_move_file;
extern FileManagerFilePropertiesFunc file_manager_get_file_properties;


void file_manager_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the truc applet made by Me (me@myadress.zglub) for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


static void file_manager_mount_unmount (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	gboolean bIsMounted = FALSE;
	gchar *cActivationURI = file_manager_is_mounting_point (icon->acCommand, &bIsMounted);
	g_print ("  cActivationURI : %s; bIsMounted : %d\n", cActivationURI, bIsMounted);
	g_free (cActivationURI);
	
	if (! bIsMounted)
	{
		cActivationURI = file_manager_mount (icon->iVolumeID);
		g_print ("apres montage : cActivationURI : %s\n", cActivationURI);
		g_free (cActivationURI);
	}
	else
		file_manager_unmount (icon->acCommand);
	
	cairo_dock_remove_one_icon_from_dock (pDock, icon);
	
	Icon *pNewIcon = file_manager_create_icon_from_URI (icon->cBaseURI, pDock);
	cairo_dock_insert_icon_in_dock (pNewIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
	
	cairo_dock_free_icon (icon);
}

static void file_manager_delete (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	gchar *question = g_strdup_printf ("You're about to delete this file (%s) from your hard-disk. Sure ?", icon->acCommand);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		file_manager_delete_file (icon->acCommand);
	}
}

static void file_manager_rename (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK_CANCEL,
		"Rename to :");
	GtkWidget *pEntry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (pEntry), icon->acName);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), pEntry);
	gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	if (answer == GTK_RESPONSE_OK)
	{
		file_manager_rename_file (icon->acCommand, gtk_entry_get_text (GTK_ENTRY (pEntry)));
	}
	gtk_widget_destroy (dialog);
}

static void file_manager_properties (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	//g_print ("%s (%s)\n", __func__, icon->acName);
	
	guint64 iSize = 0;
	time_t iLastModificationTime = 0;
	gchar *cMimeType = NULL;
	int iUID=0, iGID=0, iPermissionsMask=0;
	file_manager_get_file_properties (icon->acCommand, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask);
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK_CANCEL,
		"Properties :");
	
	GString *sInfo = g_string_new ("");
	g_string_printf (sInfo, "<b>%s</b>", icon->acName);
	GtkWidget *pFrame = gtk_frame_new (sInfo->str);
	gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), pFrame);
	
	GtkWidget *pVBox = gtk_vbox_new (FALSE, 3);
	gtk_container_add (GTK_CONTAINER (pFrame), pVBox);
	
	GtkWidget *pLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
	g_string_printf (sInfo, "<u>Size</u> : %d bytes", iSize);
	gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
	gtk_container_add (GTK_CONTAINER (pVBox), pLabel);
	
	pLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
	struct tm epoch_tm;
	localtime_r (&iLastModificationTime, &epoch_tm);  // et non pas gmtime_r.
	gchar *cTimeChain = g_new0 (gchar, 100);
	strftime (cTimeChain, 100, "%T", &epoch_tm);
	g_string_printf (sInfo, "<u>Last Modification</u> : %s", cTimeChain);
	g_free (cTimeChain);
	gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
	gtk_container_add (GTK_CONTAINER (pVBox), pLabel);
	
	gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	g_string_free (sInfo, TRUE);
	g_free (cMimeType);
}

static void file_manager_remove_from_dock (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	gchar *question = g_strdup_printf ("You're about to remove this icon (%s) from the dock (it will not alter the files on your hard-disk). Sure ?", icon->acName);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		if (icon->acDesktopFileName != NULL)
		{
			gchar *icon_path = g_strdup_printf ("%s/%s", g_cCurrentThemePath, icon->acDesktopFileName);
			g_remove (icon_path);
			g_free (icon_path);
		}
	}
	icon->fPersonnalScale = 1.0;
	if (pDock->iSidShrinkDown == 0)
		pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
	
	cairo_dock_mark_theme_as_modified (TRUE);
}


gboolean file_manager_notification_build_menu (gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	GtkWidget *menu = data[2];
	
	GtkWidget *menu_item;
	
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
	{
		if (icon->bIsMountingPoint)
		{
			gboolean bIsMounted = FALSE;
			gchar *cActivationURI = file_manager_is_mounting_point (icon->acCommand, &bIsMounted);
			g_print ("  cActivationURI : %s; bIsMounted : %d\n", cActivationURI, bIsMounted);
			g_free (cActivationURI);
			
			menu_item = gtk_menu_item_new_with_label (bIsMounted ? "Unmount" : "Mount");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(file_manager_mount_unmount), data);
		}
		else
		{
			menu_item = gtk_menu_item_new_with_label ("Delete this file");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(file_manager_delete), data);
			
			menu_item = gtk_menu_item_new_with_label ("Rename this file");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(file_manager_rename), data);
			
			menu_item = gtk_menu_item_new_with_label ("Properties");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(file_manager_properties), data);
			
			menu_item = gtk_menu_item_new_with_label ("Remove from dock");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(file_manager_remove_from_dock), data);
		}
		return (icon->acDesktopFileName != NULL ? CAIRO_DOCK_LET_PASS_NOTIFICATION : CAIRO_DOCK_INTERCEPT_NOTIFICATION);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean file_manager_notification_drop_data (gpointer *data)
{
	gchar *cReceivedData = data[0];
	Icon *icon = data[1];
	double fOrder = *((double *) data[2]);
	CairoDock *pDock = data[3];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	
	if (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".desktop"))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL);
	if (pPointingIcon != NULL && pPointingIcon->cBaseURI != NULL)  // on a lache dans un dock qui est un repertoire, on copie donc le fichier dedans.
	{
		g_print (" -> copie de %s dans %s\n", cReceivedData, pPointingIcon->cBaseURI);
		file_manager_move_file (cReceivedData, pPointingIcon->cBaseURI);
	}
	else if (fOrder == CAIRO_DOCK_LAST_ORDER && icon->cBaseURI != NULL && icon->pSubDock != NULL)  // on a lache sur une icone qui est un repertoire, on copie donc le fichier dedans.
	{
		g_print (" -> copie de %s dans %s\n", cReceivedData, icon->cBaseURI);
		file_manager_move_file (cReceivedData, icon->cBaseURI);
	}
	else  // on a lache dans un dock de conteneurs, on y rajoute un .desktop.
	{
		GError *erreur = NULL;
		const gchar *cDockName = cairo_dock_search_dock_name (pDock);
		gchar *cNewDesktopFileName = file_manager_add_desktop_file_from_uri (cReceivedData, cDockName, fOrder, pDock, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
		}
		else if (cNewDesktopFileName != NULL)
		{
			cairo_dock_mark_theme_as_modified (TRUE);
			
			cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock);
			Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
			g_free (cNewDesktopFileName);
			cairo_destroy (pCairoContext);
			
			if (pNewIcon != NULL)
				cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
			
			if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
				pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean file_manager_notification_click_icon (gpointer *data)
{
	Icon *icon = data[0];
	
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
	{
		g_print ("%s ()\n", __func__);
		
		gboolean bIsMounted = TRUE;
		
		if (icon->iVolumeID > 0 && ! bIsMounted)
		{
			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (NULL),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"Do you want to mount this point ?");
			int answer = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			if (answer != GTK_RESPONSE_YES)
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			
			gchar *cActivatedURI = file_manager_mount (icon->iVolumeID);
			g_print (" cActivatedURI : %s\n", cActivatedURI);
			if (cActivatedURI != NULL)
				file_manager_launch_uri (cActivatedURI);
			g_free (cActivatedURI);
		}
		else
			file_manager_launch_uri (icon->acCommand);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

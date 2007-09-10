/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <time.h>
#include <glib/gstdio.h>

#include "file-manager-struct.h"
#include "file-manager-menu-functions.h"

extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;
extern FileManagerDeleteFileFunc file_manager_delete_file;
extern FileManagerRenameFileFunc file_manager_rename_file;
extern FileManagerFilePropertiesFunc file_manager_get_file_properties;


void file_manager_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	GtkWidget *pMessageDialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the truc applet made by Me (me@myadress.zglub) for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


static void file_manager_mount_unmount (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	
}

static void file_manager_delete (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
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
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK_CANCEL,
		"Rename to :");
	GtkWidget *pEntry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (pEntry), icon->acName);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), pEntry);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	if (answer == GTK_RESPONSE_YES)
	{
		file_manager_rename_file (icon->acCommand, gtk_entry_get_text (GTK_ENTRY (pEntry)));
	}
	gtk_widget_destroy (dialog);
}

static void file_manager_properties (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	g_print ("%s (%s)\n", __func__, icon->acName);
	
	guint64 iSize = 0;
	time_t iLastModificationTime = 0;
	gchar *cMimeType = NULL;
	int iUID=0, iGID=0, iPermissionsMask=0;
	file_manager_get_file_properties (icon->acCommand, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask);
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK_CANCEL,
		"Rename to :");
	GtkWidget *pEntry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (pEntry), icon->acName);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), pEntry);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	g_free (cMimeType);
}

static void file_manager_remove_from_dock (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
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
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	GtkWidget *menu = data[2];
	
	GtkWidget *menu_item;
	
	if (icon->cBaseURI != NULL)
	{
		if (icon->bIsMountingPoint)
		{
			gboolean bIsMounted = FALSE;
			gchar *cMountPointID = file_manager_is_mounting_point (icon->cBaseURI, &bIsMounted);
			g_free (cMountPointID);
			
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

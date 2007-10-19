/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>

#include "dustbin-menu-functions.h"

extern gchar **my_dustbin_cTrashDirectoryList;
extern Icon *my_dustbin_pIcon;
extern CairoDock *my_dustbin_pDock;
extern GtkWidget *my_dustbin_pMenu;
extern gchar *my_dustbin_cBrowser;


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	gchar *question;
	if (cDirectory != NULL)
		question = g_strdup_printf ("You're about to delete all files in %s. Sure ?", cDirectory);
	else
		question = g_strdup_printf ("You're about to delete all files all dustbins. Sure ?");
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		GString *sCommand = g_string_new ("rm -rf ");
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, "%s/*", cDirectory);
		}
		else
		{
			int i = 0;
			while (my_dustbin_cTrashDirectoryList[i] != NULL)
			{
				g_string_append_printf (sCommand, "%s ", my_dustbin_cTrashDirectoryList[i]);
				i ++;
			}
		}
		//g_print (">>> %s\n", sCommand->str);
		GError *erreur = NULL;
		g_spawn_command_line_async (sCommand->str, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
			g_error_free (erreur);
		}
		g_string_free (sCommand, TRUE);
	}
}


void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	GString *sCommand = g_string_new (my_dustbin_cBrowser);
	if (cDirectory != NULL)
	{
		g_string_append_printf (sCommand, " %s", cDirectory);
	}
	else
	{
		int i = 0;
		while (my_dustbin_cTrashDirectoryList[i] != NULL)
		{
			g_string_append_printf (sCommand, " %s", my_dustbin_cTrashDirectoryList[i]);
			i ++;
		}
	}
	//g_print (">>> %s\n", sCommand->str);
	GError *erreur = NULL;
	g_spawn_command_line_async (sCommand->str, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
		g_error_free (erreur);
		gchar *cTipMessage = g_strdup_printf ("An problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", my_dustbin_cBrowser);
		cairo_dock_show_temporary_dialog (cTipMessage, my_dustbin_pIcon, my_dustbin_pDock, 5000);
		g_free (cTipMessage);
	}
	g_string_free (sCommand, TRUE);
}



void cd_dustbin_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the dustbin applet made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}



gboolean cd_dustbin_notification_click_icon (gpointer *data)
{
	//g_print ("%s ()\n", __func__);
	if (data[0] == my_dustbin_pIcon)
	{
		g_print ("_Note_ : You can manage many Trash directories with this applet.\n Right click on its icon to see which Trash directories are already being monitored.\n");
		
		cd_dustbin_show_trash (NULL, "trash:/");  // my_dustbin_cTrashDirectoryList[0]
		
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_dustbin_notification_build_menu (gpointer *data)
{
	if (data[0] == my_dustbin_pIcon)
	{
		GtkWidget *menu = data[2];
		
		GtkWidget *menu_item;
		menu_item = gtk_menu_item_new_with_label ("Dustbin");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
		
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), my_dustbin_pMenu);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

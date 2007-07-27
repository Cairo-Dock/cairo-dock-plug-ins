
#include <stdlib.h>

#include "dustbin-menu-functions.h"


extern gchar **my_dustbin_cTrashDirectoryList;


void dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory)
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
		g_print (">>> %s\n", sCommand->str);
		system (sCommand->str);
		g_string_free (sCommand, TRUE);
	}
}


void dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	GString *sCommand = g_string_new (g_cDefaultFileBrowser);
	if (cDirectory != NULL)
	{
		g_string_append_printf (sCommand, " %s", cDirectory);
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
	g_print (">>> %s\n", sCommand->str);
	system (sCommand->str);
	g_string_free (sCommand, TRUE);
}



void dustbin_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the dustbin applet made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


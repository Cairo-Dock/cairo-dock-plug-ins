/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-clipboard.h"


static void _cd_clipper_launch_action (GtkMenuItem *pMenuItem, CDClipperCommand *pCommand)
{
	g_print ("%s (%s)\n", __func__, pCommand->cDescription);
	gchar *cText = NULL;
	if (myData.pItems != NULL)
		cText = myData.pItems->data;
	g_return_if_fail (cText != NULL);
	
	gchar *cCommand = g_strdup_printf (pCommand->cFormat, cText);
	cd_message (cCommand);
	system (cCommand);
	g_free (cCommand);
}
static void _on_text_received (GtkClipboard *pClipBoard, const gchar *text, gpointer user_data)
{
	g_print ("%s (%s)\n", __func__, text);
	if (text == NULL)
		return ;
	
	gchar *cItem;
	GList *pElement;
	for (pElement = myData.pItems; pElement != NULL; pElement = pElement->next)
	{
		cItem = pElement->data;
		if (strcmp (cItem, text) == 0)
		{
			break ;
		}
	}
	if (pElement != NULL)
	{
		cd_debug ("Clipper : %s repasse en tete", text);
		myData.pItems = g_list_remove_link (myData.pItems, pElement);
		myData.pItems = g_list_concat (pElement, myData.pItems);
	}
	else
	{
		if (myData.iNbItems == myConfig.iNbItems)
		{
			cd_debug ("Clipper : %s remplace le dernier", text);
			pElement = g_list_last (myData.pItems);
			g_return_if_fail (pElement != NULL);
			g_free (pElement->data);
			myData.pItems = g_list_delete_link (myData.pItems, pElement);
		}
		else
		{
			cd_debug ("Clipper : %s est ajoute", text);
			myData.iNbItems ++;
		}
		myData.pItems = g_list_prepend (myData.pItems, g_strdup (text));
	}
	
	if (myConfig.bEnableActions)
	{
		CDClipperAction *pAction;
		for (pElement = myData.pActions; pElement != NULL; pElement = pElement->next)
		{
			pAction = pElement->data;
			//regex = g_regex_new ("^file:.", 0, 0, NULL);
			if (g_regex_match (pAction->pRegex, text, 0, NULL))
			{
				break ;
			}
		}
		if (pElement != NULL)
		{
			pAction = pElement->data;
			GtkWidget *pMenu = gtk_menu_new ();
			GtkWidget *pMenuItem;
			CDClipperCommand *pCommand;
			for (pElement = pAction->pCommands; pElement != NULL; pElement = pElement->next)
			{
				pCommand = pElement->data;
				CD_APPLET_ADD_IN_MENU_WITH_DATA (pCommand->cDescription, _cd_clipper_launch_action, pMenu, pCommand);
			}
			gtk_widget_show_all (pMenu);
			gtk_menu_popup (GTK_MENU (pMenu),
				NULL,
				NULL,
				NULL,
				NULL,
				1,
				gtk_get_current_event_time ());
		}
	}
}
void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data)
{
	g_print ("%s ()\n", __func__);
	gtk_clipboard_request_text (pClipBoard, (GtkClipboardTextReceivedFunc) _on_text_received, user_data);
}


GList *cd_clipper_build_expressions (void)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "clipper-kde.conf");
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Clipper : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	
	g_free (cConfFilePath);
}

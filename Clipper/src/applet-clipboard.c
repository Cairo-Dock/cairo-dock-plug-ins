/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-clipboard.h"

const gchar *cEnvName[4] = {NULL, "gnome", "kde", "xfce"};

void _on_text_received (GtkClipboard *pClipBoard, const gchar *text, gpointer user_data)
{
	g_print ("%s (%s)\n", __func__, text);
	if (text == NULL)
		return ;
	
	//\________________ On verifie que le texte est non vide.
	gboolean bTextEmpty = TRUE;
	int i = 0;
	while (text[i] != '\0')
	{
		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n')
		{
			bTextEmpty = FALSE;
			break ;
		}
		i ++;
	}
	if (bTextEmpty)
	{
		cd_message ("blank text, will be ignored");
		return ;
	}
	
	//\________________ On recherche l'existence du texte dans les precedents items.
	gboolean bSameItem = FALSE;
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
	
	//\________________ On insere ou on deplace le texte.
	if (pElement != NULL)
	{
		cd_debug ("Clipper : %s repasse en tete", text);
		if (pElement->prev == NULL)  // c'est le 1er.
			bSameItem = TRUE;
		else
		{
			myData.pItems = g_list_remove_link (myData.pItems, pElement);
			myData.pItems = g_list_concat (pElement, myData.pItems);
		}
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
	
	//\________________ On leve le menu des actions correspondantes.
	if (myConfig.bEnableActions && (! bSameItem || myConfig.bReplayAction) && ! myData.bActionBlocked)
	{
		if (myData.pActions == NULL && ! myData.bActionsLoaded)
		{
			myData.bActionsLoaded = TRUE;
			gchar *cConfFilePath = g_strdup_printf ("%s/Clipper-actions-%s.conf", g_cCairoDockDataDir, cEnvName[g_iDesktopEnv]);
			if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
			{
				gchar *cDefaultConfFilePath = g_strdup_printf ("%s/Clipper-actions-%s.conf", MY_APPLET_SHARE_DATA_DIR, 
				cEnvName[g_iDesktopEnv]);
				gchar *cCommand = g_strdup_printf ("cp '%s' '%s'", cDefaultConfFilePath, cConfFilePath);
				cd_message (cCommand);
				system (cCommand);
				g_free (cCommand);
				g_free (cDefaultConfFilePath);
			}
			myData.pActions = cd_clipper_load_actions (cConfFilePath);
			g_free (cConfFilePath);
		}
		CDClipperAction *pAction;
		for (pElement = myData.pActions; pElement != NULL; pElement = pElement->next)
		{
			pAction = pElement->data;
			g_print ("  %s\n", pAction->cDescription);
			if (g_regex_match (pAction->pRegex, text, 0, NULL))
				break ;
		}
		if (pElement != NULL)
		{
			g_print ("  trouve !\n");
			pAction = pElement->data;
			
			GtkWidget *pMenu = cd_clipper_build_action_menu (pAction);
			
			cd_clipper_show_menu (pMenu, 0);
		}
	}
	myData.bActionBlocked = FALSE;
}
void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data)
{
	g_print ("%s ()\n", __func__);
	gtk_clipboard_request_text (pClipBoard, (GtkClipboardTextReceivedFunc) _on_text_received, user_data);
}



GList *cd_clipper_load_actions (const gchar *cConfFilePath)
{
	g_print ("%s (%s)\n", __func__, cConfFilePath);
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Clipper : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	GList *pActionsList = NULL;
	gchar *cGroupName, *cExpression;
	GString *sActionGroupName = g_string_new ("");
	GString *sCommandGroupName = g_string_new ("");
	CDClipperAction *pAction;
	CDClipperCommand *pCommand;
	int i = 0, j;
        while (1)
	{
		g_string_printf (sActionGroupName, "Action_%d", i);
		if (! g_key_file_has_group (pKeyFile, sActionGroupName->str))
			break ;
		
		pAction = g_new0 (CDClipperAction, 1);
		pAction->cDescription = g_key_file_get_locale_string (pKeyFile,
			sActionGroupName->str,
			"Description",
			NULL,
			NULL);
		cExpression = g_key_file_get_string (pKeyFile,
			sActionGroupName->str,
			"Regexp",
			&erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			cd_clipper_free_action (pAction);
			i ++;
			continue;
		}
		pAction->pRegex = g_regex_new (cExpression, 0, 0, NULL);
		g_free (cExpression);
		pActionsList = g_list_prepend (pActionsList, pAction);
		
		j = 0;
		while (1)
		{
			g_string_printf (sCommandGroupName, "%s/Command_%d", sActionGroupName->str, j);
			if (! g_key_file_has_group (pKeyFile, sCommandGroupName->str))
				break ;
			
			pCommand = g_new0 (CDClipperCommand, 1);
			pCommand->cFormat = g_key_file_get_string (pKeyFile,
				sCommandGroupName->str,
				"Commandline",
				&erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				cd_clipper_free_command (pCommand);
				j ++;
				continue;
			}
			pCommand->cDescription = g_key_file_get_locale_string (pKeyFile,
				sCommandGroupName->str,
				"Description",
				NULL,
				NULL);
			pCommand->cIconFileName = g_key_file_get_locale_string (pKeyFile,
				sCommandGroupName->str,
				"Icon",
				NULL,
				NULL);
			pAction->pCommands = g_list_prepend (pAction->pCommands, pCommand);
			j ++;
		}
		i++;
	}
	g_string_free (sCommandGroupName, TRUE);
	g_string_free (sActionGroupName, TRUE);
	g_key_file_free (pKeyFile);
	return pActionsList;
}

void cd_clipper_free_command (CDClipperCommand *pCommand)
{
	if (pCommand == NULL)
		return ;
	g_free (pCommand->cDescription);
	g_free (pCommand->cFormat);
	g_free (pCommand->cIconFileName);
	g_free (pCommand);
}

void cd_clipper_free_action (CDClipperAction *pAction)
{
	if (pAction == NULL)
		return ;
	g_free (pAction->cDescription);
	g_regex_unref (pAction->pRegex);
	g_list_foreach (pAction->pCommands, (GFunc) cd_clipper_free_command, NULL);
	g_list_free (pAction->pCommands);
}




static gboolean _cd_clipper_auto_destroy_action_menu (GtkWidget *pMenu)
{
	if (pMenu == myData.pActionMenu)
	{
		g_print ("auto-destruction\n");
		gtk_widget_destroy (myData.pActionMenu);  // n'appellera pas le 'delete_menu'
		myData.pActionMenu = NULL;
	}
	return FALSE;
}
static void _on_delete_action_menu (GtkMenuShell *menu, CairoDock *pDock)
{
	if (menu == myData.pActionMenu)
	{
		g_print ("on oublie le menu actuel\n");
		myData.pActionMenu = NULL;
	}
	else
		g_print ("un ancien menu est detruit\n");
}
static void _cd_clipper_launch_action (GtkMenuItem *pMenuItem, CDClipperCommand *pCommand)
{
	g_print ("%s (%s)\n", __func__, pCommand->cDescription);
	gchar *cText = NULL;
	if (myData.pItems != NULL)
		cText = myData.pItems->data;
	g_return_if_fail (cText != NULL);
	
	gchar *cCommand = g_strdup_printf (pCommand->cFormat, cText, cText);
	cd_message (cCommand);
	system (cCommand);
	g_free (cCommand);
}
GtkWidget *cd_clipper_build_action_menu (CDClipperAction *pAction)
{
	GtkWidget *pMenu = gtk_menu_new ();
	g_print ("%s\n", pAction->cDescription);
	
	GtkWidget *pMenuItem;
	GtkWidget *pImage;
	CDClipperCommand *pCommand;
	gchar *cIconFilePath;
	GList *pElement;
	for (pElement = pAction->pCommands; pElement != NULL; pElement = pElement->next)
	{
		pCommand = pElement->data;
		if (pCommand->cIconFileName != NULL)
		{
			g_print (" icone %s\n", pCommand->cIconFileName);
			cIconFilePath = cairo_dock_search_icon_s_path (pCommand->cIconFileName);
		}
		else
		{
			gchar *tmp = pCommand->cFormat;
			while (*tmp != '\0' && *tmp != ' ')
				tmp ++;
			gchar *cIconName = g_strndup (pCommand->cFormat, tmp - pCommand->cFormat);
			g_print (" icone %s\n", cIconName);
			cIconFilePath = cairo_dock_search_icon_s_path (cIconName);
			g_free (cIconName);
		}
		
		pMenuItem = gtk_image_menu_item_new_with_mnemonic (pCommand->cDescription);
		if (cIconFilePath != NULL)
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconFilePath, 24, 24, NULL);
			pImage = gtk_image_new_from_pixbuf (pixbuf);
			g_free (cIconFilePath);
			g_object_unref (pixbuf);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), pImage);
		}
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (_cd_clipper_launch_action), pCommand);
	}
	
	g_signal_connect (G_OBJECT (pMenu),
		"deactivate",
		G_CALLBACK (_on_delete_action_menu),
		NULL);
	if (myData.pActionMenu != NULL)
	{
		g_print ("on fusille l'actuel menu\n");
		gtk_widget_destroy (myData.pActionMenu);
	}
	myData.pActionMenu = pMenu;
	g_timeout_add_seconds (myConfig.iActionMenuDuration, (GSourceFunc) _cd_clipper_auto_destroy_action_menu, (gpointer) pMenu);
	return pMenu;
}

static void _cd_clipper_activate_item (GtkMenuItem *pMenuItem, gchar *cText)
{
	g_print ("%s (%s)\n", __func__, cText);
	
	GtkClipboard *pClipBoard;
	if (myConfig.bPasteInClipboard)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	if (myConfig.bPasteInPrimary)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	
	if (! myConfig.bReplayAction)
	{
		myData.bActionBlocked = TRUE;
	}
}
static void _cd_clipper_add_item_in_menu (gchar *cText, GtkWidget *pMenu)
{
	GtkWidget *pMenuItem;
	CD_APPLET_ADD_IN_MENU_WITH_DATA (cText, _cd_clipper_activate_item, pMenu, cText);
}
GtkWidget *cd_clipper_build_items_menu (void)
{
	GtkWidget *pMenu = gtk_menu_new ();
	
	if (myDock)
	{
		myDock->bMenuVisible = TRUE;
		g_signal_connect (G_OBJECT (pMenu),
			"deactivate",
			G_CALLBACK (cairo_dock_delete_menu),
			myDock);
	}
	
	g_list_foreach (myData.pItems, (GFunc)_cd_clipper_add_item_in_menu, pMenu);
	return pMenu;
}

GtkWidget *cd_clipper_build_persistent_items_menu (void)
{
	GtkWidget *pMenu = gtk_menu_new ();
	
	if (myDock)
	{
		myDock->bMenuVisible = TRUE;
		g_signal_connect (G_OBJECT (pMenu),
			"deactivate",
			G_CALLBACK (cairo_dock_delete_menu),
			myDock);
	}
	
	gchar *cText;
	GtkWidget *pMenuItem;
	int i;
	for (i = 0; myConfig.pPersistentItems[i] != NULL; i ++)
	{
		cText = myConfig.pPersistentItems[i];
		CD_APPLET_ADD_IN_MENU_WITH_DATA (cText, _cd_clipper_activate_item, pMenu, cText);
	}
	
	return pMenu;
}

static void _place_menu (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	g_return_if_fail (myContainer != NULL && myIcon != NULL);
	*x = myContainer->iWindowPositionX + myIcon->fDrawX + myIcon->fWidth * myIcon->fScale/2;
	*y = myContainer->iWindowPositionY + myIcon->fDrawY + myIcon->fHeight * myIcon->fScale/2;
	*push_in = TRUE;  // pour que le menu se redimensionne.
}
void cd_clipper_show_menu (GtkWidget *pMenu, gint iButton)
{
	gtk_widget_show_all (pMenu);
	gtk_menu_popup (GTK_MENU (pMenu),
		NULL,
		NULL,
		((myConfig.bMenuOnMouse || (iButton == 1)) ? NULL : (GtkMenuPositionFunc) _place_menu),
		NULL,
		iButton,
		gtk_get_current_event_time ());
}

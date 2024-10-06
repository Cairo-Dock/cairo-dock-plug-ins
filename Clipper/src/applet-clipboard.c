/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-clipboard.h"

const gchar *cEnvName[4] = {"other", "gnome", "kde", "xfce"};  // "other" allows to handle all the other DE (put your own actions into the corresponding file).

// TODO idea: cairo_dock_cut_string => g_strstrip + cut X first char, then add '...', then the X last char + removed '\n'

static int _cd_clipper_compare_item (CDClipperItem *pItem1, CDClipperItem *pItem2)
{
	if (pItem1->iType < pItem2->iType)  // on insere en 1er les items de la primary.
		return 1;
	else if (pItem1->iType > pItem2->iType)
		return -1;
	else
		return 0;
}

GList *cd_clipper_get_last_item (CDClipperItemType iItemType)
{
	CDClipperItem *pItem = NULL;
	GList *pElement;
	for (pElement = myData.pItems; pElement != NULL; pElement = pElement->next)
	{
		pItem = pElement->data;
		if (pItem->iType == iItemType && (pElement->next == NULL || ((CDClipperItem *)pElement->next->data)->iType != iItemType))
		{
			cd_debug ("%s est le dernier de son type (%d)", pItem->cText, iItemType);
			break ;
		}
	}
	if (pItem != NULL && pItem->iType == iItemType)
		return pElement;
	else
		return NULL;
}
void _on_text_received (GtkClipboard *pClipBoard, const gchar *text, gpointer user_data)
{
	if (text == NULL)
		return ;
	CD_APPLET_ENTER;
	CDClipperItemType iType = GPOINTER_TO_INT (user_data);
	cd_message ("%s (%s, %d)", __func__, text, iType);
	
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
		CD_APPLET_LEAVE();
		//return ;
	}
	
	//\________________ On recherche l'existence du texte dans les precedents items.
	gboolean bSameItem = FALSE;
	CDClipperItem *pItem;
	GList *pElement;
	gboolean bFirstClipboardItem = TRUE;
	for (pElement = myData.pItems; pElement != NULL; pElement = pElement->next)
	{
		pItem = pElement->data;
		if (strcmp (pItem->cText, text) == 0)
		{
			break ;
		}
		if (pItem->iType == CD_CLIPPER_CLIPBOARD)
			bFirstClipboardItem = FALSE;
	}
	
	//\________________ On evite les ajouts incrementaux (lors d'une selection incrementale a la souris).
	if (iType & CD_CLIPPER_PRIMARY && myData.pItems != NULL)
	{
		pItem = myData.pItems->data;
		if (pItem->iType & CD_CLIPPER_PRIMARY)  // le dernier item est aussi une selection souris
		{
			guint len = strlen (pItem->cText);
			if (len < strlen (text) && (strncmp (pItem->cText, text, len) == 0 || strncmp (pItem->cText, text+strlen(text)-len, len) == 0))  // on ne peut pas dire len == strlen (text) - 1 avec l'UTF-8.
			{
				cd_debug ("incremental selection, drop previous one");
				cd_clipper_free_item (pItem);
				myData.pItems = g_list_delete_link (myData.pItems, myData.pItems);
				myData.iNbItems[iType] --;
			}
		}
	}
	
	for (pElement = myData.pItems; pElement != NULL; pElement = pElement->next)
	{
		pItem = pElement->data;
		if (strcmp (pItem->cText, text) == 0)
		{
			break ;
		}
		if (pItem->iType == CD_CLIPPER_CLIPBOARD)
			bFirstClipboardItem = FALSE;
	}
	
	
	//\________________ On insere ou on deplace le texte.
	gboolean bExistingItem;
	if (pElement != NULL)
	{
		bExistingItem = TRUE;
		cd_debug ("Clipper : %s repasse en tete", text);
		if (pElement->prev == NULL || (pItem->iType == CD_CLIPPER_CLIPBOARD && bFirstClipboardItem))  // c'est le 1er de son type.
			bSameItem = TRUE;
		myData.pItems = g_list_delete_link (myData.pItems, pElement);
		myData.iNbItems[pItem->iType] --;
		
		if (pItem->iType != iType && myData.iNbItems[iType] >= myConfig.iNbItems[iType])
		{
			cd_debug ("Clipper : %s remplace le dernier de l'autre selection", text);
			pElement = cd_clipper_get_last_item (iType);
			CD_APPLET_LEAVE_IF_FAIL (pElement != NULL);
			cd_clipper_free_item (pElement->data);
			myData.pItems = g_list_delete_link (myData.pItems, pElement);
			myData.iNbItems[iType] --;
		}
		
		pItem->iType = iType;
	}
	else
	{
		bExistingItem = FALSE;
		cd_debug ("%d items / %d", myData.iNbItems[iType], myConfig.iNbItems[iType]);
		if (myData.iNbItems[iType] >= myConfig.iNbItems[iType])
		{
			cd_debug ("Clipper : %s remplace le dernier", text);
			pElement = cd_clipper_get_last_item (iType);
			CD_APPLET_LEAVE_IF_FAIL (pElement != NULL);
			cd_clipper_free_item (pElement->data);
			myData.pItems = g_list_delete_link (myData.pItems, pElement);
			myData.iNbItems[iType] --;
		}
		else
		{
			cd_debug ("Clipper : %s est ajoute", text);
		}
		pItem = g_new0 (CDClipperItem, 1);
		pItem->iType = iType;
		pItem->cText = g_strdup (text);
		/* g_strstrip removes leading and trailing whitespaces from a string
		 * g_strstrip modifies the string in place (by moving the rest of the
		 * characters forward and cutting the trailing spaces)
		 */
		gchar *cLongText = g_strstrip (g_strdup (text)); // remove extras withespaces first
		pItem->cDisplayedText = cairo_dock_cut_string (cLongText, 50);
		g_free (cLongText);
	}
	myData.pItems = g_list_insert_sorted (myData.pItems, pItem, (GCompareFunc)_cd_clipper_compare_item);
	myData.iNbItems[pItem->iType] ++;
	cd_message ("iNbItems[%d] <- %d", pItem->iType, myData.iNbItems[pItem->iType]);
	
	//\________________ On leve le menu des actions correspondantes.
	if (myConfig.bEnableActions && ! bSameItem && (! bExistingItem || myConfig.bReplayAction) && ! myData.bActionBlocked)
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
				int r = system (cCommand);
				if (r < 0)
					cd_warning ("Not able to launch this command: %s", cCommand);
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
			cd_debug ("  %s", pAction->cDescription);
			if (g_regex_match (pAction->pRegex, text, 0, NULL))
				break ;
		}
		if (pElement != NULL)
		{
			cd_debug ("  trouve !\n");
			pAction = pElement->data;
			
			GtkWidget *pMenu = cd_clipper_build_action_menu (pAction);
			
			cd_clipper_popup_menu (pMenu);
		}
	}
	myData.bActionBlocked = FALSE;
	CD_APPLET_LEAVE();
}
void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s ()", __func__);
	CDClipperItemType iItemType;
	if (! myConfig.bSeparateSelections)
		iItemType = CD_CLIPPER_BOTH;
	else
	{
		GtkClipboard *pClipBoardSelection = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		iItemType = (pClipBoard == pClipBoardSelection ? CD_CLIPPER_CLIPBOARD : CD_CLIPPER_PRIMARY);
	}
	gtk_clipboard_request_text (pClipBoard, (GtkClipboardTextReceivedFunc) _on_text_received, GINT_TO_POINTER (iItemType));
	CD_APPLET_LEAVE();
}



GList *cd_clipper_load_actions (const gchar *cConfFilePath)
{
	cd_message ("%s (%s)", __func__, cConfFilePath);
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Clipper : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	gboolean bEnabled;
	GList *pActionsList = NULL;
	gchar *cExpression;
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
			
			bEnabled = g_key_file_get_boolean (pKeyFile,
				sCommandGroupName->str,
				"Enabled",
				&erreur);
			if (erreur != NULL)
			{
				cd_debug ("pas de cle Enabled, on suppose que cette comande est active");
				g_error_free (erreur);
				erreur = NULL;
				bEnabled = TRUE;
			}
			if (! bEnabled)
			{
				j ++;
				continue;
			}
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


void cd_clipper_free_item (CDClipperItem *pItem)
{
	if (pItem == NULL)
		return ;
	g_free (pItem->cText);
	g_free (pItem->cDisplayedText);
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
	CD_APPLET_ENTER;
	if (pMenu == myData.pActionMenu)
	{
		cd_debug ("auto-destruction\n");
		gtk_widget_destroy (myData.pActionMenu);
		myData.pActionMenu = NULL;
	}
	CD_APPLET_LEAVE (FALSE);
}
static void _cd_clipper_launch_action (GtkMenuItem *pMenuItem, CDClipperCommand *pCommand)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, pCommand->cDescription);
	CDClipperItem *pItem = NULL;
	if (myData.pItems != NULL)
		pItem = myData.pItems->data;
	CD_APPLET_LEAVE_IF_FAIL (pItem != NULL);
	
	gchar *cCommand = g_strdup_printf (pCommand->cFormat, pItem->cText, pItem->cText);
	gchar *cBGCommand = g_strconcat (cCommand, " &", NULL);
	cd_message (cBGCommand);
	int r = system (cBGCommand);
	if (r < 0)
		cd_warning ("Not able to launch this command: %s", cBGCommand);
	g_free (cBGCommand);
	g_free (cCommand);
	CD_APPLET_LEAVE();
}
GtkWidget *cd_clipper_build_action_menu (CDClipperAction *pAction)
{
	cd_message ("%s (%s)", __func__, pAction->cDescription);
	
	if (myData.pActionMenu != NULL)
		gtk_widget_destroy (myData.pActionMenu);
	
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	
	GtkWidget *pMenuItem;
	CDClipperCommand *pCommand;
	const gchar *cImage;
	gchar *str;
	GList *pElement;
	for (pElement = pAction->pCommands; pElement != NULL; pElement = pElement->next)
	{
		pCommand = pElement->data;
		str = NULL;
		if (pCommand->cIconFileName != NULL)
		{
			cImage = pCommand->cIconFileName;
		}
		else
		{
			cImage = pCommand->cFormat;
			str = strchr (pCommand->cFormat, ' ');
			if (str) *str = '\0';
		}
		
		pMenuItem = gldi_menu_item_new_full (pCommand->cDescription, cImage, TRUE, 0);  // TRUE <=> use mnemonic, 0 <=> default size
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (_cd_clipper_launch_action), pCommand);
		gtk_menu_shell_append (GTK_MENU_SHELL (pMenu), pMenuItem);
		
		if (str) *str = ' ';
	}
	
	myData.pActionMenu = pMenu;
	g_object_add_weak_pointer (G_OBJECT (pMenu), (gpointer*)&myData.pActionMenu);  // will nullify 'pActionMenu' as soon as the menu is destroyed.
	
	if (myData.iSidMenuAutoDestroy != 0)
		g_source_remove (myData.iSidMenuAutoDestroy);
	myData.iSidMenuAutoDestroy = g_timeout_add_seconds (myConfig.iActionMenuDuration, (GSourceFunc) _cd_clipper_auto_destroy_action_menu, (gpointer) pMenu);
	return pMenu;
}


static void _cd_clipper_activate_text_in_clipboard (GtkMenuItem *pMenuItem, gchar *cText)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, cText);
	GtkClipboard *pClipBoard;
	
	if (myConfig.bPasteInPrimary)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	if (myConfig.bPasteInClipboard)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	
	if (! myConfig.bReplayAction)
	{
		myData.bActionBlocked = TRUE;
	}
	CD_APPLET_LEAVE();
}
static void _cd_clipper_activate_text_in_selection (GtkMenuItem *pMenuItem, gchar *cText)
{
	CD_APPLET_ENTER;
	cd_message ("%s (%s)", __func__, cText);
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
	CD_APPLET_LEAVE();
}
static void _cd_clipper_activate_item (GtkMenuItem *pMenuItem, CDClipperItem *pItem)
{
	CD_APPLET_ENTER;
	if (pItem->iType == CD_CLIPPER_CLIPBOARD)
	{
		_cd_clipper_activate_text_in_clipboard (pMenuItem, pItem->cText);
	}
	else
	{
		_cd_clipper_activate_text_in_selection (pMenuItem, pItem->cText);
	}
	CD_APPLET_LEAVE();
}
/* Not used
static void _cd_clipper_add_item_in_menu (CDClipperItem *pItem, GtkWidget *pMenu)
{
	CD_APPLET_ADD_IN_MENU_WITH_DATA ((pItem->cDisplayedText != NULL ? pItem->cDisplayedText : pItem->cText), _cd_clipper_activate_item, pMenu, pItem);
}
*/
GtkWidget *cd_clipper_build_items_menu (void)
{
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	
	CDClipperItem *pItem;
	GList *pElement;
	for (pElement = myData.pItems; pElement != NULL; pElement = pElement->next)
	{
		pItem = pElement->data;
		CD_APPLET_ADD_IN_MENU_WITH_DATA ((pItem->cDisplayedText != NULL ? pItem->cDisplayedText : pItem->cText), _cd_clipper_activate_item, pMenu, pItem);
		if (pElement->next != NULL && ((CDClipperItem *)pElement->next->data)->iType != pItem->iType)
		{
			CD_APPLET_ADD_SEPARATOR_IN_MENU (pMenu);
		}
	}
	return pMenu;
}

GtkWidget *cd_clipper_build_persistent_items_menu (void)
{
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	
	gchar *cText;
	int i;
	for (i = 0; myConfig.pPersistentItems[i] != NULL; i ++)
	{
		cText = myConfig.pPersistentItems[i];
		CD_APPLET_ADD_IN_MENU_WITH_DATA (cText, _cd_clipper_activate_text_in_clipboard, pMenu, cText);
	}
	
	return pMenu;
}

void cd_clipper_popup_menu (GtkWidget *pMenu)
{
	gtk_widget_show_all (pMenu);
	if (myConfig.bMenuOnMouse)
	{
		GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
		if (pParams) pParams->pIcon = NULL;
	}
	CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
}

gchar *cd_clipper_concat_items_of_type (CDClipperItemType iType, const gchar *cSeparator)
{
	GString *sText = g_string_new ("");
	CDClipperItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if ((pItem->iType & iType) == 0)
			continue;
		g_string_prepend (sText, pItem->cText); // we prepend the string in order to keep the same order
		if (it->next)
			g_string_prepend (sText, cSeparator);
		// g_string_append_printf (sText, "%s%s", pItem->cText, it->next ? cSeparator : "");
	}
	return g_string_free (sText, FALSE);
}

void cd_clipper_load_items (const gchar *cItems)
{
	CDClipperItem *pItem;
	int iClipperItemType = myConfig.bSeparateSelections ? CD_CLIPPER_CLIPBOARD : CD_CLIPPER_BOTH;
	gchar **cItemList = g_strsplit (cItems, CD_ITEMS_DELIMITER, -1);
	int i;
	for (i = 0; cItemList[i] != NULL; i ++)
	{
		// if we have reduced the number of items to display
		if (i == myConfig.iNbItems[iClipperItemType])
			break;
		pItem = g_new0 (CDClipperItem, 1);
		pItem->iType = iClipperItemType;
		pItem->cText = cItemList[i];
		/* g_strstrip removes leading and trailing whitespaces from a string
		 * g_strstrip modifies the string in place (by moving the rest of the
		 * characters forward and cutting the trailing spaces)
		 */
		gchar *cLongText = g_strstrip (g_strdup (pItem->cText)); // remove extras withespaces first
		pItem->cDisplayedText = cairo_dock_cut_string (cLongText, 50);
		g_free (cLongText);
		myData.pItems = g_list_insert_sorted (myData.pItems, pItem, (GCompareFunc)_cd_clipper_compare_item);
		myData.iNbItems[iClipperItemType] ++;
	}
	g_free (cItemList);
}

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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-rss.h"
#include "applet-notifications.h"


static void _new_url_to_conf (GldiModuleInstance *myApplet, const gchar *cNewURL)
{
	if (g_strstr_len (cNewURL, -1, "http") != NULL)  // On verifie que l'element glisser/copier commence bien par http
	{
		cd_debug ("RSSreader-debug : This seems to be a valid URL -> Let's continue...");
		// on definit la nouvelle URL en conf.
		g_free (myConfig.cUrl);
		myConfig.cUrl = g_strdup (cNewURL);
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING,
			"Configuration",
			"url_rss_feed",
			myConfig.cUrl,
			G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
		
		// on remet a zero les items actuels.
		CD_APPLET_SET_NAME_FOR_MY_ICON (NULL);  // pour mettre a jour le titre par la meme occasion.
		
		g_free (myData.PrevFirstTitle);
		myData.PrevFirstTitle = NULL;
		cd_rssreader_free_item_list (myApplet);
		
		// on recupere le nouveau flux.
		CDRssItem *pItem = g_new0 (CDRssItem, 1);  // on commence au debut de la liste (c'est le titre).
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		pItem->cTitle = g_strdup (D_("Retrieving data..."));
		myData.bInit = FALSE;
		myData.bError = FALSE;
		
		if (myDesklet)
			cd_applet_update_my_icon (myApplet);
		
		cd_rssreader_launch_task (myApplet); // On lance l'upload pour mettre a jour notre applet
	}
	else
	{
		cd_debug ("RSSreader-debug : It doesn't seem to be a valid URL.");	
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("It doesn't seem to be a valid URL."),
			myIcon,
			myContainer,
			3000, // Suffisant 
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);		
	}
}

static void _paste_new_url_to_conf (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	GtkClipboard *pClipBoardSelection = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);	
	gchar *cEntry = gtk_clipboard_wait_for_text (pClipBoardSelection);
	cd_debug ("RSSreader-debug : Paste from clipboard -> \"%s\"", cEntry);	
	_new_url_to_conf (myApplet, cEntry);
	CD_APPLET_LEAVE ();
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_rssreader_show_dialog (myApplet);
CD_APPLET_ON_CLICK_END


static inline void _update_feeds (GldiModuleInstance *myApplet)
{
	myData.bUpdateIsManual = TRUE;
	// on ne met pas de message d'attente pour conserver les items actuels, on prefere afficher un dialogue signalant ou pas une modification.
	if (! gldi_task_is_running (myData.pTask))  // sinon on va bloquer jusqu'a ce que la tache courante se termine, pour la relancer aussitot, ce qui n'a aucun interet.
		cd_rssreader_launch_task (myApplet);
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_update_feeds (myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_debug ("RSSreader-debug : \"%s\" was dropped", CD_APPLET_RECEIVED_DATA);
	_new_url_to_conf (myApplet, CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _start_browser (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.cSpecificWebBrowser != NULL)  // une commande specifique est fournie.
	{
		const gchar * const args[] = {myConfig.cSpecificWebBrowser, myConfig.cUrl, NULL};
		cairo_dock_launch_command_argv_full (args, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	}
	else  // sinon on utilise la commande par defaut.
		cairo_dock_fm_launch_uri (myConfig.cUrl);
}
static void _refresh (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_update_feeds (myApplet);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Paste a new RSS Url (drag and drop)"), GLDI_ICON_NAME_PASTE, _paste_new_url_to_conf, CD_APPLET_MY_MENU);	
	
	if (myConfig.cUrl != NULL) // On ajoute une entrée dans le menu SI il y a une url seulement
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open with your web browser"), GLDI_ICON_NAME_EXECUTE, _start_browser, CD_APPLET_MY_MENU);
		gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Refresh"), D_("middle-click"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_REFRESH, _refresh, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
CD_APPLET_ON_BUILD_MENU_END


static gboolean _redraw_desklet_idle (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_applet_update_my_icon (myApplet);
	myData.iSidRedrawIdle = 0;
	CD_APPLET_LEAVE (FALSE);
	//return FALSE;
}
CD_APPLET_ON_SCROLL_BEGIN
	if (! myDesklet)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	myData.iFirstDisplayedItem += (CD_APPLET_SCROLL_UP ? -1 : 1);
	if (myData.iFirstDisplayedItem < 0)  // on a scrolle trop haut.
	{
		myData.iFirstDisplayedItem = 0;
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	}
	else
	{
		int n = g_list_length (myData.pItemList) - 1;  // la 1ere ligne est le titre.
		if (myData.iFirstDisplayedItem > n - 1)  // on a scrolle trop bas.
		{
			myData.iFirstDisplayedItem = n - 1;
			CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
		}
	}
	if (myData.iSidRedrawIdle == 0)  // on planifie un redessin pour quand la boucle principale sera accessible, de facon a éviter de la surcharger en cas de scroll rapide.
		myData.iSidRedrawIdle = g_idle_add ((GSourceFunc)_redraw_desklet_idle, myApplet);
CD_APPLET_ON_SCROLL_END

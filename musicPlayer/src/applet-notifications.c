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
#include "applet-musicplayer.h"
#include "applet-draw.h"
#include "applet-dbus.h"
#include "3dcover-draw.h"
#include "applet-cover.h"
#include "applet-notifications.h"

static void _show_players_list_dialog (void);


static void _cd_musicplayer_prev (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_PREVIOUS, NULL);
}
static void _cd_musicplayer_pp (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_PLAY_PAUSE, NULL);
}
static void _cd_musicplayer_stop (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_STOP, NULL);
}
static void _cd_musicplayer_next (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_NEXT, NULL);
}
static void _cd_musicplayer_show_from_systray (GtkMenuItem *menu_item, gpointer *data)
{
	gboolean bRaised = FALSE;
	if (myData.pCurrentHandler->raise)
		bRaised = myData.pCurrentHandler->raise ();
	if (! bRaised && myData.pCurrentHandler->pAppInfo)
		gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
}
static void _cd_musicplayer_quit (GtkMenuItem *menu_item, gpointer *data)
{
	gboolean bQuit = FALSE;
	if (myData.pCurrentHandler->quit)
		bQuit = myData.pCurrentHandler->quit ();
	if (! bQuit)
	{ //!!! TODO
/*		gchar *cmd = g_strdup_printf ("killall %s", myData.pCurrentHandler->launch);
		cairo_dock_launch_command (cmd);
		g_free (cmd); */
	}
}
static void _cd_musicplayer_jumpbox (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_JUMPBOX, NULL);
}
static void _cd_musicplayer_shuffle (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_SHUFFLE, NULL);
}
static void _cd_musicplayer_repeat (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandler->control (PLAYER_REPEAT, NULL);
}
static void _cd_musicplayer_rate (GtkMenuItem *menu_item, gpointer *data) {
	/// trouver un moyen esthetique.
}
static void _cd_musicplayer_info (GtkMenuItem *menu_item, gpointer *data)
{
	// the user wants to see this dialogue, it's maybe better to not remove it automatically
	cd_musicplayer_popup_info (0); // 0 = without any timeout
}
static void _cd_musicplayer_launch (GtkMenuItem *menu_item, gpointer *data)
{
	gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
}

static void _cd_musicplayer_choose_player (GtkMenuItem *menu_item, gpointer *data)
{
	CD_APPLET_ENTER;
	_show_players_list_dialog ();
	CD_APPLET_LEAVE ();
}

static void _cd_musicplayer_find_player (GtkMenuItem *menu_item, gpointer *data)
{
	CD_APPLET_ENTER;
	MusicPlayerHandler *pHandler = cd_musicplayer_dbus_find_opened_player ();
	if (pHandler == NULL)
	{
		gldi_dialog_show_temporary_with_icon (D_("Sorry, I couldn't detect any player.\nIf it is running, it is maybe because its version is too old and does not offer such service."),
			myIcon,
			myContainer,
			7000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	else if (pHandler != myData.pCurrentHandler || !strcmp (pHandler->name, "Mpris2"))
	{
		if (myData.pCurrentHandler != NULL)
		{
			cd_musicplayer_stop_current_handler (TRUE);
		}
		
		// get the name of the running player.
		const gchar *cPlayerName;
		if (strcmp (pHandler->name, "Mpris2") == 0)  // generic MPRIS2 handler, use the class.
			cPlayerName = pHandler->appclass;
		else
			cPlayerName = pHandler->name;
		cd_debug ("found %s (%s)", pHandler->name, cPlayerName);
		
		// write it down into our conf file
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING, "Configuration", "current-player", cPlayerName,
			G_TYPE_STRING, "Configuration", "desktop-entry", "",  // reset the desktop filename, we'll get the new one from the "DesktopEntry" property of the new player
			G_TYPE_INVALID);
		g_free (myConfig.cMusicPlayer);
		myConfig.cMusicPlayer = g_strdup (cPlayerName);
		g_free (myConfig.cLastKnownDesktopFile);
		myConfig.cLastKnownDesktopFile = NULL;
		
		// set the handler with this value.
		cd_musicplayer_set_current_handler (myConfig.cMusicPlayer);
	}
	CD_APPLET_LEAVE ();
}


static void _choice_dialog_action (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 1 || iClickedButton == -2)  // click on "cancel", or Escape
		return;
	// get the value in the widget.
	GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pInteractiveWidget));
	const gchar *cPlayerName = gtk_entry_get_text (GTK_ENTRY (pEntry));
	if (cPlayerName == NULL || *cPlayerName == '\0')
		return;
	// write it down into our conf file
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_STRING, "Configuration", "current-player", cPlayerName,
		G_TYPE_STRING, "Configuration", "desktop-entry", "",  // reset the desktop filename, we'll get the new one from the "DesktopEntry" property of the new player
		G_TYPE_INVALID);
	g_free (myConfig.cMusicPlayer);
	myConfig.cMusicPlayer = g_strdup (cPlayerName);
	g_free (myConfig.cLastKnownDesktopFile);
	myConfig.cLastKnownDesktopFile = NULL;
	// set the handler with this value.
	cd_musicplayer_set_current_handler (myConfig.cMusicPlayer);
	// launch it, if it's already running, it's likely to have no effect
	gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
}
static void _show_players_list_dialog (void)
{
	// build a list of the available groups.
	GtkWidget *pComboBox = gtk_combo_box_text_new_with_entry ();
	GList *h;
	MusicPlayerHandler *handler;
	for (h = myData.pHandlers; h != NULL; h = h->next)
	{
		handler = h->data;
		if (handler->cMprisService != NULL)
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pComboBox), handler->name);
	}
	GtkTreeModel *pModel = gtk_combo_box_get_model (GTK_COMBO_BOX (pComboBox));
	if (pModel)
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (pModel), CAIRO_DOCK_MODEL_NAME, GTK_SORT_ASCENDING);
	/// maybe try to set the dialog color (text and bg colors)...

	// detect a running player and set it as the current choice.
	MusicPlayerHandler *pRunningHandler = cd_musicplayer_dbus_find_opened_player ();
	if (pRunningHandler != NULL)
	{
		GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pComboBox));
		if (strcmp (pRunningHandler->name, "Mpris2") == 0)  // generic MPRIS2 handler, use the app class.
			gtk_entry_set_text (GTK_ENTRY (pEntry), pRunningHandler->appclass);
		else
			gtk_entry_set_text (GTK_ENTRY (pEntry), pRunningHandler->name);
	}

	// build the dialog.
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cText = D_("Choose a music player to control");
	attr.cImageFilePath = NULL;
	attr.pInteractiveWidget = pComboBox;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc)_choice_dialog_action;
	attr.pUserData = NULL;
	attr.pFreeDataFunc = (GFreeFunc)NULL;
	const gchar *cButtons[] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;

	gldi_dialog_new (&attr);
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (myData.pCurrentHandler != NULL)
	{
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myData.numberButtons != 0  && myConfig.bOpenglThemes && myDesklet)
		{
			// Actions au clic sur un bouton :
			if (myData.mouseOnButton1)
			{
				if(myData.bIsRunning)
				{
					_cd_musicplayer_pp (NULL, NULL);
				}
				else
				{
					if (myIcon->cClass != NULL)
						gldi_icon_launch_command (myIcon);
					else if (myData.pCurrentHandler->pAppInfo)
						gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
				}
			}
			else if (myData.mouseOnButton2)
				_cd_musicplayer_prev (NULL, NULL);
			else if (myData.mouseOnButton3)
				_cd_musicplayer_next (NULL, NULL);
			else if (myData.mouseOnButton4)
			{
				if(myData.bIsRunning)
				{
					if (myData.pCurrentHandler->iPlayerControls & PLAYER_JUMPBOX)
						_cd_musicplayer_jumpbox (NULL, NULL);
					else if (myIcon->pAppli != NULL)
						gldi_window_show (myIcon->pAppli);
				}
				else if (myData.pCurrentHandler->pAppInfo)
					gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
			}
			else
			{
				if(myData.bIsRunning)
					cd_musicplayer_popup_info (myConfig.iDialogDuration);
				else
				{
					if (myIcon->cClass != NULL)
						gldi_icon_launch_command (myIcon);
					else if (myData.pCurrentHandler->pAppInfo)
						gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
				}
			}
		}
		else  // pas de boutons.
		{
			if(myData.bIsRunning)
			{
				if (myConfig.bPauseOnClick)
				{
					_cd_musicplayer_pp (NULL, NULL);
				}
				else if (myIcon->pAppli != NULL)
				{
					if (myIcon->pAppli == gldi_windows_get_active ())  // la fenetre du lecteur a le focus. en mode desklet ca ne marche pas car il aura pris le focus au moment du clic.
						gldi_window_minimize (myIcon->pAppli);
					else
						gldi_window_show (myIcon->pAppli);
				}
				else
				{
					_cd_musicplayer_show_from_systray (NULL, NULL);
				}
			}
			else
			{
				if (myIcon->cClass != NULL)
					gldi_icon_launch_command (myIcon);
				else if (myData.pCurrentHandler->pAppInfo)
					gldi_app_info_launch (myData.pCurrentHandler->pAppInfo, NULL);
			}
		}
	}
	else
	{
		_show_players_list_dialog ();
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (! myData.bIsRunning)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Find opened player"), GLDI_ICON_NAME_FIND, _cd_musicplayer_find_player, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandler != NULL)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (myData.pCurrentHandler->cDisplayedName?myData.pCurrentHandler->cDisplayedName:myData.pCurrentHandler->name, GLDI_ICON_NAME_MEDIA_PLAY, _cd_musicplayer_launch, CD_APPLET_MY_MENU);
		else
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Choose a player"), GLDI_ICON_NAME_MEDIA_PLAY, _cd_musicplayer_choose_player, CD_APPLET_MY_MENU);
	}
	else
	{
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_PREVIOUS)
		{
			gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Previous"), D_("scroll-up"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_MEDIA_PREVIOUS, _cd_musicplayer_prev, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_PLAY_PAUSE)
		{
			gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Play/Pause"), myConfig.bPauseOnClick ? D_("left-click") : D_("middle-click"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, (myData.iPlayingStatus != PLAYER_PLAYING ? GLDI_ICON_NAME_MEDIA_PLAY : GLDI_ICON_NAME_MEDIA_PAUSE), _cd_musicplayer_pp, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_NEXT)
		{
			gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Next"), D_("scroll-down"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_MEDIA_NEXT, _cd_musicplayer_next, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_STOP)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Stop"), GLDI_ICON_NAME_MEDIA_STOP, _cd_musicplayer_stop, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);  // on n'a jamais zero action, donc on met toujours un separateur.
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Information"), GLDI_ICON_NAME_INFO, _cd_musicplayer_info, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_JUMPBOX)
			CD_APPLET_ADD_IN_MENU (D_("Show JumpBox"), _cd_musicplayer_jumpbox, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_SHUFFLE)
		{
			pMenuItem = gtk_check_menu_item_new_with_label (D_("Shuffle"));
			gboolean bIsShuffle = (myData.pCurrentHandler->get_shuffle_status ? myData.pCurrentHandler->get_shuffle_status() : FALSE);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pMenuItem), bIsShuffle);
			gtk_menu_shell_append  (GTK_MENU_SHELL (CD_APPLET_MY_MENU), pMenuItem);
			g_signal_connect (G_OBJECT (pMenuItem), "toggled", G_CALLBACK (_cd_musicplayer_shuffle), NULL);
		}
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_REPEAT)
		{
			pMenuItem = gtk_check_menu_item_new_with_label (D_("Repeat"));
			gboolean bIsLoop = (myData.pCurrentHandler->get_loop_status ? myData.pCurrentHandler->get_loop_status() : FALSE);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pMenuItem), bIsLoop);
			gtk_menu_shell_append  (GTK_MENU_SHELL (CD_APPLET_MY_MENU), pMenuItem);
			g_signal_connect (G_OBJECT (pMenuItem), "toggled", G_CALLBACK (_cd_musicplayer_repeat), NULL);
		}
		
		if (myData.pCurrentHandler->iPlayerControls & PLAYER_RATE)
			CD_APPLET_ADD_IN_MENU (D_("Rate this song"), _cd_musicplayer_rate, CD_APPLET_MY_MENU);
		
		if (myIcon->pAppli == NULL)  // player in the systray.
		{
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Show"), GLDI_ICON_NAME_FIND, _cd_musicplayer_show_from_systray, CD_APPLET_MY_MENU);
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Quit"), GLDI_ICON_NAME_CLOSE, _cd_musicplayer_quit, CD_APPLET_MY_MENU);  // GLDI_ICON_NAME_QUIT looks too much like the "quit" of the dock.
		}
	}
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.pCurrentHandler != NULL)
	{
		if (myConfig.bPauseOnClick)
		{
			_cd_musicplayer_next (NULL, NULL);
		}
		else
		{
			_cd_musicplayer_pp (NULL, NULL);
		}
	}
	else
	{
		_show_players_list_dialog ();
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" %s --> nouvelle pochette ou chanson !", CD_APPLET_RECEIVED_DATA);
	
	if (myData.pCurrentHandler != NULL)
	{
			gboolean isJpeg = g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpg") 
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPG")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpeg")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPEG");

		if (isJpeg)
		{
			if (myData.cArtist != NULL && myData.cAlbum != NULL/* && myData.pCurrentHandler->cCoverDir != NULL*/)
			{
				cd_debug ("MP - Le fichier est un JPEG");
				gchar *cDirPath = myData.pCurrentHandler->cCoverDir ? g_strdup (myData.pCurrentHandler->cCoverDir) : g_strdup_printf("%s/musicplayer", g_cCairoDockDataDir);
				gchar *cCommand, *cHost = NULL;
				gchar *cFilePath = (*CD_APPLET_RECEIVED_DATA == '/' ? g_strdup (CD_APPLET_RECEIVED_DATA) : g_filename_from_uri (CD_APPLET_RECEIVED_DATA, &cHost, NULL));
				if (cHost != NULL)  // fichier distant, on le telecharge dans le cache du lecteur.
				{
					cd_debug ("MP - Le fichier est distant (sur %s)", cHost);
					cCommand = g_strdup_printf ("wget -O \"%s/%s - %s.jpg\" '%s'",
						cDirPath,
						myData.cArtist,
						myData.cAlbum,
						CD_APPLET_RECEIVED_DATA);
				}
				else  // fichier local, on le copie juste dans le cache du lecteur.
				{
					cd_debug ("MP - Le fichier est local");
					cCommand = g_strdup_printf ("cp \"%s\" \"%s/%s - %s.jpg\"",
						cFilePath,
						cDirPath,
						myData.cArtist,
						myData.cAlbum);
				}
				cd_debug ("MP - on recupere la pochette par : '%s'", cCommand);
				cairo_dock_launch_command (cCommand);
				g_free (cCommand);
				g_free (cFilePath);
				g_free (cHost);
				g_free (cDirPath);

				cd_musicplayer_set_cover_path (NULL);
				cd_musicplayer_update_icon ();
			}
		}
		else
		{
			cd_debug ("MP - on rajoute la chanson a la queue.");
			myData.pCurrentHandler->control (PLAYER_ENQUEUE, CD_APPLET_RECEIVED_DATA);
		}
	}
	else
	{
		_show_players_list_dialog ();
	}
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_SCROLL_BEGIN
	if (myData.pCurrentHandler != NULL)
	{
		if (myConfig.bNextPrevOnScroll)
		{
			if (CD_APPLET_SCROLL_DOWN)
  				_cd_musicplayer_next (NULL, NULL);
			else if (CD_APPLET_SCROLL_UP)
  				_cd_musicplayer_prev (NULL, NULL);
		}
		else if (myData.pCurrentHandler->iPlayerControls & PLAYER_VOLUME)
		{
			if (CD_APPLET_SCROLL_DOWN)
				myData.pCurrentHandler->control (PLAYER_VOLUME, "down");
			else if (CD_APPLET_SCROLL_UP)
  				myData.pCurrentHandler->control (PLAYER_VOLUME, "up");
		}
		else
			cd_warning ("can't control the volume with the player '%s'", myData.pCurrentHandler->name);
	}
	else
	{
		_show_players_list_dialog ();
	}
CD_APPLET_ON_SCROLL_END


#define _update_button_count(on, count) \
	if (on) {\
		if (count < NB_TRANSITION_STEP) {\
			count ++;\
			bNeedsUpdate = TRUE; } }\
	else if (count != 0) {\
		count --;\
		bNeedsUpdate = TRUE; }

CD_APPLET_ON_UPDATE_ICON_BEGIN

	gboolean bNeedsUpdate = FALSE;
	
	if (myData.iCoverTransition > 0)
	{
		myData.iCoverTransition --;
		bNeedsUpdate = TRUE;
	}
	
	_update_button_count (myData.mouseOnButton1, myData.iButton1Count);
	_update_button_count (myData.mouseOnButton2, myData.iButton2Count);
	_update_button_count (myData.mouseOnButton3, myData.iButton3Count);
	_update_button_count (myData.mouseOnButton4, myData.iButton4Count);
	
	if (! bNeedsUpdate)
		CD_APPLET_STOP_UPDATE_ICON;  // quit.
	
	cd_opengl_render_to_texture (myApplet);
	
	if ((myData.iCoverTransition == 0) &&
		(myData.iButton1Count == 0 || myData.iButton1Count == NB_TRANSITION_STEP) &&
		(myData.iButton2Count == 0 || myData.iButton2Count == NB_TRANSITION_STEP) &&
		(myData.iButton3Count == 0 || myData.iButton3Count == NB_TRANSITION_STEP) &&
		(myData.iButton4Count == 0 || myData.iButton4Count == NB_TRANSITION_STEP))
	{
		CD_APPLET_PAUSE_UPDATE_ICON;  // redraw and stop.
	}
	
CD_APPLET_ON_UPDATE_ICON_END


gboolean cd_opengl_test_mouse_over_buttons (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	int iPrevButtonState = myData.iButtonState;
	myData.iButtonState = cd_opengl_check_buttons_state (myApplet);
	
	if (iPrevButtonState != myData.iButtonState)
	{
		*bStartAnimation = TRUE;  // ca c'est pour faire une animation de transition...
	}
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-draw.h"
#include "applet-dbus.h"
#include "3dcover-draw.h"
#include "applet-cover.h"
#include "applet-notifications.h"


static void _cd_musicplayer_prev (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_PREVIOUS, NULL);
}
static void _cd_musicplayer_pp (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_PLAY_PAUSE, NULL);
}
static void _cd_musicplayer_stop (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_STOP, NULL);
}
static void _cd_musicplayer_next (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_NEXT, NULL);
}
static void _cd_musicplayer_jumpbox (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_JUMPBOX, NULL);
}
static void _cd_musicplayer_shuffle (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_SHUFFLE, NULL);
}
static void _cd_musicplayer_repeat (GtkMenuItem *menu_item, gpointer *data) {
	myData.pCurrentHandeler->control (PLAYER_REPEAT, NULL);
}
static void _cd_musicplayer_rate (GtkMenuItem *menu_item, gpointer *data) {
	/// trouver un moyen esthetique.
}
static void _cd_musicplayer_info (GtkMenuItem *menu_item, gpointer *data)
{
	cd_musicplayer_popup_info ();
}

static void _cd_musicplayer_find_player (GtkMenuItem *menu_item, gpointer *data)
{
	MusicPlayerHandeler *pHandler = cd_musicplayer_dbus_find_opened_player ();
	if (pHandler != NULL && pHandler != myData.pCurrentHandeler)
	{
		if (myData.pCurrentHandeler)
		{
			cd_musicplayer_stop_handler ();  // libère tout ce qu'occupe notre ancien handler.
			CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandeler->appclass, FALSE);
		}
		myData.pCurrentHandeler = pHandler;
		cd_musicplayer_launch_handler ();
		CD_APPLET_MANAGE_APPLICATION (myData.pCurrentHandeler->appclass, myConfig.bStealTaskBarIcon);
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myData.numberButtons != 0  && myConfig.bOpenglThemes && myDesklet)
	{
		// Actions au clic sur un bouton :
		if (myData.mouseOnButton1)
		{
			if(myData.bIsRunning)
			{
				_cd_musicplayer_pp (NULL, NULL);
			}
			else if (myData.pCurrentHandeler->launch != NULL)
			{
				cairo_dock_launch_command (myData.pCurrentHandeler->launch);
			}
		}
		else if (myData.mouseOnButton2)
			_cd_musicplayer_prev (NULL, NULL);
		else if (myData.mouseOnButton3)
			_cd_musicplayer_next (NULL, NULL);
		else if (myData.mouseOnButton4)
			_cd_musicplayer_jumpbox (NULL, NULL);
		else
		{
			if(myData.bIsRunning)
				cd_musicplayer_popup_info ();
			else if (myData.pCurrentHandeler->launch != NULL)
				cairo_dock_launch_command (myData.pCurrentHandeler->launch);
		}
	}
	else
	{
		if(myData.bIsRunning)
		{
			if (myConfig.bPauseOnClick)
			{
				_cd_musicplayer_pp (NULL, NULL);
			}
			else
			{
				if (myIcon->Xid == cairo_dock_get_current_active_window ())  // la fenetre du lecteur a le focus. en mode desklet ca ne marche pas car il aura pris le focus au moment du clic.
					cairo_dock_minimize_xwindow (myIcon->Xid);
				else
					cairo_dock_show_xwindow (myIcon->Xid);
			}
		}
		else if (myData.pCurrentHandeler->launch != NULL)
			cairo_dock_launch_command (myData.pCurrentHandeler->launch);
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	if (! myData.bIsRunning)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Find opened player"), GTK_STOCK_FIND, _cd_musicplayer_find_player, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_PLAY_PAUSE)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Play/Pause (left-click)"), (myData.iPlayingStatus != PLAYER_PLAYING ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE), _cd_musicplayer_pp, CD_APPLET_MY_MENU);
	}
	else
	{
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_PREVIOUS)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Previous"), GTK_STOCK_MEDIA_PREVIOUS, _cd_musicplayer_prev, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_PLAY_PAUSE)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Play/Pause (left-click)"), (myData.iPlayingStatus != PLAYER_PLAYING ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE), _cd_musicplayer_pp, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_NEXT)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Next (middle-click)"), GTK_STOCK_MEDIA_NEXT, _cd_musicplayer_next, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_STOP)
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Stop"), GTK_STOCK_MEDIA_STOP, _cd_musicplayer_stop, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_JUMPBOX)
			CD_APPLET_ADD_IN_MENU (D_("Show JumpBox"), _cd_musicplayer_jumpbox, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_SHUFFLE)
			CD_APPLET_ADD_IN_MENU (D_("Toggle Shuffle"), _cd_musicplayer_shuffle, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_REPEAT)
			CD_APPLET_ADD_IN_MENU (D_("Toggle Repeat"), _cd_musicplayer_repeat, CD_APPLET_MY_MENU);
		if (myData.pCurrentHandeler->iPlayerControls & PLAYER_RATE)
			CD_APPLET_ADD_IN_MENU (D_("Rate this song"), _cd_musicplayer_rate, CD_APPLET_MY_MENU);
	}
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Information"), GTK_STOCK_INFO, _cd_musicplayer_info, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myConfig.bPauseOnClick)
	{
		_cd_musicplayer_next (NULL, NULL);
	}
	else
	{
		_cd_musicplayer_pp (NULL, NULL);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" %s --> nouvelle pochette ou chanson !", CD_APPLET_RECEIVED_DATA);
	
	gboolean isJpeg = g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpg") 
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPG")
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpeg")
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPEG");
	
	if (isJpeg)
	{
		if (myData.cArtist != NULL && myData.cAlbum != NULL/* && myData.pCurrentHandeler->cCoverDir != NULL*/)
		{
			cd_debug("Le fichier est un JPEG");
			gchar *cDirPath = myData.pCurrentHandeler->cCoverDir ? g_strdup (myData.pCurrentHandeler->cCoverDir) : g_strdup_printf("%s/musicplayer", g_cCairoDockDataDir);
			gchar *cCommand, *cHost = NULL;
			gchar *cFilePath = (*CD_APPLET_RECEIVED_DATA == '/' ? g_strdup (CD_APPLET_RECEIVED_DATA) : g_filename_from_uri (CD_APPLET_RECEIVED_DATA, &cHost, NULL));
			if (cHost != NULL)  // fichier distant, on le telecharge dans le cache du lecteur.
			{
				cd_debug("Le fichier est distant (sur %s)", cHost);
				cCommand = g_strdup_printf ("wget -O \"%s/%s - %s.jpg\" '%s'",
					cDirPath,
					myData.cArtist,
					myData.cAlbum,
					CD_APPLET_RECEIVED_DATA);
			}
			else  // fichier local, on le copie juste dans le cache du lecteur.
			{
				cd_debug("Le fichier est local");
				cCommand = g_strdup_printf ("cp \"%s\" \"%s/%s - %s.jpg\"",
					cFilePath,
					cDirPath,
					myData.cArtist,
					myData.cAlbum);
				
			}
			cd_debug ("on recupere la pochette par : '%s'", cCommand);
			cairo_dock_launch_command (cCommand);
			g_free (cCommand);
			g_free (cFilePath);
			g_free (cHost);
			g_free (cDirPath);
			
			cd_musicplayer_get_cover_path (NULL, TRUE);
			cd_musicplayer_update_icon (FALSE);
		}
	}
	else
	{
		cd_debug ("on rajoute la chanson a la queue.");
		myData.pCurrentHandeler->control (PLAYER_ENQUEUE, CD_APPLET_RECEIVED_DATA);
	}
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_SCROLL_BEGIN
	if (CD_APPLET_SCROLL_DOWN) {
  		_cd_musicplayer_next (NULL, NULL);
	}
	else if (CD_APPLET_SCROLL_UP) {
  		_cd_musicplayer_prev (NULL, NULL);
	}
	else
  		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
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


gboolean cd_opengl_test_mouse_over_buttons (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	int iPrevButtonState = myData.iButtonState;
	myData.iButtonState = cd_opengl_check_buttons_state (myApplet);
	
	if (iPrevButtonState != myData.iButtonState)
	{
		*bStartAnimation = TRUE;  // ca c'est pour faire une animation de transition...
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-musicplayer.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the musicPlayer applet\n made by ChAnGFu & Mav for Cairo-Dock"))

void cd_musicplayer_prev (void) {
	myData.pCurrentHandeler->control (PLAYER_PREVIOUS, NULL);
}
void cd_musicplayer_pp (void) {
	myData.pCurrentHandeler->control (PLAYER_PLAY_PAUSE, NULL);
}
void cd_musicplayer_s (void) {
	myData.pCurrentHandeler->control (PLAYER_STOP, NULL);
}
void cd_musicplayer_next (void) {
	myData.pCurrentHandeler->control (PLAYER_NEXT, NULL);
}
void cd_musicplayer_jumpbox (void) {
	myData.pCurrentHandeler->control (PLAYER_JUMPBOX, NULL);
}
void cd_musicplayer_shuffle (void) {
	myData.pCurrentHandeler->control (PLAYER_SHUFFLE, NULL);
}
void cd_musicplayer_repeat (void) {
	myData.pCurrentHandeler->control (PLAYER_REPEAT, NULL);
}


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
static void _musicplayer_action_by_id (int iAction) {
	switch (iAction) {
		case 0:
			myData.pCurrentHandeler->control (PLAYER_PREVIOUS, NULL);
		break;
		case 1:
			myData.pCurrentHandeler->control (PLAYER_PLAY_PAUSE, NULL);
		break;
		case 2:
			myData.pCurrentHandeler->control (PLAYER_STOP, NULL);
		break;
		case 3:
			myData.pCurrentHandeler->control (PLAYER_NEXT, NULL);
		break;
		default :
			cd_warning ("No action defined, Halt.");
		break;
	}
}
CD_APPLET_ON_CLICK_BEGIN
	if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL && pClickedIcon != myIcon) {  // clic sur une des icones du desklet.
		_musicplayer_action_by_id (pClickedIcon->iType);
	}
	else {
	  myData.pCurrentHandeler->control (PLAYER_PLAY_PAUSE, NULL);
	}
CD_APPLET_ON_CLICK_END

//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU (myConfig.cMusicPlayer, pSubMenu, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU (D_("Previous"), cd_musicplayer_prev, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU (D_("Play/Pause (left-click)"), cd_musicplayer_pp, CD_APPLET_MY_MENU)
	
	if (myData.pCurrentHandeler->ask_control (PLAYER_STOP))
		CD_APPLET_ADD_IN_MENU (D_("Stop"), cd_musicplayer_s, CD_APPLET_MY_MENU)
		
	CD_APPLET_ADD_IN_MENU (D_("Next (middle-click)"), cd_musicplayer_next, CD_APPLET_MY_MENU)
	
	if (myData.pCurrentHandeler->ask_control (PLAYER_JUMPBOX))
		CD_APPLET_ADD_IN_MENU (D_("Show JumpBox"), cd_musicplayer_jumpbox, pSubMenu)
		
	if (myData.pCurrentHandeler->ask_control (PLAYER_SHUFFLE))	
		CD_APPLET_ADD_IN_MENU (D_("Toggle Shuffle"), cd_musicplayer_shuffle, pSubMenu)
		
	if (myData.pCurrentHandeler->ask_control (PLAYER_REPEAT))	
		CD_APPLET_ADD_IN_MENU (D_("Toggle Repeat"), cd_musicplayer_repeat, pSubMenu)
		
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  myData.pCurrentHandeler->control (PLAYER_NEXT, NULL);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" Musicplayer: %s to enqueue", CD_APPLET_RECEIVED_DATA);
	myData.pCurrentHandeler->control (PLAYER_ENQUEUE, CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_SCROLL_BEGIN
		if (CD_APPLET_SCROLL_DOWN) {
			myData.pCurrentHandeler->control (PLAYER_NEXT, NULL);
		}
		else if (CD_APPLET_SCROLL_UP) {
			myData.pCurrentHandeler->control (PLAYER_PREVIOUS, NULL);
		}
		else
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_SCROLL_END

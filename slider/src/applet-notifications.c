/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-silder.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_ABOUT (D_("This is the Slider applet\n made by ChAnGFu for Cairo-Dock"))

void cd_slider_toogle_pause(void) {
	cd_message("Toggeling pause: %d", myData.bPause);
  if (!myData.bPause) {
  	myData.bPause = TRUE;
  	g_source_remove(myData.iTimerID); //on coupe le timer en cours
  }
 	else {
 		myData.bPause = FALSE;
 		cd_slider_draw_images(); //on relance le diapo
 	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_slider_toogle_pause();
	
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Slider", pSubMenu, CD_APPLET_MY_MENU)
	
		if (myData.bPause) {
			CD_APPLET_ADD_IN_MENU (D_("Play"), cd_slider_toogle_pause, pSubMenu)
		}
		else {
			CD_APPLET_ADD_IN_MENU (D_("Pause"), cd_slider_toogle_pause, pSubMenu)
		}
		
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

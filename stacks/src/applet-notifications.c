/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-stacks.h"


CD_APPLET_ABOUT (D_("This is the stacks applet\n made by ChAnGFu for Cairo-Dock"))


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON != myIcon) {
		//cd_message ("Stacks - Click on %s(%s)", CD_APPLET_CLICKED_ICON->acName, CD_APPLET_CLICKED_ICON->cBaseURI);
		cairo_dock_fm_launch_uri(CD_APPLET_CLICKED_ICON->cBaseURI);
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_IN_MENU (D_("Reload Stacks"), cd_stacks_reload, pSubMenu);
	CD_APPLET_ADD_IN_MENU (D_("Clean local directory"), cd_stacks_clean_local, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  cd_stacks_run_dir();
CD_APPLET_ON_MIDDLE_CLICK_END

CD_APPLET_ON_DROP_DATA_BEGIN
	//cd_message ("File to link in local: %s", CD_APPLET_RECEIVED_DATA);
	cd_stacks_mklink (CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END

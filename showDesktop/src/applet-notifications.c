/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


CD_APPLET_ABOUT (D_("This is the showDesktop applet\n made by Romain Perol for Cairo-Dock"))


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("showDesktop", pSubMenu, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	if (bDesktopIsVisible)  // on remet comme avant.
	{
		cairo_dock_set_desklets_visibility_to_default ();
	}
	else  // on montre le bureau, et les desklets.
	{
		cairo_dock_set_all_desklets_visible (FALSE);  // ne passe pas les desklets de la couche des widgets devant.
	}
CD_APPLET_ON_MIDDLE_CLICK_END

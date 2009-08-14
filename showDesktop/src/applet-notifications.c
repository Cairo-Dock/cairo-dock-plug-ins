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


static gboolean _cd_allow_minimize (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, gpointer data)
{
	pDesklet->bAllowMinimize = TRUE;
	return FALSE;
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	
	if (! bDesktopIsVisible && !myConfig.bswapclic)  // on autorise chaque desklet a etre minimise. l'autorisation est annulee lors de leur cachage, donc on n'a pas besoin de faire le contraire apres avoir montre le bureau.
	{
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cd_allow_minimize, NULL);
	}
	
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	
	if (! bDesktopIsVisible && myConfig.bswapclic)  // on autorise chaque desklet a etre minimise. l'autorisation est annulee lors de leur cachage, donc on n'a pas besoin de faire le contraire apres avoir montre le bureau.
	{
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cd_allow_minimize, NULL);
	}
	
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
CD_APPLET_ON_MIDDLE_CLICK_END

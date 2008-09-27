/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the Clipper applet\n made by Fabrice Rey (Fabounet) for Cairo-Dock"))


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (myData.pItems == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_ICON_FILE);
		cairo_dock_show_temporary_dialog_with_icon (D_("No items yet."), myIcon, myContainer, 2000, cIconPath);
		g_free (cIconPath);
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_items_menu ();
		cd_clipper_show_menu (pMenu, 1);
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _cd_clipper_clear_history (GtkMenuItem *menu_item, gpointer *data)
{
	GtkClipboard *pClipBoard;
	if ((myConfig.iItemType & CD_CLIPPER_CLIPBOARD) && myConfig.iNbItems != 0)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_clear (pClipBoard);
	}
	
	if ((myConfig.iItemType & CD_CLIPPER_PRIMARY) && myConfig.iNbItems != 0)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_clear (pClipBoard);
	}
	
	g_list_foreach (myData.pItems, (GFunc) g_free, NULL);
        g_list_free (myData.pItems);
        myData.pItems = NULL;
        int i;
        for (i=0; i<4; i++)
	        myData.iNbItems[i] = 0;
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Clipper", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK ("Clear clipboard History", "gtk-clear", _cd_clipper_clear_history, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myConfig.pPersistentItems == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_ICON_FILE);
		cairo_dock_show_temporary_dialog_with_icon (D_("No persistent items.\nYou can add some by editing the config of this applet."), myIcon, myContainer, 6000, cIconPath);
		g_free (cIconPath);
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_persistent_items_menu ();
		cd_clipper_show_menu (pMenu, 1);
	}
CD_APPLET_ON_MIDDLE_CLICK_END

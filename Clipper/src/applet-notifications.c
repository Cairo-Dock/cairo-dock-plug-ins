/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (myData.pItems == NULL)
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("No items yet."), myIcon, myContainer, 2000, "same icon");
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
	if (myConfig.iItemType & CD_CLIPPER_CLIPBOARD)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_clear (pClipBoard);
	}
	
	if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
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
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU_WITH_STOCK ("Clear clipboard History", "gtk-clear", _cd_clipper_clear_history, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myConfig.pPersistentItems == NULL)
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("No persistent items.\nYou can add some by drag and dropping some text on the icon."), myIcon, myContainer, 6000, "same icon");
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_persistent_items_menu ();
		cd_clipper_show_menu (pMenu, 1);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message ("'%s' --> permanent !", CD_APPLET_RECEIVED_DATA);
	
	if (myConfig.pPersistentItems == NULL)
	{
		myConfig.pPersistentItems = g_new0 (gchar *, 2);
		myConfig.pPersistentItems[0] = g_strdup (CD_APPLET_RECEIVED_DATA);
	}
	else
	{
		GString *pString = g_string_new ("");
		int i;
		for (i = 0; myConfig.pPersistentItems[i] != NULL; i ++)
		{
			g_string_append_printf (pString, "%s;", myConfig.pPersistentItems[i]);
		}
		g_string_append (pString, CD_APPLET_RECEIVED_DATA);
		cairo_dock_update_conf_file (myApplet->cConfFilePath,
			G_TYPE_STRING, "Configuration", "persistent", pString->str,
			G_TYPE_INVALID);
		
		myConfig.pPersistentItems = g_realloc (myConfig.pPersistentItems, (i + 2) * sizeof (gchar*));
		myConfig.pPersistentItems[i] = g_strdup (CD_APPLET_RECEIVED_DATA);
		myConfig.pPersistentItems[i+1] = NULL;  // precaution.
		g_string_free (pString, TRUE);
	}
CD_APPLET_ON_DROP_DATA_END

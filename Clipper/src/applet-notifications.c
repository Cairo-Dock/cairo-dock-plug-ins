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

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the Clipper applet\n made by Fabrice Rey (Fabounet) for Cairo-Dock"))


static void _cd_clipper_activate_item (GtkMenuItem *pMenuItem, gchar *cText)
{
	g_print ("%s (%s)\n", __func__, cText);
	
	GtkClipboard *pClipBoard;
	if (myConfig.bPasteInClipboard)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	if (myConfig.bPasteInPrimary)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	
	if (myConfig.bEnableActions)
	{
		
	}
}
static void _cd_clipper_build_menu (gchar *cText, GtkWidget *pMenu)
{
	GtkWidget *pMenuItem;
	CD_APPLET_ADD_IN_MENU_WITH_DATA (cText, _cd_clipper_activate_item, pMenu, cText);
}
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
		GtkWidget *pMenu = gtk_menu_new ();
		GtkWidget *pMenuItem;
		
		if (myDock)
		{
			myDock->bMenuVisible = TRUE;
			g_signal_connect (G_OBJECT (pMenu),
				"deactivate",
				G_CALLBACK (cairo_dock_delete_menu),
				myDock);
		}
		
		g_list_foreach (myData.pItems, (GFunc)_cd_clipper_build_menu, pMenu);
		gtk_widget_show_all (pMenu);
		gtk_menu_popup (GTK_MENU (pMenu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Clipper", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

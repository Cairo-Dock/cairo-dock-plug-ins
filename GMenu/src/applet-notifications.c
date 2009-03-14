/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-notifications.h"


CD_APPLET_ABOUT (D_("This is the GMenu applet\n made by Fabounet (Fabrice Rey) for Cairo-Dock"))

static void cd_menu_show_menu (void)
{
	if (myData.pMenu != NULL)
	{
		myDock->bMenuVisible = TRUE;
		gtk_menu_popup (GTK_MENU (myData.pMenu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
}
static void _cd_menu_on_quick_launch (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok ou entree.
	{
		const gchar *cCommand = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cCommand != NULL && *cCommand != '0')
			cairo_dock_launch_command (cCommand);
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY (pInteractiveWidget), "");
	}
	cairo_dock_dialog_reference (myData.pQuickLaunchDialog);
	cairo_dock_hide_dialog (myData.pQuickLaunchDialog);
}
static void cd_menu_show_hide_quick_launch (void)
{
	if (myData.pQuickLaunchDialog == NULL)
	{
		myData.pQuickLaunchDialog = cairo_dock_show_dialog_with_entry (D_("Enter a command to launch :"),
			myIcon, myContainer,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			NULL,
			(CairoDockActionOnAnswerFunc) _cd_menu_on_quick_launch, NULL, NULL);
		cairo_dock_dialog_reference (myData.pQuickLaunchDialog);
	}
	else
	{
		cairo_dock_toggle_dialog_visibility (myData.pQuickLaunchDialog);
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_menu_show_menu ();
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _cd_menu_configure_menu (GtkMenuItem *menu_item, gpointer data)
{
	if (myConfig.cConfigureMenuCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cConfigureMenuCommand);
	}
	else
	{
		if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE)  /// a confirmer pour XFCE ...
			cairo_dock_launch_command ("alacarte");
		else
			cd_warning ("Sorry, couldn't guess what to do to configure the menu");
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("GMenu", pSubMenu, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Quick launch"), GTK_STOCK_EXECUTE, cd_menu_show_hide_quick_launch, pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Configure menu"), GTK_STOCK_PREFERENCES, _cd_menu_configure_menu, pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear recent"), GTK_STOCK_CLEAR, cd_menu_clear_recent, pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_menu_show_hide_quick_launch ();
CD_APPLET_ON_MIDDLE_CLICK_END


void cd_menu_on_shortkey_menu (const char *keystring, gpointer data)
{
	cd_menu_show_menu ();
}

void cd_menu_on_shortkey_quick_launch (const char *keystring, gpointer data)
{
	cd_menu_show_hide_quick_launch ();
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-run-dialog.h"
#include "applet-notifications.h"



static void cd_menu_show_menu (void)
{
	if (myData.pMenu != NULL)
	{
		if (myDock)
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

static void cd_menu_show_hide_quick_launch (void)
{
	if (myData.pQuickLaunchDialog == NULL)
	{
		myData.pQuickLaunchDialog = cd_menu_create_quick_launch_dialog (myApplet);
		cairo_dock_dialog_reference (myData.pQuickLaunchDialog);
	}
	else
	{
		cairo_dock_toggle_dialog_visibility (myData.pQuickLaunchDialog);
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
static void _on_activate_item (GtkWidget *pMenuItem, gpointer *data);
static void _fill_menu_with_dir (gchar *cDirPath, GtkWidget *pMenu)
{
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		return ;
	}
	
	const gchar *cFileName;
	gchar *cPath;
	gpointer *data;
	GtkWidget *pMenuItem;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		cPath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		pMenuItem = gtk_menu_item_new_with_label (cFileName);
		data = g_new (gpointer, 2);
		data[0] = cPath;
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), data);
		
		if (g_file_test (cPath, G_FILE_TEST_IS_DIR))
		{
			GtkWidget *pSubMenu = gtk_menu_new ();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
			data[1] = pSubMenu;
		}
	}
	while (1);
	g_dir_close (dir);
}
static void _on_activate_item (GtkWidget *pMenuItem, gpointer *data)
{
	gchar *cPath = data[0];
	g_print ("%s (%s)\n", __func__, cPath);
	
	if (g_file_test (cPath, G_FILE_TEST_IS_DIR))
	{
		g_print ("c'est un repertoire\n");
		GtkWidget *pSubMenu = data[1];
		_fill_menu_with_dir (cPath, pSubMenu);
		gtk_widget_show_all (pSubMenu);
	}
	else
	{
		cairo_dock_fm_launch_uri (cPath);
	}
}
CD_APPLET_ON_CLICK_BEGIN
	cd_menu_show_menu ();
	
	/*gchar *cDirPath = g_getenv ("HOME");
	GtkWidget *pMenu = gtk_menu_new ();
	
	_fill_menu_with_dir (cDirPath, pMenu);
	
	gtk_widget_show_all (pMenu);
	
	gtk_menu_popup (GTK_MENU (pMenu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());*/
	
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
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
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

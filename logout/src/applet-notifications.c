
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "applet-notifications.h"

extern Icon *myIcon;
extern CairoDock *myDock;

extern CairoDockDesktopEnv my_logout_iDesktopEnv;


CD_APPLET_ABOUT ("This is a very simple logout applet made by Fabrice Rey for Cairo-Dock")


CD_APPLET_ON_CLICK_BEGIN
	if (my_logout_iDesktopEnv == CAIRO_DOCK_GNOME)
	{
		system ("gnome-session-save --kill --gui");
	}
	else if (my_logout_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		GtkWidget *dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Log out ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer == GTK_RESPONSE_YES)
		{
			system ("dcop ksmserver default logout 0 0 0");  // kdmctl shutdown reboot forcenow  // kdeinit_shutdown
		}
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = NULL;
	CD_APPLET_ADD_SUB_MENU("Logout", pSubMenu, pAppletMenu)
	/*pMenuItem = gtk_menu_item_new_with_label ("Logout");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
	GtkWidget *pSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
	
	pMenuItem = gtk_menu_item_new_with_label ("About");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), pMenuItem);
	g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (about), NULL);*/
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

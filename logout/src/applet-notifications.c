
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "applet-notifications.h"

extern Icon *myIcon;
extern CairoDock *myDock;

extern CairoDockDesktopEnv my_logout_iDesktopEnv;


void cd_logout_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is a very simple logout applet made by Fabrice Rey for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


gboolean cd_logout_notification_click_icon (gpointer *data)
{
	if (data[0] == myIcon)
	{
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
		
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_logout_notification_build_menu (gpointer *data)
{
	if (data[0] == myIcon)
	{
		GtkWidget *menu = data[2];
		
		GtkWidget *menu_item;
		menu_item = gtk_menu_item_new_with_label ("Logout");
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		
		GtkWidget *pSubMenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
		
		menu_item = gtk_menu_item_new_with_label ("About");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_logout_about), NULL);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


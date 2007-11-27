/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>

#include "cd-clock-menu-functions.h"

extern CairoDock *myDock;
extern Icon *myIcon;

void cd_clock_launch_time_admin (GtkMenuItem *menu_item, gpointer *data)
{
	system ("gksu time-admin &");
}


void cd_clock_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is a clock applet made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock.\n The analogic representation is a port of the well-known Cairo-Clock from MacSlow (http://macslow.thepimp.net).");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


gboolean cd_clock_notification_click_icon (gpointer *data)
{
	//g_print ("%s ()\n", __func__);
	if (data[0] == myIcon)
	{
		GtkWidget *pDialog = gtk_dialog_new ();
		
		GtkWidget *pCalendar = gtk_calendar_new ();
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pCalendar);
		gtk_widget_show (pCalendar);
		gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);
		
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_clock_notification_build_menu (gpointer *data)
{
	if (data[0] == myIcon)
	{
		GtkWidget *menu = data[2];
		
		GtkWidget *menu_item;
		menu_item = gtk_menu_item_new_with_label ("Clock");
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		
		GtkWidget *pSubMenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
		
		menu_item = gtk_menu_item_new_with_label ("Set up time and date");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_clock_launch_time_admin), NULL);
		
		menu_item = gtk_menu_item_new_with_label ("About");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_clock_about), NULL);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
